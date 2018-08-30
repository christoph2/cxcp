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

#include <stdint.h>
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
                            /* FREE_DAQ    ALLOC_DAQ      ALLOC_ODT      ALLOC_ODT_ENTRY    */
/* ALLOC_IDLE*/             {DAQ_ALLOC_OK, DAQ_ALLOC_ERR, DAQ_ALLOC_ERR, DAQ_ALLOC_ERR },
/* AFTER_FREE_DAQ  */       {DAQ_ALLOC_OK, DAQ_ALLOC_OK , DAQ_ALLOC_ERR, DAQ_ALLOC_ERR },
/* AFTER_ALLOC_DAQ */       {DAQ_ALLOC_OK, DAQ_ALLOC_OK , DAQ_ALLOC_OK,  DAQ_ALLOC_ERR },
/* AFTER_ALLOC_ODT */       {DAQ_ALLOC_OK, DAQ_ALLOC_ERR, DAQ_ALLOC_OK,  DAQ_ALLOC_OK  },
/* AFTER_ALLOC_ODT_ENTRY */ {DAQ_ALLOC_OK, DAQ_ALLOC_ERR, DAQ_ALLOC_ERR, DAQ_ALLOC_OK  },
};

static Daq_AllocStateType Daq_AllocState;
static Xcp_DaqEntityType daqEntities[NUM_DAQ_ENTITIES];
static uint16_t daqEntityCount = 0;
static uint16_t daqListCount = 0;
static uint16_t daqOdtCount = 0;

/*
** Local Function Prototypes.
*/
static Xcp_DaqListType * Daq_GetList(uint8_t daqListNumber);
static Xcp_ODTType * Daq_GetOdt(uint8_t daqListNumber, uint8_t odtNumber);
static Xcp_ODTEntryType * Daq_GetOdtEntry(uint8_t daqListNumber, uint8_t odtNumber, uint8_t odtEntryNumber);

/*
** Local Functions.
*/
bool Daq_AllocValidTransition(Daq_AllocTransitionype transition)
{
    if (Daq_AllocTransitionTable[Daq_AllocState][transition] == DAQ_ALLOC_OK) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void freeDaq(void)
{
    daqEntityCount = 0;
    daqListCount = 0;
    daqOdtCount = 0;

    Xcp_MemSet(daqEntities, 0, sizeof(Xcp_DaqEntityType) * NUM_DAQ_ENTITIES);
    //Xcp_DaqEntityType daqEntities[NUM_DAQ_ENTITIES]

    if (Daq_AllocValidTransition(XCP_CALL_FREE_DAQ)) {
        Daq_AllocState = XCP_AFTER_FREE_DAQ;
    } else {

    }
}

void allocDaq(uint16_t daqCount)
{
    uint16_t idx;

    if (Daq_AllocValidTransition(XCP_CALL_ALLOC_DAQ)) {
        Daq_AllocState = XCP_AFTER_ALLOC_DAQ;
    } else {
        printf("allocDaq() not allowed.\n");
        return;
    }
    // TODO: Overflow check!!!

    for (idx = daqEntityCount; idx < (daqEntityCount + daqCount); ++idx) {
        daqEntities[idx].kind = XCP_ENTITY_DAQ_LIST;
        daqEntities[idx].entity.daqList.direction = XCP_DIRECTION_DAQ;
        daqEntities[idx].entity.daqList.numOdts = 0; // idx;
    }
    daqListCount += daqCount;
    daqEntityCount += daqCount;
}

void allocOdt(uint16_t daqListNumber, uint8_t odtCount)
{
    uint16_t idx;

    if (Daq_AllocValidTransition(XCP_CALL_ALLOC_ODT)) {
        Daq_AllocState = XCP_AFTER_ALLOC_ODT;
    } else {
        printf("allocOdt() not allowed.\n");
        return;
    }

    for (idx = daqEntityCount; idx < (daqEntityCount + odtCount); ++idx) {
        daqEntities[idx].kind = XCP_ENTITY_ODT;
    }
    daqEntities[daqListNumber].entity.daqList.numOdts += odtCount;
    daqEntities[daqListNumber].entity.daqList.firstOdt = daqEntityCount;
    daqOdtCount += odtCount;
    daqEntityCount += odtCount;
}


void allocOdtEntry(uint16_t daqListNumber, uint8_t odtNumber, uint8_t odtEntriesCount)
{
    uint16_t idx;
    uint8_t odt;

    if (Daq_AllocValidTransition(XCP_CALL_ALLOC_ODT_ENTRY)) {
        Daq_AllocState = XCP_AFTER_ALLOC_ODT_ENTRY;
    } else {
        printf("allocOdtEntry() not allowed.\n");
        return;
    }

    for (idx = daqEntityCount; idx < (daqEntityCount + odtEntriesCount); ++idx) {
        daqEntities[idx].kind = XCP_ENTITY_ODT_ENTRY;
    }
    odt = daqEntities[daqListNumber].entity.daqList.firstOdt + odtNumber;
    daqEntities[odt].entity.odt.firstOdtEntry = daqEntityCount;
    daqEntities[odt].entity.odt.numOdtEntries = odtEntriesCount;
    daqEntityCount += odtEntriesCount;
}

void Daq_Init(void)
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
