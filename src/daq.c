/*
 * pySART - Simplified AUTOSAR-Toolkit for Python.
 *
 * (C) 2007-2018 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

#if defined(_WIN32)
#include <stdio.h>
#endif // _WIN32

#include "xcp.h"


#define NUM_DAQ_ENTITIES    (256)


/*
** Local Types.
*/
typedef enum tagXcpDaq_AllocResultType {
    DAQ_ALLOC_OK,
    DAQ_ALLOC_ERR
} XcpDaq_AllocResultType;

typedef enum tagXcpDaq_AllocStateType {
    XCP_ALLOC_IDLE,
    XCP_AFTER_FREE_DAQ,
    XCP_AFTER_ALLOC_DAQ,
    XCP_AFTER_ALLOC_ODT,
    XCP_AFTER_ALLOC_ODT_ENTRY
} XcpDaq_AllocStateType;

typedef enum tagXcpDaq_AllocTransitionType {
    XCP_CALL_FREE_DAQ,
    XCP_CALL_ALLOC_DAQ,
    XCP_CALL_ALLOC_ODT,
    XCP_CALL_ALLOC_ODT_ENTRY
} XcpDaq_AllocTransitionype;

typedef enum tagXcpDaq_ListTransitionType {
    DAQ_LIST_TRANSITION_START,
    DAQ_LIST_TRANSITION_STOP
} XcpDaq_ListTransitionType;

/*
** Local Constants.
*/
static const uint8_t XcpDaq_AllocTransitionTable[5][4] = {
                            /* FREE_DAQ           ALLOC_DAQ             ALLOC_ODT             ALLOC_ODT_ENTRY    */
/* ALLOC_IDLE*/             {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR) },
/* AFTER_FREE_DAQ  */       {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_OK) , UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR) },
/* AFTER_ALLOC_DAQ */       {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_OK) , UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_ERR) },
/* AFTER_ALLOC_ODT */       {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_OK)  },
/* AFTER_ALLOC_ODT_ENTRY */ {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_OK)  },
};


/*
** Local Variables.
*/
static XcpDaq_AllocStateType XcpDaq_AllocState;
static XcpDaq_EntityType XcpDaq_Entities[NUM_DAQ_ENTITIES];
static uint16_t XcpDaq_EntityCount = UINT16(0);
static uint16_t XcpDaq_ListCount = UINT16(0);
static uint16_t XcpDaq_OdtCount = UINT16(0);

#if XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED  == XCP_OFF
static uint8_t XcpDaq_ListForEvent[XCP_DAQ_MAX_EVENT_CHANNEL];
#else
    #error XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED option currently not supported
#endif // XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED


/*
** Local Function Prototypes.
*/
static XcpDaq_ODTType * XcpDaq_GetOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber);
static bool XcpDaq_AllocValidTransition(XcpDaq_AllocTransitionype transition);
void XcpDaq_DumpEntities(void);


/*
** Global Functions.
*/
Xcp_ReturnType XcpDaq_Free(void)
{
    Xcp_ReturnType result = ERR_SUCCESS;

    XcpDaq_EntityCount = UINT16(0);
    XcpDaq_ListCount = UINT16(0);
    XcpDaq_OdtCount = UINT16(0);

#if XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED  == XCP_OFF
    Xcp_MemSet(XcpDaq_ListForEvent, UINT8(0), UINT32(sizeof(XcpDaq_ListForEvent[0]) * UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)));
#endif // XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED

    if (XcpDaq_AllocValidTransition(XCP_CALL_FREE_DAQ)) {
        Xcp_MemSet(XcpDaq_Entities, UINT8(0), UINT32(sizeof(XcpDaq_EntityType) * UINT16(NUM_DAQ_ENTITIES)));
        XcpDaq_AllocState = XCP_AFTER_FREE_DAQ;
    } else {
        result = ERR_SEQUENCE;
    }
    return result;
}

Xcp_ReturnType XcpDaq_Alloc(XcpDaq_ListIntegerType daqCount)
{
    uint16_t idx;
    Xcp_ReturnType result = ERR_SUCCESS;

    if (!XcpDaq_AllocValidTransition(XCP_CALL_ALLOC_DAQ)) {
        result = ERR_SEQUENCE;
        DBG_PRINT1("Xcp_AllocDaq() not allowed.\n");
    } else {
        if ((XcpDaq_EntityCount + daqCount) <= UINT16(NUM_DAQ_ENTITIES)) {
            XcpDaq_AllocState = XCP_AFTER_ALLOC_DAQ;
            for (idx = XcpDaq_EntityCount; idx < (XcpDaq_EntityCount + daqCount); ++idx) {
                XcpDaq_Entities[idx].kind = UINT8(XCP_ENTITY_DAQ_LIST);
                XcpDaq_Entities[idx].entity.daqList.numOdts = (XcpDaq_ODTIntegerType)0;
            }
            XcpDaq_ListCount += UINT16(daqCount);
            XcpDaq_EntityCount += UINT16(daqCount);
        } else {
            result = ERR_MEMORY_OVERFLOW;
            DBG_PRINT1("Xcp_AllocDaq(): not enough memory.\n");
        }
    }
    return result;
}

Xcp_ReturnType XcpDaq_AllocOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtCount)
{
    uint16_t idx;
    Xcp_ReturnType result = ERR_SUCCESS;

    if (!XcpDaq_AllocValidTransition(XCP_CALL_ALLOC_ODT)) {
        result = ERR_SEQUENCE;
        DBG_PRINT1("Xcp_AllocOdt() not allowed.\n");
    } else {
        if ((XcpDaq_EntityCount + odtCount) <= UINT16(NUM_DAQ_ENTITIES)) {
            XcpDaq_AllocState = XCP_AFTER_ALLOC_ODT;
            for (idx = XcpDaq_EntityCount; idx < (XcpDaq_EntityCount + odtCount); ++idx) {
                XcpDaq_Entities[idx].kind = UINT8(XCP_ENTITY_ODT);
            }
            XcpDaq_Entities[daqListNumber].entity.daqList.numOdts += odtCount;
            XcpDaq_Entities[daqListNumber].entity.daqList.firstOdt = XcpDaq_EntityCount;
            XcpDaq_OdtCount += UINT16(odtCount);
            XcpDaq_EntityCount += UINT16(odtCount);
        } else {
            result = ERR_MEMORY_OVERFLOW;
            DBG_PRINT1("Xcp_AllocOdt(): not enough memory.\n");
        }

    }
    return result;
}

Xcp_ReturnType XcpDaq_AllocOdtEntry(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntriesCount)
{
    uint16_t idx;
    XcpDaq_ODTIntegerType odt;
    Xcp_ReturnType result = ERR_SUCCESS;

    if (!XcpDaq_AllocValidTransition(XCP_CALL_ALLOC_ODT_ENTRY)) {
        result = ERR_SEQUENCE;
        DBG_PRINT1("Xcp_AllocOdtEntry() not allowed.\n");
    } else {
        if ((XcpDaq_EntityCount + odtEntriesCount) <= UINT16(NUM_DAQ_ENTITIES)) {
            XcpDaq_AllocState = XCP_AFTER_ALLOC_ODT_ENTRY;
            for (idx = XcpDaq_EntityCount; idx < (XcpDaq_EntityCount + odtEntriesCount); ++idx) {
                XcpDaq_Entities[idx].kind = UINT8(XCP_ENTITY_ODT_ENTRY);
            }
            odt = (XcpDaq_ODTIntegerType)(XcpDaq_Entities[daqListNumber].entity.daqList.firstOdt + UINT16(odtNumber));
            XcpDaq_Entities[odt].entity.odt.firstOdtEntry = XcpDaq_EntityCount;
            XcpDaq_Entities[odt].entity.odt.numOdtEntries = odtEntriesCount;
            XcpDaq_EntityCount += UINT16(odtEntriesCount);
        } else {
            result = ERR_MEMORY_OVERFLOW;
            DBG_PRINT1("Xcp_AllocOdtEntry(): not enough memory.\n");
        }
    }
    return result;
}

void XcpDaq_Init(void)
{
    XcpDaq_AllocState = XCP_ALLOC_IDLE;
}

XcpDaq_ODTEntryType * XcpDaq_GetOdtEntry(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber)
{
    XcpDaq_ODTType const * odt;
    XcpDaq_ODTIntegerType idx;

    // TODO: Range checking.
    odt = XcpDaq_GetOdt(daqListNumber, odtNumber);
    idx = (XcpDaq_ODTIntegerType)(odt->firstOdtEntry + UINT16(odtEntryNumber));
    return &XcpDaq_Entities[idx].entity.odtEntry;
}

XcpDaq_ListType * XcpDaq_GetList(XcpDaq_ListIntegerType daqListNumber)
{
    return &XcpDaq_Entities[daqListNumber].entity.daqList;
}

bool XcpDaq_ValidateConfiguration(void)
{
    return (bool)((XcpDaq_EntityCount > UINT16(0)) && (XcpDaq_ListCount > UINT16(0)) &&  (XcpDaq_OdtCount > UINT16(0)));
}

bool XcpDaq_ValidateList(XcpDaq_ListIntegerType daqListNumber)
{
    XcpDaq_ListType const * daqList;
    XcpDaq_ODTType const * odt;
    bool result = (bool)TRUE;
    XcpDaq_ODTIntegerType numOdts;
    uint8_t idx;

    if (daqListNumber > (XcpDaq_ListCount - UINT16(1))) {
        result = (bool)FALSE;
    } else {
        daqList = XcpDaq_GetList(daqListNumber);
        numOdts = daqList->numOdts ;
        if (numOdts == UINT8(0)) {
            result = (bool)FALSE;
        } else {
            result = (bool)FALSE;
            for (idx = UINT8(0); idx < numOdts; ++idx) {
                odt = XcpDaq_GetOdt(daqListNumber, idx);
                if (odt->numOdtEntries != UINT8(0)) {
                    result = (bool)TRUE;
                    break;
                }
            }
        }
    }
    return result;
}

bool XcpDaq_ValidateOdtEntry(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntry)
{
    XcpDaq_ListType const * daqList;
    XcpDaq_ODTType const * odt;
    bool result = (bool)TRUE;

    if (daqListNumber > (XcpDaq_ListCount - UINT16(1))) {
        result = (bool)FALSE;
    } else {
        daqList = XcpDaq_GetList(daqListNumber);
        if (odtNumber > (daqList->numOdts - (XcpDaq_ODTIntegerType)1)) {
            result = (bool)FALSE;
        } else {
            odt = XcpDaq_GetOdt(daqListNumber, odtNumber);
            if (odtEntry > (odt->numOdtEntries - (XcpDaq_ODTEntryIntegerType)1)) {
                result = (bool)FALSE;
            }
        }
    }
    return result;
}

void XcpDaq_Mainfunction(void)
{
    // TODO: Check global state for DAQ/STIM running.
}

void XcpDaq_AddEventChannel(XcpDaq_ListIntegerType daqListNumber, uint16_t eventChannelNumber)
{
#if XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED  == XCP_OFF
    XcpDaq_ListForEvent[eventChannelNumber] = daqListNumber;
#endif // XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED
}

void XcpDaq_TriggerEvent(uint8_t eventChannelNumber)
{
    XcpDaq_ListIntegerType daqList;

    if (eventChannelNumber > UINT8(XCP_DAQ_MAX_EVENT_CHANNEL - 1)) {
        return;
    }

#if XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED  == XCP_OFF
    daqList = XcpDaq_ListForEvent[eventChannelNumber];
#endif // XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED
}


#if defined(_WIN32)
void XcpDaq_DumpEntities(void)
{
    uint16_t idx;
    XcpDaq_EntityType const * entry;

    for (idx = UINT16(0); idx < XcpDaq_EntityCount; ++idx) {
        entry = &XcpDaq_Entities[idx];
        switch (entry->kind) {
            case XCP_ENTITY_DAQ_LIST:
                printf("DAQ-LIST [numOdts: %u firstODT: %u]\n", entry->entity.daqList.numOdts, entry->entity.daqList.firstOdt);
                break;
            case XCP_ENTITY_ODT:
                printf("ODT: [numOdtEntries: %u firstOdtEntry: %u]\n", entry->entity.odt.numOdtEntries, entry->entity.odt.firstOdtEntry);
                break;
            case XCP_ENTITY_ODT_ENTRY:
                //printf("ODT-ENTRY: [length: %02u  ext: %02X address: %08X]\n", entry->entity.odtEntry.length,
                //       entry->entity.odtEntry.mta.ext, entry->entity.odtEntry.mta.address);
                break;
            default:
                break;
        }
    }
}
#endif

/*
** Local Functions.
*/
static XcpDaq_ODTType * XcpDaq_GetOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber)
{
    XcpDaq_ListType const * dl;
    XcpDaq_ODTIntegerType idx;

    dl = XcpDaq_GetList(daqListNumber);
    idx = (XcpDaq_ODTIntegerType)dl->firstOdt + odtNumber;
    return &XcpDaq_Entities[idx].entity.odt;
}

static bool XcpDaq_AllocValidTransition(XcpDaq_AllocTransitionype transition)
{
    if (XcpDaq_AllocTransitionTable[XcpDaq_AllocState][transition] == UINT8(DAQ_ALLOC_OK)) {
        return (bool)TRUE;
    } else {
        return (bool)FALSE;
    }
}


static void Daq_TransitLists(XcpDaq_ListTransitionType transition)
{
    /*
     *  The slave has to reset the SELECTED flag in the mode at GET_DAQ_LIST_MODE as soon
     *  as the related START_STOP_SYNCH or SET_REQUEST have been acknowledged.
     */
    XcpDaq_ListIntegerType idx;
    XcpDaq_ListType * entry;

    for (idx = (XcpDaq_ListIntegerType)0; idx < XcpDaq_ListCount; ++idx) {
        entry = XcpDaq_GetList(idx);
        if ((entry->mode & XCP_DAQ_LIST_MODE_SELECTED) == XCP_DAQ_LIST_MODE_SELECTED) {
            if (transition == DAQ_LIST_TRANSITION_START) {
                entry->mode |= XCP_DAQ_LIST_MODE_STARTED;
            } else if (transition == DAQ_LIST_TRANSITION_STOP) {
                entry->mode &= UINT8(~XCP_DAQ_LIST_MODE_STARTED);
            } else {
                /* Do nothing (to keep MISRA happy). */
            }
            entry->mode &= UINT8(~XCP_DAQ_LIST_MODE_SELECTED);
        }
    }
}

#if 0
1.1.1.3  OBJECT DESCRIPTION TABLE (ODT)

ODT entries are grouped in ODTs.
If DAQ lists are configured statically, MAX_ODT_ENTRIES specifies the maximum number of ODT
entries in each ODT of this DAQ list.
If DAQ lists are configured dynamically, MAX_ODT_ENTRIES is not fixed and will be 0.


For every ODT the numbering of the ODT entries through ODT_ENTRY_NUMBER restarts from 0

ODT_ENTRY_NUMBER [0,1,..MAX_ODT_ENTRIES(DAQ list)-1]


#endif // 0
