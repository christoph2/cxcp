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

#include <stdio.h>
#include "xcp.h"

#define NUM_DAQ_ENTITIES    (256)
#define ODT_BASE            (daqListCount)
#define ODT_ENTRY_BASE      (daqListCount + daqOdtCount)

/*
** Local Types.
*/
typedef enum tagDaq_AllocResultType {
    DAQ_ALLOC_OK,
    DAQ_ALLOC_ERR,
} Daq_AllocResultType;

typedef enum tagDaq_AllocStateType {
    XCP_ALLOC_IDLE,
    XCP_AFTER_FREE_DAQ,
    XCP_AFTER_ALLOC_DAQ,
    XCP_AFTER_ALLOC_ODT,
    XCP_AFTER_ALLOC_ODT_ENTRY,
} Daq_AllocStateType;

typedef enum tagDaq_AllocTransitionType {
    XCP_CALL_FREE_DAQ,
    XCP_CALL_ALLOC_DAQ,
    XCP_CALL_ALLOC_ODT,
    XCP_CALL_ALLOC_ODT_ENTRY,
} Daq_AllocTransitionype;

/*
** Local Variables.
*/
static const uint8_t Daq_AllocTransitionTable[5][4] = {
                            /* FREE_DAQ           ALLOC_DAQ             ALLOC_ODT             ALLOC_ODT_ENTRY    */
/* ALLOC_IDLE*/             {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR) },
/* AFTER_FREE_DAQ  */       {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_OK) , UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR) },
/* AFTER_ALLOC_DAQ */       {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_OK) , UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_ERR) },
/* AFTER_ALLOC_ODT */       {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_OK)  },
/* AFTER_ALLOC_ODT_ENTRY */ {UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_OK)  },
};

static Daq_AllocStateType Daq_AllocState;
static Xcp_DaqEntityType daqEntities[NUM_DAQ_ENTITIES];
static uint16_t daqEntityCount = UINT16(0);
static uint16_t daqListCount = UINT16(0);
static uint16_t daqOdtCount = UINT16(0);

/*
** Local Function Prototypes.
*/
static Xcp_DaqListType * Daq_GetList(uint8_t daqListNumber);
static Xcp_ODTType * Daq_GetOdt(uint8_t daqListNumber, uint8_t odtNumber);
static Xcp_ODTEntryType * Daq_GetOdtEntry(uint8_t daqListNumber, uint8_t odtNumber, uint8_t odtEntryNumber);
static bool Daq_AllocValidTransition(Daq_AllocTransitionype transition);

/*
** Global Functions.
*/
static bool Daq_AllocValidTransition(Daq_AllocTransitionype transition)
{
    if (Daq_AllocTransitionTable[Daq_AllocState][transition] == UINT8(DAQ_ALLOC_OK)) {
        return (bool)TRUE;
    } else {
        return (bool)FALSE;
    }
}

Xcp_ReturnType Xcp_FreeDaq(void)
{
    Xcp_ReturnType result = ERR_SUCCESS;

    daqEntityCount = UINT16(0);
    daqListCount = UINT16(0);
    daqOdtCount = UINT16(0);


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
        Daq_AllocState = XCP_AFTER_ALLOC_DAQ;
    // TODO: Overflow check!!!
        for (idx = daqEntityCount; idx < (daqEntityCount + daqCount); ++idx) {
            daqEntities[idx].kind = XCP_ENTITY_DAQ_LIST;
            daqEntities[idx].entity.daqList.direction = XCP_DIRECTION_DAQ;
            daqEntities[idx].entity.daqList.numOdts = UINT8(0);
        }
        daqListCount += daqCount;
        daqEntityCount += daqCount;
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
        Daq_AllocState = XCP_AFTER_ALLOC_ODT;
        for (idx = daqEntityCount; idx < (daqEntityCount + odtCount); ++idx) {
            daqEntities[idx].kind = XCP_ENTITY_ODT;
        }
        daqEntities[daqListNumber].entity.daqList.numOdts += odtCount;
        daqEntities[daqListNumber].entity.daqList.firstOdt = daqEntityCount;
        daqOdtCount += odtCount;
        daqEntityCount += odtCount;
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
        result = ERR_SUCCESS;
        Daq_AllocState = XCP_AFTER_ALLOC_ODT_ENTRY;
        for (idx = daqEntityCount; idx < (daqEntityCount + odtEntriesCount); ++idx) {
            daqEntities[idx].kind = XCP_ENTITY_ODT_ENTRY;
        }
        odt = daqEntities[daqListNumber].entity.daqList.firstOdt + odtNumber;
        daqEntities[odt].entity.odt.firstOdtEntry = daqEntityCount;
        daqEntities[odt].entity.odt.numOdtEntries = odtEntriesCount;
        daqEntityCount += odtEntriesCount;
    }
    return result;
}

void Xcp_InitDaq(void)
{
    Daq_AllocState = XCP_ALLOC_IDLE;
}

static Xcp_DaqListType * Daq_GetList(uint8_t daqListNumber)
{
    return &daqEntities[daqListNumber].entity.daqList;
}

static Xcp_ODTType * Daq_GetOdt(uint8_t daqListNumber, uint8_t odtNumber)
{
    Xcp_DaqListType const * dl;
    uint8_t num;

    dl = Daq_GetList(daqListNumber);
    num = dl->firstOdt + odtNumber;
    return &daqEntities[num].entity.odt;
}

static Xcp_ODTEntryType * Daq_GetOdtEntry(uint8_t daqListNumber, uint8_t odtNumber, uint8_t odtEntryNumber)
{
    Xcp_ODTType * odt;
    uint8_t num;

    odt = Daq_GetOdt(daqListNumber, odtNumber);
    num = odt->firstOdtEntry + odtEntryNumber;
    return &daqEntities[num].entity.odtEntry;
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
                printf("ODT-ENTRY: [length: %02u  ext: %02X address: %08X]\n", entry->entity.odtEntry.length,
                       entry->entity.odtEntry.mta.ext, entry->entity.odtEntry.mta.address);
                break;
        }
    }
}
#endif

