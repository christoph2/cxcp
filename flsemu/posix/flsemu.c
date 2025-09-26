/*
 * BlueParrot XCP
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
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "flsemu.h"
/*!!! END-INCLUDE-SECTION !!!*/

#if defined(__APPLE__)
    #define O_DIRECT (0)
#endif

#define handle_error(msg)                                                                                                          \
    do {                                                                                                                           \
        perror(msg);                                                                                                               \
        exit(EXIT_FAILURE);                                                                                                        \
    } while (0)
#define FLSEMU_ERASED_VALUE (0xff) /**< Value of an erased Flash/EEPROM cell. */

static void FlsEmu_ClosePersitentArray(FlsEmu_PersistentArrayType const * persistentArray, uint32_t size);
static bool FlsEmu_Flush(uint8_t segmentIdx);

uint32_t FlsEmu_GetAllocationGranularity(void) {
    return sysconf(_SC_PAGE_SIZE); /* don't use getpagesize() for portable appz */
}

void FlsEmu_Close(uint8_t segmentIdx) {
    FlsEmu_SegmentType const * segment = XCP_NULL;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    FlsEmu_Flush(segmentIdx);
    FlsEmu_ClosePersitentArray(segment->persistentArray, segment->memSize);
    if (segment->persistentArray) {
        free(segment->persistentArray);
    }
}

static bool FlsEmu_Flush(uint8_t segmentIdx) {
    FlsEmu_SegmentType const * segment = XCP_NULL;

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

static void FlsEmu_ClosePersitentArray(FlsEmu_PersistentArrayType const * persistentArray, uint32_t size) {
    FLSEMU_ASSERT_INITIALIZED();

    FlsEmu_UnmapAddress(persistentArray->mappingAddress, size);
    close((uint32_t)persistentArray->fileHandle);
}

static void FlsEmu_MapAddress(void* mappingAddress, int offset, uint32_t size, int fd) {
    void* res = XCP_NULL;

    res = mmap(mappingAddress, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    if (res == MAP_FAILED) {
        handle_error("mmap");
    }
}

static void FlsEmu_UnmapAddress(void* addr, uint32_t size) {
    if (munmap(addr, size) == -1) {
        handle_error("munmap");
    }
}

FlsEmu_OpenCreateResultType FlsEmu_OpenCreatePersitentArray(
    char const * fileName, uint32_t size, FlsEmu_PersistentArrayType* persistentArray
) {
    int                         fd      = 0;
    void*                       addr    = XCP_NULL;
    FlsEmu_OpenCreateResultType result  = OPEN_ERROR;
    bool                        newFile = XCP_FALSE;

    /* Open existing file without fragile O_DIRECT/O_DSYNC flags for portability */
    fd = open(fileName, O_RDWR, 0666);
    if (fd == -1) {
        if (errno == ENOENT) {
            fd = open(fileName, O_RDWR | O_CREAT | O_TRUNC | O_EXCL, 0666);
            if (fd == -1) {
                handle_error("open(O_CREAT)");
                return OPEN_ERROR;
            } else {
#if defined(__APPLE__)
                fstore_t store = { F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, (off_t)size };
                int      ret   = fcntl(fd, F_PREALLOCATE, &store);
                if (-1 == ret) {
                    store.fst_flags = F_ALLOCATEALL;
                    ret             = fcntl(fd, F_PREALLOCATE, &store);
                }
                if (-1 == ret) {
                    if (ftruncate(fd, (off_t)size) == -1) {
                        handle_error("ftruncate");
                        close(fd);
                        return OPEN_ERROR;
                    }
                }
#elif defined(__linux__)
                if (posix_fallocate(fd, 0, (off_t)size) != 0) {
                    if (ftruncate(fd, (off_t)size) == -1) {
                        handle_error("ftruncate");
                        close(fd);
                        return OPEN_ERROR;
                    }
                }
#else
                if (ftruncate(fd, (off_t)size) == -1) {
                    handle_error("ftruncate");
                    close(fd);
                    return OPEN_ERROR;
                }
#endif
                newFile = XCP_TRUE;
            }
        } else {
            handle_error("open");
            return OPEN_ERROR;
        }
    } else {
        newFile = XCP_FALSE;
    }
    persistentArray->fileHandle = (MEM_HANDLE)fd;

    addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        handle_error("mmap");
    }

    persistentArray->mappingAddress = addr;
    persistentArray->mappingHandle  = NULL;
    persistentArray->currentPage    = 0;

    result = newFile ? NEW_FILE : OPEN_EXSISTING;
    return result;
}

void FlsEmu_SelectPage(uint8_t segmentIdx, uint8_t page) {
    FlsEmu_SegmentType* segment = XCP_NULL;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    /* On POSIX we keep the entire file mapped; selecting a page only updates metadata. */
    segment->currentPage = page;
}
