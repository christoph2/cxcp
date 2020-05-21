/*
 * BlueParrot XCP.
 *
 * (C) 2007-2020 by Christoph Schueler <github.com/Christoph2,
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
#include "flsemu.h"

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
    uint32_t pageSize;
} FlsEmu_SystemMemoryType;

/*
**  Local Variables.
*/
static FlsEmu_ModuleStateType FlsEmu_ModuleState = FLSEMU_UNINIT; /**< Module-state variable. */
static FlsEmu_SystemMemoryType FlsEmu_SystemMemory;     /**< System memory configuration. */
static FlsEmu_ConfigType const * FlsEmu_Config = NULL;  /**< Segment configuration. */


/*
**  Global Functions.
*/


/** @brief Initializes flash-emulator system.
 *
 *
 */
void FlsEmu_Init(FlsEmu_ConfigType const * config)
{
    uint8_t idx;

    FlsEmu_SystemMemory.pageSize = FlsEmu_GetPageSize();
    FlsEmu_Config = config;
    FlsEmu_ModuleState = FLSEMU_INIT;
    for (idx = 0; idx < FlsEmu_GetConfig()->numSegments; ++idx) {
        //printf("FlsEmu_Init-SEG-NAME: %s\n", FlsEmu_Config->segments[idx]->name);
        FlsEmu_OpenCreate(idx);
    }

}


void FlsEmu_DeInit(void)
{
    uint8_t idx;

    for (idx = 0; idx < FlsEmu_GetConfig()->numSegments; ++idx) {
        //printf("UNLOAD-SEG-NAME: %s\n", FlsEmu_Config->segments[idx]->name);
        FlsEmu_Close(idx);
    }
    FlsEmu_ModuleState = FLSEMU_UNINIT;
}


void * FlsEmu_BasePointer(uint8_t segmentIdx)
{
    /// TODO: getSegment()
    FlsEmu_SegmentType const * segment;
    FLSEMU_ASSERT_INITIALIZED();

    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return (void*)NULL;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    //
    return segment->persistentArray->mappingAddress;
}


uint32_t FlsEmu_NumPages(uint8_t segmentIdx)
{
    FlsEmu_SegmentType const * segment;
    FLSEMU_ASSERT_INITIALIZED();

    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return UINT32(0);
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];

    return segment->memSize / segment->pageSize;
}


void FlsEmu_EraseSector(uint8_t segmentIdx, uint32_t address)
{
    uint32_t mask;
    uint16_t * ptr;
    FlsEmu_SegmentType const * segment;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    mask = (uint32_t)segment->sectorSize - 1UL;
    ptr = (uint16_t *)segment->persistentArray->mappingAddress;
    if ((address & mask) != 0UL) {
        // TODO: warn misalignment.
        // ("address (%#X) should be aligned to %u-byte sector boundary.", address, segment->sectorSize)
    }
    XcpUtl_MemSet(ptr + (address & ~mask), FLSEMU_ERASED_VALUE, segment->sectorSize);
}


void FlsEmu_ErasePage(uint8_t segmentIdx, uint8_t page)
{
    uint8_t * ptr = (uint8_t * )FlsEmu_BasePointer(segmentIdx);
    FlsEmu_SegmentType * segment;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    XcpUtl_MemSet(ptr + (segment->pageSize * page), FLSEMU_ERASED_VALUE, segment->pageSize);
    segment->currentPage = page;
}


void FlsEmu_EraseBlock(uint8_t segmentIdx, uint16_t block)
{
    /* TODO: Nur den entsprechenden Block mappen!!! */
    uint8_t * ptr;
    uint32_t offset;
    uint32_t blockSize;
    FlsEmu_SegmentType * segment;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    assert(block < (segment->blockCount));

    blockSize = (segment->memSize / segment->blockCount);
    offset = (blockSize * block);

    ptr = (uint8_t * )FlsEmu_BasePointer(segmentIdx) + offset;
    XcpUtl_MemSet(ptr, FLSEMU_ERASED_VALUE, blockSize);
}

Xcp_MemoryMappingResultType FlsEmu_MemoryMapper(Xcp_MtaType * dst, Xcp_MtaType const * src)
{
    uint8_t idx;
    uint8_t * ptr;
    FlsEmu_SegmentType * segment;

    /* printf("addr: %x ext: %d\n", src->address, src->ext); */

    for (idx = 0; idx < FlsEmu_GetConfig()->numSegments; ++idx) {
        ptr = FlsEmu_BasePointer(idx);
        segment = FlsEmu_GetConfig()->segments[idx];
        if ((src->address >= segment->baseAddress) && (src->address < (segment->baseAddress + segment->pageSize))) {
            if (src->ext >= FlsEmu_NumPages(idx)) {
                return XCP_MEMORY_ADDRESS_INVALID;
            } else {
                FlsEmu_SelectPage(idx, src->ext);
            }
            dst->address = ((uint32_t)ptr - segment->baseAddress) + src->address;
            dst->ext = src->ext;
            /* printf("MAPPED: addr: %x ext: %d TO: %x:%d\n", src->address, src->ext, dst->address, dst->ext); */
            return XCP_MEMORY_MAPPED;
        }
    }
    return XCP_MEMORY_NOT_MAPPED;
}

FlsEmu_ConfigType const * FlsEmu_GetConfig(void)
{
    return FlsEmu_Config;
}

FlsEmu_ModuleStateType * FlsEmu_GetModuleState(void)
{
    return &FlsEmu_ModuleState;
}
