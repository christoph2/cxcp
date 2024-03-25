/*
 * BlueParrot XCP.
 *
 * (C) 2007-2021 by Christoph Schueler <github.com/Christoph2,
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "flsemu.h"

/*!!! END-INCLUDE-SECTION !!!*/

/*
**  Local Defines.
*/

/*
**  Local Types.
*/

typedef enum tagFlsEmu_MemoryTypeType {
    FLSEMU_FLASH,
    FLSEMU_EEPROM,
    FLSEMU_RAM
} FlsEmu_MemoryTypeType;

/** @brief Information about your system memory.
 *
 *
 */
typedef struct tagFlsEmu_SystemMemoryType {
    uint32_t AllocationGranularity;
} FlsEmu_SystemMemoryType;

/*
**  Local Variables.
*/
static FlsEmu_ModuleStateType   FlsEmu_ModuleState = FLSEMU_UNINIT; /**< Module-state variable. */
static FlsEmu_SystemMemoryType  FlsEmu_SystemMemory;                /**< System memory configuration. */
static FlsEmu_ConfigType const *FlsEmu_Config = XCP_NULL;           /**< Segment configuration. */

/*
**  Global Functions.
*/

/** @brief Initializes flash-emulator system.
 *
 *
 */
void FlsEmu_Init(FlsEmu_ConfigType const *config) {
    uint8_t idx = 0;

    FlsEmu_SystemMemory.AllocationGranularity = FlsEmu_GetAllocationGranularity();
    FlsEmu_Config                             = config;
    FlsEmu_ModuleState                        = FLSEMU_INIT;
    for (idx = 0; idx < FlsEmu_GetConfig()->numSegments; ++idx) {
        FlsEmu_OpenCreate(idx);
    }
}

void FlsEmu_DeInit(void) {
    uint8_t idx = 0;

    for (idx = 0; idx < FlsEmu_GetConfig()->numSegments; ++idx) {
        // printf("UNLOAD-SEG-NAME: %s\n", FlsEmu_Config->segments[idx]->name);
        FlsEmu_Close(idx);
    }
    FlsEmu_ModuleState = FLSEMU_UNINIT;
}

void FlsEmu_OpenCreate(uint8_t segmentIdx) {
    uint32_t                    length = 0;
    char                        rom[1024];
    FlsEmu_SegmentType         *segment    = XCP_NULL;
    FlsEmu_OpenCreateResultType result     = 0;
    uint32_t                    fillerSize = 0UL;
    Xcp_PointerSizeType         offset     = 0UL;
    uint32_t                    numPages   = 0U;
    uint32_t                    pageIdx    = 0U;
    uint32_t                    pageSize   = 0U;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    for (pageIdx = 0U; pageIdx < numPages; ++pageIdx) {
        // FIXME!!!
        offset = ((Xcp_PointerSizeType)segment->persistentArray->mappingAddress) + (pageIdx * pageSize);
        XcpUtl_MemSet((void *)offset, FLSEMU_ERASED_VALUE, segment->pageSize);
        if (fillerSize > 0) {
            XcpUtl_MemSet((void *)(offset + segment->pageSize), FLSEMU_FILLER_VALUE, fillerSize);
        }
        segment                   = FlsEmu_GetConfig()->segments[segmentIdx];
        segment->persistentArray  = (FlsEmu_PersistentArrayType *)malloc(sizeof(FlsEmu_PersistentArrayType));
        segment->currentPage      = 0x00;
        segment->alloctedPageSize = FlsEmu_AllocatedSize(segmentIdx);
        pageSize                  = XCP_MAX(segment->pageSize, segment->alloctedPageSize);
        length                    = strlen(segment->name);
#if defined(_MSC_VER)
        strncpy_s((char *)rom, 1024, (char *)segment->name, length);
        rom[length] = '\x00';
        strcat_s((char *)rom, 1024, ".rom");
#else
        strncpy((char *)rom, (char *)segment->name, length);
        rom[length] = '\x00';
        strcat((char *)rom, ".rom");
#endif /* _MSC_VER */
        numPages = FlsEmu_NumPages(segmentIdx);
        result   = FlsEmu_OpenCreatePersitentArray(rom, pageSize * numPages, segment->persistentArray);
        if (result == OPEN_ERROR) {
        } else if (result == NEW_FILE) {
            if (segment->alloctedPageSize > segment->pageSize) {
                fillerSize = segment->alloctedPageSize - segment->pageSize;
            } else {
                fillerSize = 0;
            }
            for (pageIdx = 0U; pageIdx < numPages; ++pageIdx) {
                // FIXME!!!
                offset = ((Xcp_PointerSizeType)segment->persistentArray->mappingAddress) + (pageIdx * pageSize);
                XcpUtl_MemSet((void *)offset, FLSEMU_ERASED_VALUE, segment->pageSize);
                if (fillerSize > 0) {
                    XcpUtl_MemSet((void *)(offset + segment->pageSize), FLSEMU_FILLER_VALUE, fillerSize);
                }
            }
        }
    }
}

void *FlsEmu_BasePointer(uint8_t segmentIdx) {
    /// TODO: getSegment()
    FlsEmu_SegmentType const *segment = XCP_NULL;
    FLSEMU_ASSERT_INITIALIZED();

    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return XCP_NULL;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    return segment->persistentArray->mappingAddress;
}

uint32_t FlsEmu_NumPages(uint8_t segmentIdx) {
    FlsEmu_SegmentType const *segment = XCP_NULL;
    FLSEMU_ASSERT_INITIALIZED();

    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return UINT32(0);
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];

    return segment->memSize / segment->pageSize;
}

void FlsEmu_EraseSector(uint8_t segmentIdx, uint32_t address) {
    uint32_t                  mask    = 0UL;
    uint16_t                 *ptr     = XCP_NULL;
    FlsEmu_SegmentType const *segment = XCP_NULL;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    mask    = (uint32_t)segment->sectorSize - 1UL;
    ptr     = (uint16_t *)segment->persistentArray->mappingAddress;
    if ((address & mask) != 0UL) {
        // TODO: warn misalignment.
        // ("address (%#X) should be aligned to %u-byte sector boundary.", address,
        // segment->sectorSize)
    }
    XcpUtl_MemSet(ptr + (address & ~mask), FLSEMU_ERASED_VALUE, segment->sectorSize);
}

void FlsEmu_ErasePage(uint8_t segmentIdx, uint8_t page) {
    uint8_t            *ptr     = (uint8_t *)FlsEmu_BasePointer(segmentIdx);
    FlsEmu_SegmentType *segment = XCP_NULL;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    XcpUtl_MemSet(ptr + (segment->pageSize * page), FLSEMU_ERASED_VALUE, segment->pageSize);
    segment->currentPage = page;
}

void FlsEmu_EraseBlock(uint8_t segmentIdx, uint16_t block) {
    /* TODO: Nur den entsprechenden Block mappen!!! */
    uint8_t                  *ptr       = XCP_NULL;
    uint32_t                  offset    = 0UL;
    uint32_t                  blockSize = 0UL;
    FlsEmu_SegmentType const *segment   = XCP_NULL;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    assert(block < (segment->blockCount));

    blockSize = (segment->memSize / segment->blockCount);
    offset    = (blockSize * block);

    ptr = (uint8_t *)FlsEmu_BasePointer(segmentIdx) + offset;
    XcpUtl_MemSet(ptr, FLSEMU_ERASED_VALUE, blockSize);
}

Xcp_MemoryMappingResultType FlsEmu_MemoryMapper(Xcp_MtaType *dst, Xcp_MtaType const *src) {
    uint8_t                   idx     = 0;
    uint8_t                  *ptr     = XCP_NULL;
    FlsEmu_SegmentType const *segment = XCP_NULL;

    /* printf("addr: %x ext: %d\n", src->address, src->ext); */

    for (idx = 0; idx < FlsEmu_GetConfig()->numSegments; ++idx) {
        ptr     = FlsEmu_BasePointer(idx);
        segment = FlsEmu_GetConfig()->segments[idx];
        if ((src->address >= segment->baseAddress) && (src->address < (segment->baseAddress + segment->pageSize))) {
            if (src->ext >= FlsEmu_NumPages(idx)) {
                return XCP_MEMORY_ADDRESS_INVALID;
            } else {
                FlsEmu_SelectPage(idx, src->ext);
            }
            dst->address = ((Xcp_PointerSizeType)ptr - segment->baseAddress) + src->address;
            dst->ext     = src->ext;
            /* printf("MAPPED: addr: %x ext: %d TO: %x:%d\n", src->address, src->ext,
             * dst->address, dst->ext); */
            return XCP_MEMORY_MAPPED;
        }
    }
    return XCP_MEMORY_NOT_MAPPED;
}

/*!
 *  Align emulated page-size to OS allocation granularity.
 */
uint32_t FlsEmu_AllocatedSize(uint8_t segmentIdx) {
    FlsEmu_SegmentType const *segment = XCP_NULL;
    FLSEMU_ASSERT_INITIALIZED();

    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return (uint32_t)0;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];

    return FlsEmu_SystemMemory.AllocationGranularity * (((segment->pageSize / FlsEmu_SystemMemory.AllocationGranularity) +
                                                         ((segment->pageSize % FlsEmu_SystemMemory.AllocationGranularity) != 0)) ?
                                                            1 :
                                                            0);
}

FlsEmu_ConfigType const *FlsEmu_GetConfig(void) {
    return FlsEmu_Config;
}

FlsEmu_ModuleStateType *FlsEmu_GetModuleState(void) {
    return &FlsEmu_ModuleState;
}

bool FlsEmu_Initialized(void) {
    FlsEmu_ModuleStateType const *state = FlsEmu_GetModuleState();

    return *state == FLSEMU_INIT;
}
