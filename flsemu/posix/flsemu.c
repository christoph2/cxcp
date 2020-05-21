/*
 * BlueParrot XCP
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

#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>


#include <flsemu.h>
#include <flsemu.h>

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define FLSEMU_ERASED_VALUE (0xff) /**< Value of an erased Flash/EEPROM cell. */

static int FlsEmu_OpenCreatePersitentArray(char const * fileName, uint32_t size, FlsEmu_PersistentArrayType * persistentArray);
static void FlsEmu_ClosePersitentArray(FlsEmu_PersistentArrayType const * persistentArray, uint32_t size);
static void FlsEmu_UnmapAddress(void * addr, uint32_t size);
static void FlsEmu_MapAddress(void * mappingAddress, int offset, uint32_t size, int fd);
static bool FlsEmu_Flush(uint8_t segmentIdx);


uint32_t FlsEmu_GetPageSize(void)
{
    return sysconf(_SC_PAGE_SIZE);
}

void FlsEmu_Close(uint8_t segmentIdx)
{
    FlsEmu_SegmentType const * segment;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    FlsEmu_Flush(segmentIdx);
    FlsEmu_ClosePersitentArray(segment->persistentArray, segment->memSize);
    free(segment->persistentArray);
}

static bool FlsEmu_Flush(uint8_t segmentIdx)
{
    FlsEmu_SegmentType const * segment;

    FLSEMU_ASSERT_INITIALIZED();

    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return XCP_FALSE;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];

    if (msync(segment->persistentArray->mappingAddress, segment->memSize, MS_SYNC) == -1) {
        handle_error("msync");
    }
    return XCP_TRUE;
}

void FlsEmu_OpenCreate(uint8_t segmentIdx)
{
    int result;
    int length;
    char rom[1024];
    FlsEmu_SegmentType * segment;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    segment->persistentArray = (FlsEmu_PersistentArrayType *)malloc(sizeof(FlsEmu_PersistentArrayType));
    segment->currentPage = 0x00;
    length = strlen(segment->name);
    strncpy((char *)rom, (char *)segment->name, length);
    rom[length] = '\x00';
    strcat((char *)rom, ".rom");

    result = FlsEmu_OpenCreatePersitentArray(rom, segment->memSize, segment->persistentArray);
    if (result) {
        XcpUtl_MemSet(segment->persistentArray->mappingAddress, FLSEMU_ERASED_VALUE, segment->memSize);
    }

}

static void FlsEmu_ClosePersitentArray(FlsEmu_PersistentArrayType const * persistentArray, uint32_t size)
{
    FLSEMU_ASSERT_INITIALIZED();

    FlsEmu_UnmapAddress(persistentArray->mappingAddress, size);
    close((int)persistentArray->fileHandle);
}

static void FlsEmu_MapAddress(void * mappingAddress, int offset, uint32_t size, int fd)
{
    void * res;

    res = mmap(mappingAddress, size, PROT_READ | PROT_WRITE,  MAP_SHARED, fd, offset);
    if (res == MAP_FAILED) {
        handle_error("mmap");
    }
}

static void FlsEmu_UnmapAddress(void * addr, uint32_t size)
{
    if (munmap(addr, size) == -1) {
        handle_error("munmap");
    }
}

static int FlsEmu_OpenCreatePersitentArray(char const * fileName, uint32_t size, FlsEmu_PersistentArrayType * persistentArray)
{
    int fd;
    void * addr;

    fd = open(fileName, O_RDWR | O_DIRECT | O_DSYNC, 0666);
    if (fd == -1) {
        if (errno == ENOENT) {
            fd = open(fileName, O_RDWR | O_CREAT | O_TRUNC | O_EXCL | O_DIRECT | O_DSYNC, 0666);
            if (fd == -1) {
                handle_error("creat");
                return -1;
            } else {
                if (fallocate(fd, 0, 0, size) == -1) {
                    handle_error("fallocate");
                    return -1;
                }
            }
        } else {
            handle_error("open");
            return -1;
        }
    }
    persistentArray->fileHandle = (MEM_HANDLE)fd;

    addr = mmap(NULL, size, PROT_READ | PROT_WRITE,  MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        handle_error("mmap");
    }
    printf("ADDR: %p\n", addr);

    persistentArray->mappingAddress = addr;
    persistentArray->mappingHandle = NULL;
    persistentArray->currentPage = 0;

    return 1;
}


void FlsEmu_SelectPage(uint8_t segmentIdx, uint8_t page)
{
    uint32_t offset;
    FlsEmu_SegmentType * segment;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    if (segment->persistentArray->currentPage == page) {
        return; /* Nothing to do. */
    }
    offset = (segment->pageSize * page);
    printf("page# %u offset: %x\n", page, offset);
    FlsEmu_UnmapAddress(segment->persistentArray->mappingAddress, segment->memSize);
    FlsEmu_MapAddress(segment->persistentArray->mappingAddress, offset, segment->memSize, segment->persistentArray->fileHandle);
/*
    if (FlsEmu_MapView(segment, offset, segment->pageSize)) {
        segment->currentPage = page;
    }
*/
}

#if 0
///
/// TESTING
///
#define FLS_SECTOR_SIZE (256)

#define FLS_PAGE_ADDR           ((uint16_t)0x8000U)
#define FLS_PAGE_SIZE           ((uint32_t)0x4000U)


static FlsEmu_SegmentType S12D512_PagedFlash = {
    "XCPSIM_Flash",
    FLSEMU_KB(512),
    2,
    FLS_SECTOR_SIZE,
    FLS_PAGE_SIZE,
    4,
    0x8000,
    XCP_NULL,
    0,
};

static FlsEmu_SegmentType S12D512_EEPROM = {
    "XCPSIM_EEPROM",
    FLSEMU_KB(4),
    2,
    4,
    FLSEMU_KB(4),
    1,
    0x4000,
    XCP_NULL,
    0,
};

static FlsEmu_SegmentType const * segments[] = {
    &S12D512_PagedFlash,
    &S12D512_EEPROM,
};

static const FlsEmu_ConfigType FlsEmu_Config = {
    2,
    (FlsEmu_SegmentType**)segments,
};

int main(void)
{
    int pageSize = FlsEmu_GetPageSize();
    void * ptr;

    printf("PAGE_SIZE: %u\n", pageSize);

    FlsEmu_Init(&FlsEmu_Config);

    for (int i = 0; i < 32; ++i) {
        FlsEmu_SelectPage(0, i);
        ptr = FlsEmu_BasePointer(0);
        //printf("\tBP: %p\n", ptr);
        XcpUtl_MemSet(ptr, i, 16 * 1024);
    }

    FlsEmu_DeInit();
    return 0;
}
#endif

