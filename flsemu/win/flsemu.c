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

#include <assert.h>
#include <string.h>

#include "flsemu.h"
#include "xcp_hw.h"

#define _WIN32_WINNT    0x601
#include <Windows.h>

#if _MSC_VER
#pragma warning(disable: 4996)
#endif /* _MSC_VER */

/*
**  Local Defines.
*/


/*
**  Local Types.
*/

typedef struct tagFlsEmu_FileViewType {
    MEM_HANDLE mappingHandle;
    void * mappingAddress;
} FlsEmu_HwFileViewType;


typedef enum tagFlsEmu_StatusType {
    FLSEMU_OK,
    FLSEMU_NOT_OK
} FlsEmu_StatusType;


typedef enum tagFlsEmu_OpenCreateType {
    OPEN_ERROR,
    OPEN_EXSISTING,
    NEW_FILE
} FlsEmu_OpenCreateResultType;


/*
**  Local Function Prototypes.
*/
static FlsEmu_OpenCreateResultType FlsEmu_OpenCreatePersitentArray(char const * fileName, DWORD size, FlsEmu_PersistentArrayType * persistentArray);
static bool FlsEmu_Flush(uint8_t segmentIdx);
static void FlsEmu_CloseFileView(FlsEmu_HwFileViewType * fileView);
static void FlsEmu_ClosePersitentArray(FlsEmu_PersistentArrayType const * persistentArray);
static bool FlsEmu_MapView(FlsEmu_SegmentType * config, uint32_t offset, uint32_t length);
static void MemoryInfo(void * address);
static MEM_HANDLE OpenCreateFile(char const * fileName, bool create);
static bool CreateFileView(MEM_HANDLE handle, DWORD length, FlsEmu_HwFileViewType * fileView);

/*
**  Local Variables.
*/

/*
**  Global Functions.
*/

void FlsEmu_OpenCreate(uint8_t segmentIdx)
{
    int length;
    char rom[1024];
    FlsEmu_SegmentType * segment;
    FlsEmu_OpenCreateResultType result;

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
    if (result == OPEN_ERROR) {

    } else if (result == NEW_FILE) {
        XcpUtl_MemSet(segment->persistentArray->mappingAddress, FLSEMU_ERASED_VALUE, segment->memSize);
    }
}


/** @brief Flush memory arrray and close file.
 *
 * @param segmentIdx
 *
 */
void FlsEmu_Close(uint8_t segmentIdx)
{
    FlsEmu_SegmentType const * segment;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    FlsEmu_Flush(segmentIdx);
    FlsEmu_ClosePersitentArray(segment->persistentArray);
    free(segment->persistentArray);
}


/** @brief Flushes, i.e. writes data to disk.
 *
 * @param segmentIdx
 * @return TRUE in successful otherwise FALSE.
 *
 */
static bool FlsEmu_Flush(uint8_t segmentIdx)
{
    MEMORY_BASIC_INFORMATION  info;
    FlsEmu_SegmentType const * segment;

    FLSEMU_ASSERT_INITIALIZED();

    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return FALSE;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];

    if (VirtualQuery(segment->persistentArray->mappingAddress, &info, sizeof(MEMORY_BASIC_INFORMATION)) == 0) {
        XcpHw_ErrorMsg("FlsEmu_Flush::VirtualQuery()", GetLastError());
        return FALSE;
    }

    if (!FlushViewOfFile(segment->persistentArray->mappingAddress, info.RegionSize)) {
        XcpHw_ErrorMsg("FlsEmu_Flush::FlushViewOfFile()", GetLastError());
        return FALSE;
    }

    if (!FlushFileBuffers(segment->persistentArray->fileHandle)) {
        XcpHw_ErrorMsg("FlsEmu_Flush::FlushFileBuffers()", GetLastError());
        return FALSE;
    }

    return TRUE;
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
    if (FlsEmu_MapView(segment, offset, segment->pageSize)) {
        segment->currentPage = page;
    }
}

#if 0
static void FlsEmu_SelectBlock(uint8_t segmentIdx, uint8_t block)
{
    uint32_t offset, blockSize;
    FlsEmu_SegmentType * segment;

    FLSEMU_ASSERT_INITIALIZED();
    if (!FLSEMU_VALIDATE_SEGMENT_IDX(segmentIdx)) {
        return;
    }
    segment = FlsEmu_GetConfig()->segments[segmentIdx];
    blockSize = (segment->memSize / segment->blockCount);
    offset = (blockSize * block);

     if (FlsEmu_MapView(segment, offset, blockSize)) {

    }
}
#endif


#if 0
bool XcpOw_MapFileOpen(char const * fname, Xcp_HwMapFileType * mf)
{
    LARGE_INTEGER size;

    mf->handle = OpenCreateFile(fname, FALSE);
    if (mf->handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    GetFileSizeEx(mf->handle, &size);

    mf->size = size.QuadPart;

    if (!CreateFileView(mf->handle, size.QuadPart, &mf->view)) {
        return FALSE;
    }
    return TRUE;
}

void XcpOw_MapFileClose(Xcp_HwMapFileType const * mf)
{
    CloseHandle(mf->view.mappingHandle);
    CloseHandle(mf->handle);
}
#endif  // 0


/*
**  Local Functions.
*/
static bool FlsEmu_MapView(FlsEmu_SegmentType * config, uint32_t offset, uint32_t length)
{
    DWORD error;

    FLSEMU_ASSERT_INITIALIZED();
//    assert((offset % FlsEmu_SystemMemory.pageSize) == 0);   /*  Offset must be a multiple of the allocation granularity! */

    error = UnmapViewOfFile(config->persistentArray->mappingAddress);
    if (error == 0UL) {
        XcpHw_ErrorMsg("FlsEmu_MapView::MapViewOfFile()", GetLastError());
        CloseHandle(config->persistentArray->mappingHandle);
        return FALSE;
    }

    config->persistentArray->mappingAddress = (void *)MapViewOfFile(config->persistentArray->mappingHandle, FILE_MAP_ALL_ACCESS, 0, offset, length);
    if (config->persistentArray->mappingAddress == NULL) {
        XcpHw_ErrorMsg("FlsEmu_MapView::MapViewOfFile()", GetLastError());
        CloseHandle(config->persistentArray->mappingHandle);
        return FALSE;
    }
    return TRUE;
}

static FlsEmu_OpenCreateResultType FlsEmu_OpenCreatePersitentArray(char const * fileName, DWORD size, FlsEmu_PersistentArrayType * persistentArray)
{
    DWORD error;
    FlsEmu_HwFileViewType fileView;
    FlsEmu_OpenCreateResultType result;
    bool newFile = FALSE;

    FLSEMU_ASSERT_INITIALIZED();
    persistentArray->fileHandle = OpenCreateFile(fileName, FALSE);
    if (persistentArray->fileHandle == INVALID_HANDLE_VALUE) {
        error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            newFile = TRUE;
            persistentArray->fileHandle = OpenCreateFile(fileName, TRUE);
            if (persistentArray->fileHandle == INVALID_HANDLE_VALUE) {
                return OPEN_ERROR;
            }
        } else {
            return OPEN_ERROR;
        }
    }

    if (!CreateFileView(persistentArray->fileHandle, size, &fileView)) {
        return OPEN_ERROR;
    }

    persistentArray->mappingAddress = fileView.mappingAddress;
    persistentArray->mappingHandle  = fileView.mappingHandle;

    MemoryInfo(persistentArray->mappingAddress);

    if (newFile) {
        result = NEW_FILE;
    } else {
        result = OPEN_EXSISTING;
    }
    return result;
}

static void FlsEmu_ClosePersitentArray(FlsEmu_PersistentArrayType const * persistentArray)
{
    FLSEMU_ASSERT_INITIALIZED();
    UnmapViewOfFile(persistentArray->mappingAddress);
    CloseHandle(persistentArray->mappingHandle);
    CloseHandle(persistentArray->fileHandle);
}

/*
**  Wrappers for Windows Functions.
*/
uint32_t FlsEmu_GetPageSize(void)
{
    SYSTEM_INFO info;

    GetSystemInfo(&info);
    return info.dwPageSize;
}

#if 0
static bool FileExits(char * const name)
{
    DWORD attribs = GetFileAttributes(name);

    return (attribs != INVALID_FILE_ATTRIBUTES) && !(attribs & FILE_ATTRIBUTE_DIRECTORY);
}
#endif // 0

/** @brief Open or create a file.
 *
 * @param fileName
 * @param create
 * @return MEM_HANDLE of file
 *
 */
static MEM_HANDLE OpenCreateFile(char const * fileName, bool create)
{
    MEM_HANDLE handle;
    DWORD dispoition = (create == TRUE) ? CREATE_NEW : OPEN_EXISTING;

    handle = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        (LPSECURITY_ATTRIBUTES)NULL, dispoition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, (MEM_HANDLE)NULL
    );
    if (handle == INVALID_HANDLE_VALUE) {
        XcpHw_ErrorMsg("OpenCreateFile::CreateFile()", GetLastError());
    }

    return handle;
}

static bool CreateFileView(MEM_HANDLE handle, DWORD length, FlsEmu_HwFileViewType * fileView)
{
    fileView->mappingHandle = CreateFileMapping(handle, (LPSECURITY_ATTRIBUTES)NULL, PAGE_READWRITE, (DWORD)0, (DWORD)length, NULL);
    if (fileView->mappingHandle == NULL) {
        return FALSE;
    }

    /* TODO: Refactor to function; s. FlsEmu_MapView() */
    fileView->mappingAddress = (void *)MapViewOfFile(fileView->mappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, length);
    if (fileView->mappingAddress == NULL) {
        XcpHw_ErrorMsg("CreateFileView::MapViewOfFile()", GetLastError());
        CloseHandle(fileView->mappingHandle);
        return FALSE;
    }

    return TRUE;
}

static void MemoryInfo(void * address)
{
    MEMORY_BASIC_INFORMATION info;

    VirtualQuery(address, &info, sizeof(MEMORY_BASIC_INFORMATION));
}


static void FlsEmu_CloseFileView(FlsEmu_HwFileViewType * fileView)
{
    CloseHandle(fileView->mappingHandle);
}


#if defined(_WIN32)
void FlsEmu_Info(void)
{
    uint8_t idx;
    uint8_t * ptr;
    FlsEmu_SegmentType * segment;

    printf("\nFlash-Emulator\n");
    printf("--------------\n");
    printf("Segment              Mapped     Virtual    Size(KB) Pagesize(KB) #Pages\n");
    for (idx = 0; idx < FlsEmu_GetConfig()->numSegments; ++idx) {
        ptr = FlsEmu_BasePointer(idx);
        segment = FlsEmu_GetConfig()->segments[idx];
        printf("%-20.20s 0x%p 0x%p %8d         %4d %6d\n", segment->name, (void*)segment->baseAddress, ptr, segment->memSize / 1024, segment->pageSize / 1024, FlsEmu_NumPages(idx));
    }
    printf("\n");

}
#endif
