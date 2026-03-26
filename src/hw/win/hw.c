/*
 * BlueParrot XCP
 *
 * (C) 2007-2022 by Christoph Schueler <github.com/Christoph2,
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

#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_hw.h"

/*!!! END-INCLUDE-SECTION !!!*/

/*
**  Local Types.
*/
typedef struct tagHwStateType {
    LARGE_INTEGER StartingTime;
    LARGE_INTEGER TicksPerSecond;
    bool          Initialized;
} HwStateType;

/*
** Local Defines.
*/
#define TIMER_PS_1US   (1000000UL)
#define TIMER_PS_10US  (100000UL)
#define TIMER_PS_100US (10000UL)

#define TIMER_PS_1MS   (1000UL)
#define TIMER_PS_10MS  (100UL)
#define TIMER_PS_100MS (10UL)
#define TIMER_PS_1S    (1UL)

/* Set timer prescaler value. */
#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_1US)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10US
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_10US)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100US
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_100US)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_1MS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10MS
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_10MS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100MS
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_100MS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1S
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_1S)
#else
    #error Timestamp-unit not supported.
#endif  // XCP_DAQ_TIMESTAMP_UNIT

#define TIMER_MASK_1 (0x000000FFUL)
#define TIMER_MASK_2 (0x0000FFFFUL)
#define TIMER_MASK_4 (0xFFFFFFFFUL)

/*
**  Local Function Prototypes.
*/
static void XcpHw_InitLocks(void);

static void XcpHw_DeinitLocks();

static uint64_t XcpHw_GetElapsedTime(uint32_t prescaler);

/*
**  External Function Prototypes.
*/
void XcpTl_PrintConnectionInformation(void);

void FlsEmu_Info(void);

void XcpDaq_Info(void);

void XcpDaq_PrintDAQDetails();

bool XcpDaq_QueueEmpty(void);

bool XcpDaq_QueueDequeue(uint16_t *len, uint8_t *data);

void XcpHw_MainFunction(void);

void exitFunc(void);

/*
**  Local Variables.
*/
static HwStateType HwState = { 0 };
CRITICAL_SECTION   XcpHw_Locks[XCP_HW_LOCK_COUNT];

/*
**  Global Functions.
*/
void XcpHw_Init(void) {
    fflush(stdout);
    QueryPerformanceFrequency(&HwState.TicksPerSecond);
    QueryPerformanceCounter(&HwState.StartingTime);
    HwState.Initialized = (HwState.TicksPerSecond.QuadPart != 0);
    XcpHw_InitLocks();
}

void XcpHw_Deinit(void) {
    XcpHw_DeinitLocks();
    HwState.Initialized = false;
}

static uint64_t XcpHw_GetElapsedTime(uint32_t prescaler) {
    LARGE_INTEGER Now;
    LARGE_INTEGER Elapsed;

    if (!HwState.Initialized || HwState.TicksPerSecond.QuadPart == 0 || prescaler == 0U) {
        return 0ULL;
    }

    QueryPerformanceCounter(&Now);
    Elapsed.QuadPart = Now.QuadPart - HwState.StartingTime.QuadPart;
    /* Avoid divide-by-zero and improve precision by multiplying first. */
    Elapsed.QuadPart = (Elapsed.QuadPart * (int64_t)prescaler) / HwState.TicksPerSecond.QuadPart;
    return (uint64_t)Elapsed.QuadPart;
}

uint32_t XcpHw_GetTimerCounter(void) {
    uint64_t Now = XcpHw_GetElapsedTime(XCP_HW_TIMER_PRESCALER);

#if XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_1
    return (uint32_t)Now & TIMER_MASK_1;
#elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_2
    return (uint32_t)Now & TIMER_MASK_2;
#elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_4
    return (uint32_t)Now & TIMER_MASK_4;
#else
    #error Timestamp-size not supported.
#endif  // XCP_DAQ_TIMESTAMP_SIZE
}

uint32_t XcpHw_GetTimerCounterMS(void) {
    return (uint32_t)((XcpHw_GetElapsedTime(TIMER_PS_1MS)) & TIMER_MASK_4);
}

static void XcpHw_InitLocks(void) {
    uint8_t idx = UINT8(0);

    for (idx = UINT8(0); idx < XCP_HW_LOCK_COUNT; ++idx) {
        InitializeCriticalSection(&XcpHw_Locks[idx]);
    }
}

static void XcpHw_DeinitLocks(void) {
    uint8_t idx = UINT8(0);

    for (idx = UINT8(0); idx < XCP_HW_LOCK_COUNT; ++idx) {
        DeleteCriticalSection(&XcpHw_Locks[idx]);
    }
}

void XcpHw_AcquireLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    EnterCriticalSection(&XcpHw_Locks[lockIdx]);
}

void XcpHw_ReleaseLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    LeaveCriticalSection(&XcpHw_Locks[lockIdx]);
}

void XcpHw_ErrorMsg(char * const fun, int errorCode) {
    char buffer[1024];

    ZeroMemory(buffer, sizeof(buffer));
    DWORD len = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, (DWORD)errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, (DWORD)sizeof(buffer), NULL
    );
    if (len == 0) {
        fprintf(stderr, "[%s] failed with: [%d]\n", fun, errorCode);
        return;
    }
    fprintf(stderr, "[%s] failed with: [%d] %s", fun, errorCode, buffer);
    fflush(stderr);
}

/*
 *
 * Sleep for `usec` microseconds.
 *
 */
void XcpHw_Sleep(uint64_t usec) {
    HANDLE        timer;
    LARGE_INTEGER ft;

    ZeroMemory(&ft, sizeof(LARGE_INTEGER));
    ft.QuadPart = -(10 * (int64_t)usec);
    timer       = CreateWaitableTimer(NULL, TRUE, NULL);
    if (timer == NULL) {
        /* Fallback: millisekundengenau schlafen */
        DWORD ms = (DWORD)(usec / 1000ULL);
        if (ms == 0 && usec > 0) {
            ms = 1;
        }
        Sleep(ms);
        return;
    }
    if (!SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0)) {
        CloseHandle(timer);
        DWORD ms = (DWORD)(usec / 1000ULL);
        if (ms == 0 && usec > 0) {
            ms = 1;
        }
        Sleep(ms);
        return;
    }
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}

/*
**  Calibration/Paging Services (PAG).
*/
#if (XCP_ENABLE_CAL_COMMANDS == XCP_ON) || (XCP_ENABLE_PAG_COMMANDS == XCP_ON)

    #include "flsemu.h"

bool XcpHw_SetCalPage(uint8_t segment, uint8_t page, uint8_t mode) {
    if (!FlsEmu_Initialized()) {
        return false;
    }
    if (segment >= FlsEmu_GetConfig()->numSegments) {
        return false;
    }
    if (page >= FlsEmu_NumPages(segment)) {
        return false;
    }
    /* Mode: 0x01 = ECU, 0x02 = XCP, 0x03 = both. In this emulation we only have one active page per segment. */
    FlsEmu_SelectPage(segment, page);
    return true;
}

bool XcpHw_GetCalPage(uint8_t segment, uint8_t mode, uint8_t *page) {
    if (!FlsEmu_Initialized()) {
        return false;
    }
    if (segment >= FlsEmu_GetConfig()->numSegments) {
        return false;
    }
    *page = FlsEmu_GetConfig()->segments[segment]->currentPage;
    return true;
}

bool XcpHw_GetPagProcessorInfo(XcpHw_PagProcessorInfoType *info) {
    if (!FlsEmu_Initialized()) {
        return false;
    }
    info->maxSegment = FlsEmu_GetConfig()->numSegments;
    info->properties = 0x01; /* FREEZE_SUPPORTED */
    return true;
}

bool XcpHw_GetSegmentInfo(uint8_t segment, uint8_t mode, XcpHw_SegmentInfoType *info) {
    FlsEmu_SegmentType const *seg;
    (void)mode;
    if (!FlsEmu_Initialized()) {
        return false;
    }
    if (segment >= FlsEmu_GetConfig()->numSegments) {
        return false;
    }
    seg           = FlsEmu_GetConfig()->segments[segment];
    info->address = (uint32_t)seg->baseAddress;
    info->length  = (uint32_t)seg->memSize;
    if (seg->type == FLSEMU_RAM) {
        info->numPages = 1;
    } else {
        info->numPages = (uint8_t)FlsEmu_NumPages(segment);
    }
    info->addressExtension  = seg->addressExtension;
    info->maxPage           = info->numPages - 1;
    info->compressionMethod = 0;
    info->encryptionMethod  = 0;
    return true;
}

bool XcpHw_GetPageInfo(uint8_t segment, uint8_t page, XcpHw_PageInfoType *info) {
    if (!FlsEmu_Initialized()) {
        return false;
    }
    if (segment >= FlsEmu_GetConfig()->numSegments) {
        return false;
    }
    if (page >= FlsEmu_NumPages(segment)) {
        return false;
    }
    info->properties  = 0x03; /* ECU_ACCESS_READ_WRITE | XCP_ACCESS_READ_WRITE */
    info->initSegment = 0;
    return true;
}

bool XcpHw_SetSegmentMode(uint8_t segment, uint8_t mode) {
    /* Not implemented in flsemu */
    return true;
}

bool XcpHw_GetSegmentMode(uint8_t segment, uint8_t *mode) {
    *mode = 0; /* Normal mode */
    return true;
}

bool XcpHw_CopyCalPage(uint8_t srcSegment, uint8_t srcPage, uint8_t dstSegment, uint8_t dstPage) {
    FlsEmu_SegmentType *srcSeg;
    FlsEmu_SegmentType *dstSeg;
    uint8_t            *srcPtr;
    uint8_t            *dstPtr;

    if (!FlsEmu_Initialized()) {
        return false;
    }
    if (srcSegment >= FlsEmu_GetConfig()->numSegments || dstSegment >= FlsEmu_GetConfig()->numSegments) {
        return false;
    }
    if (srcPage >= FlsEmu_NumPages(srcSegment) || dstPage >= FlsEmu_NumPages(dstSegment)) {
        return false;
    }
    srcSeg = FlsEmu_GetConfig()->segments[srcSegment];
    dstSeg = FlsEmu_GetConfig()->segments[dstSegment];

    if (srcSeg->pageSize != dstSeg->pageSize) {
        return false;
    }

    /* We need to be careful with currently mapped pages.
       FlsEmu_BasePointer returns the pointer to the CURRENTLY mapped page.
       This is a bit tricky since we might want to copy from/to pages that are NOT currently mapped.
    */
    /* Temporary hack: Select pages, copy, then restore. */
    uint8_t oldSrcPage = srcSeg->currentPage;
    uint8_t oldDstPage = dstSeg->currentPage;

    FlsEmu_SelectPage(srcSegment, srcPage);
    srcPtr = (uint8_t *)FlsEmu_BasePointer(srcSegment);

    FlsEmu_SelectPage(dstSegment, dstPage);
    dstPtr = (uint8_t *)FlsEmu_BasePointer(dstSegment);

    memcpy(dstPtr, srcPtr, srcSeg->pageSize);

    FlsEmu_SelectPage(srcSegment, oldSrcPage);
    FlsEmu_SelectPage(dstSegment, oldDstPage);

    return true;
}

#endif /* XCP_ENABLE_CAL_COMMANDS || XCP_ENABLE_PAG_COMMANDS */
