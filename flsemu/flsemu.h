/*
 * BlueParrot XCP.
 *
 * (C) 2007-2019 by Christoph Schueler <github.com/Christoph2,
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



#ifndef __FLSEMU_H
#define __FLSEMU_H

#define _WIN32_WINNT    0x601
#include <Windows.h>

#if STANDALONE_DLL == 1
#include <stdint.h>
#include <stdbool.h>
#else
#include "xcp.h"
#endif


/*
**  Global Defines.
*/

/*
**  Global Macros.
*/
#define FLSEMU_KB(v)   (((uint32_t)((v))) * 0x400UL)                /**< Kilobytes to bytes. */
#define FLSEMU_MB(v)   (((uint32_t)((v))) * (0x400UL * 0x400UL))    /**< Megabytes to bytes. */


/*
**  Global Types.
*/
typedef enum tagFlsEmu_MemoryTypeType {
    FLSEMU_FLASH,
    FLSEMU_EEPROM,
    FLSEMU_RAM
} FlsEmu_MemoryTypeType;

typedef struct tagFlsEmu_FileViewType {
    HANDLE mappingHandle;
    void * mappingAddress;
} FlsEmu_HwFileViewType;

#if 0
typedef struct tagFlsEmu_HwMapFileType {
    HANDLE handle;
    uint64_t size;
    FlsEmu_HwFileViewType view;
} FlsEmu_MapFieleType;
#endif

typedef enum tagFlsEmu_StatusType {
    FLSEMU_OK,
    FLSEMU_NOT_OK
} FlsEmu_StatusType;


typedef struct tagFlsEmu_PersistentArrayType {
    HANDLE fileHandle;
    HANDLE mappingHandle;
    void * mappingAddress;
    uint16_t currentPage;
} FlsEmu_PersistentArrayType;


typedef enum tagFlsEmu_OpenCreateType {
    OPEN_ERROR,
    OPEN_EXSISTING,
    NEW_FILE
} FlsEmu_OpenCreateResultType;


typedef struct tagFlsEmu_SegmentType {
    char name[1024];
    uint32_t memSize;
    uint8_t  writeSize;
    uint16_t sectorSize;
    uint32_t pageSize;
    uint8_t blockCount;
    uint32_t baseAddress;
    FlsEmu_PersistentArrayType * persistentArray;
    uint8_t currentPage;
} FlsEmu_SegmentType;


typedef struct tagFlsEmu_ConfigType {
    uint8_t numSegments;
    FlsEmu_SegmentType /*const*/ ** segments;
} FlsEmu_ConfigType;



/*
**  Global Functions.
*/
void FlsEmu_Init(FlsEmu_ConfigType const * config);
void FlsEmu_DeInit(void);
void * FlsEmu_BasePointer(uint8_t segmentIdx);
void FlsEmu_SelectPage(uint8_t segmentIdx, uint8_t page);
uint32_t FlsEmu_NumPages(uint8_t segmentIdx);
void FlsEmu_ErasePage(uint8_t segmentIdx, uint8_t page);
void FlsEmu_EraseSector(uint8_t segmentIdx, uint32_t address);
void FlsEmu_EraseBlock(uint8_t segmentIdx, uint16_t block);
Xcp_MemoryMappingResultType FlsEmu_MemoryMapper(Xcp_MtaType * dst, Xcp_MtaType const * src);

#endif // __FLSEMU_H

