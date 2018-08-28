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
typedef enum tagDaq_AllocResultType {
    DAQ_ALLOC_OK,
    DAQ_ALLOC_ERR
} Daq_AllocResultType;

typedef enum tagDaq_AllocStateType {
    XCP_ALLOC_IDLE,
    XCP_AFTER_FREE_DAQ,
    XCP_AFTER_ALLOC_DAQ,
    XCP_AFTER_ALLOC_ODT,
    XCP_AFTER_ALLOC_ODT_ENTRY
} Daq_AllocStateType;

typedef enum tagDaq_AllocTransitionType {
    XCP_CALL_FREE_DAQ,
    XCP_CALL_ALLOC_DAQ,
    XCP_CALL_ALLOC_ODT,
    XCP_CALL_ALLOC_ODT_ENTRY
} Daq_AllocTransitionype;

typedef enum tagDaq_ListTransitionType {
    DAQ_LIST_TRANSITION_START,
    DAQ_LIST_TRANSITION_STOP
} Daq_ListTransitionType;

/*
** Local Constants.
*/
static const uint8_t Daq_AllocTransitionTable[5][4] = {
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
static Daq_AllocStateType Daq_AllocState;
static Xcp_DaqEntityType daqEntities[NUM_DAQ_ENTITIES];
static uint16_t daqEntityCount = UINT16(0);
static uint16_t daqListCount = UINT16(0);
static uint16_t daqOdtCount = UINT16(0);

#if XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED  == XCP_OFF
static uint8_t daqListForEvent[XCP_DAQ_MAX_EVENT_CHANNEL];
#else
    #error XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED option currently not supported
#endif // XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED


/*
** Local Function Prototypes.
*/
static Xcp_ODTType * Daq_GetOdt(uint8_t daqListNumber, uint8_t odtNumber);
static bool Daq_AllocValidTransition(Daq_AllocTransitionype transition);
void dumpEntities(void);


/*
** Global Functions.
*/
Xcp_ReturnType Xcp_FreeDaq(void)
{
    Xcp_ReturnType result = ERR_SUCCESS;

    daqEntityCount = UINT16(0);
    daqListCount = UINT16(0);
    daqOdtCount = UINT16(0);

#if XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED  == XCP_OFF
    Xcp_MemSet(&daqListForEvent, UINT8(0), UINT32(sizeof(daqListForEvent[0]) * XCP_DAQ_MAX_EVENT_CHANNEL));
#endif // XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED

    if (Daq_AllocValidTransition(XCP_CALL_FREE_DAQ)) {
        Xcp_MemSet(daqEntities, UINT8(0), UINT32(sizeof(Xcp_DaqEntityType) * UINT16(NUM_DAQ_ENTITIES)));
        Daq_AllocState = XCP_AFTER_FREE_DAQ;
    } else {
        result = ERR_SEQUENCE;
    }
    return result;
}

Xcp_ReturnType Xcp_AllocDaq(uint16_t daqCount)
{
    uint16_t idx;
    Xcp_ReturnType result = ERR_SUCCESS;

    if (!Daq_AllocValidTransition(XCP_CALL_ALLOC_DAQ)) {
        result = ERR_SEQUENCE;
        DBG_PRINT1("Xcp_AllocDaq() not allowed.\n");
    } else {
        if ((daqEntityCount + daqCount) <= UINT16(NUM_DAQ_ENTITIES)) {
            Daq_AllocState = XCP_AFTER_ALLOC_DAQ;
            for (idx = daqEntityCount; idx < (daqEntityCount + daqCount); ++idx) {
                daqEntities[idx].kind = UINT8(XCP_ENTITY_DAQ_LIST);
                daqEntities[idx].entity.daqList.numOdts = UINT8(0);
            }
            daqListCount += daqCount;
            daqEntityCount += daqCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
            DBG_PRINT1("Xcp_AllocDaq(): not enough memory.\n");
        }
    }
    return result;
}

Xcp_ReturnType Xcp_AllocOdt(uint16_t daqListNumber, uint8_t odtCount)
{
    uint16_t idx;
    Xcp_ReturnType result = ERR_SUCCESS;

    if (!Daq_AllocValidTransition(XCP_CALL_ALLOC_ODT)) {
        result = ERR_SEQUENCE;
        DBG_PRINT1("Xcp_AllocOdt() not allowed.\n");
    } else {
        if ((daqEntityCount + odtCount) <= UINT16(NUM_DAQ_ENTITIES)) {
            Daq_AllocState = XCP_AFTER_ALLOC_ODT;
            for (idx = daqEntityCount; idx < (daqEntityCount + odtCount); ++idx) {
                daqEntities[idx].kind = UINT8(XCP_ENTITY_ODT);
            }
            daqEntities[daqListNumber].entity.daqList.numOdts += odtCount;
            daqEntities[daqListNumber].entity.daqList.firstOdt = daqEntityCount;
            daqOdtCount += odtCount;
            daqEntityCount += odtCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
            DBG_PRINT1("Xcp_AllocOdt(): not enough memory.\n");
        }

    }
    return result;
}

Xcp_ReturnType Xcp_AllocOdtEntry(uint16_t daqListNumber, uint8_t odtNumber, uint8_t odtEntriesCount)
{
    uint16_t idx;
    uint8_t odt;
    Xcp_ReturnType result = ERR_SUCCESS;

    if (!Daq_AllocValidTransition(XCP_CALL_ALLOC_ODT_ENTRY)) {
        result = ERR_SEQUENCE;
        DBG_PRINT1("Xcp_AllocOdtEntry() not allowed.\n");
    } else {
        if ((daqEntityCount + odtEntriesCount) <= UINT16(NUM_DAQ_ENTITIES)) {
            Daq_AllocState = XCP_AFTER_ALLOC_ODT_ENTRY;
            for (idx = daqEntityCount; idx < (daqEntityCount + odtEntriesCount); ++idx) {
                daqEntities[idx].kind = UINT8(XCP_ENTITY_ODT_ENTRY);
            }
            odt = daqEntities[daqListNumber].entity.daqList.firstOdt + odtNumber;
            daqEntities[odt].entity.odt.firstOdtEntry = daqEntityCount;
            daqEntities[odt].entity.odt.numOdtEntries = odtEntriesCount;
            daqEntityCount += odtEntriesCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
            DBG_PRINT1("Xcp_AllocOdtEntry(): not enough memory.\n");
        }
    }
    return result;
}

void Xcp_InitDaq(void)
{
    Daq_AllocState = XCP_ALLOC_IDLE;
}

Xcp_ODTEntryType * Daq_GetOdtEntry(uint8_t daqListNumber, uint8_t odtNumber, uint8_t odtEntryNumber)
{
    Xcp_ODTType * odt;
    uint8_t idx;

    odt = Daq_GetOdt(daqListNumber, odtNumber);
    idx = odt->firstOdtEntry + odtEntryNumber;
    return &daqEntities[idx].entity.odtEntry;
}

Xcp_DaqListType * Daq_GetList(uint8_t daqListNumber)
{
    return &daqEntities[daqListNumber].entity.daqList;
}

bool XcpDaq_ValidateConfiguration(void)
{
    return ((daqEntityCount > UINT16(0)) && (daqListCount > UINT16(0)) &&  (daqOdtCount > UINT16(0)));
}

bool Xcp_DaqListValid(uint8_t daqListNumber)
{
    Xcp_DaqListType * daqList;
    bool result = (bool)TRUE;

    // TODO: Rangecheck!!!

    daqList = Daq_GetList(daqListNumber);
    if (daqList->numOdts == UINT8(0)) {
        result = (bool)FALSE;
    }
    return result;
}

void Xcp_DaqMainfunction(void)
{
    // TODO: Check global state for DAQ/STIM running.
}

void Xcp_DaqAddEventChannel(uint16_t daqListNumber, uint16_t eventChannelNumber)
{

}

void Xcp_TriggerDaqEvent(uint8_t eventNumber)
{
    uint16_t daqList;

    if (eventNumber > UINT8(XCP_DAQ_MAX_EVENT_CHANNEL - 1)) {
        return;
    }

#if XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED  == XCP_OFF
    daqList = daqListForEvent[eventNumber];
#endif // XCP_DAQ_MULTIPLE_DAQ_LISTS_PER_EVENT_SUPPORTED
}


#if defined(_WIN32)
void dumpEntities(void)
{
    uint16_t idx;
    Xcp_DaqEntityType * entry;

    for (idx = 0; idx < daqEntityCount; ++idx) {
        entry = &daqEntities[idx];
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
        }
    }
}
#endif

/*
** Local Functions.
*/
static Xcp_ODTType * Daq_GetOdt(uint8_t daqListNumber, uint8_t odtNumber)
{
    Xcp_DaqListType const * dl;
    uint8_t idx;

    dl = Daq_GetList(daqListNumber);
    idx = dl->firstOdt + odtNumber;
    return &daqEntities[idx].entity.odt;
}

static bool Daq_AllocValidTransition(Daq_AllocTransitionype transition)
{
    if (Daq_AllocTransitionTable[Daq_AllocState][transition] == UINT8(DAQ_ALLOC_OK)) {
        return (bool)TRUE;
    } else {
        return (bool)FALSE;
    }
}


static void Daq_TransitLists(Daq_ListTransitionType transition)
{
    /*
     *  The slave has to reset the SELECTED flag in the mode at GET_DAQ_LIST_MODE as soon
     *  as the related START_STOP_SYNCH or SET_REQUEST have been acknowledged.
     */
    uint8_t idx;
    Xcp_DaqListType * entry;

    for (idx = UINT8(0); idx < daqListCount; ++idx) {
        entry = Daq_GetList(idx);
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
