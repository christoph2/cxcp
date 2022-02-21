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
} HwStateType;

/*
** Local Defines.
*/
#define TIMER_PS_1US (1000000UL)
#define TIMER_PS_10US (100000UL)
#define TIMER_PS_100US (10000UL)

#define TIMER_PS_1MS (1000UL)
#define TIMER_PS_10MS (100UL)
#define TIMER_PS_100MS (10UL)

#define TIMER_MASK_1 (0x000000FFUL)
#define TIMER_MASK_2 (0x0000FFFFUL)
#define TIMER_MASK_4 (0xFFFFFFFFUL)

/*
**  Local Function Prototypes.
*/
static void XcpHw_InitLocks(void);
static void XcpHw_DeinitLocks();

/*
**  External Function Prototypes.
*/
void XcpTl_PrintConnectionInformation(void);

void FlsEmu_Info(void);
void XcpDaq_Info(void);
void XcpDaq_PrintDAQDetails();
bool XcpDaq_QueueEmpty(void);
bool XcpDaq_QueueDequeue(uint16_t* len, uint8_t* data);

void XcpHw_MainFunction(void);

void exitFunc(void);

/*
**  Local Variables.
*/
static HwStateType HwState = {0};
CRITICAL_SECTION XcpHw_Locks[XCP_HW_LOCK_COUNT];

/*
**  Global Functions.
*/
void XcpHw_Init(void) {
    fflush(stdout);
    //_setmode(_fileno(stdout), _O_WTEXT);    /* Permit Unicode output on console */
    //_setmode(_fileno(stdout), _O_U8TEXT);    /* Permit Unicode output on console */
    QueryPerformanceFrequency(&HwState.TicksPerSecond);
    XcpHw_InitLocks();
}

void XcpHw_Deinit(void) { XcpHw_DeinitLocks(); }

uint32_t XcpHw_GetTimerCounter(void) {
    LARGE_INTEGER Now;
    LARGE_INTEGER Elapsed;

    QueryPerformanceCounter(&Now);
    Elapsed.QuadPart = Now.QuadPart - HwState.StartingTime.QuadPart;

#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
    Elapsed.QuadPart *= TIMER_PS_1US;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10US
    Elapsed.QuadPart *= TIMER_PS_10US;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100US
    Elapsed.QuadPart *= TIMER_PS_100US;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
    Elapsed.QuadPart *= TIMER_PS_1MS;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10MS
    Elapsed.QuadPart *= TIMER_PS_10MS;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100MS
    Elapsed.QuadPart *= TIMER_PS_100MS;
#else
#error Timestamp-unit not supported.
#endif  // XCP_DAQ_TIMESTAMP_UNIT

    Elapsed.QuadPart /= HwState.TicksPerSecond.QuadPart;

#if XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_1
    return (uint32_t)Elapsed.QuadPart & TIMER_MASK_1;
#elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_2
    return (uint32_t)Elapsed.QuadPart) & TIMER_MASK_2;
#elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_4
    return (uint32_t)Elapsed.QuadPart & TIMER_MASK_4;
#else
#error Timestamp-size not supported.
#endif  // XCP_DAQ_TIMESTAMP_SIZE
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

void XcpHw_ErrorMsg(char* const fun, int errorCode) {
    char buffer[1024];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 1024, NULL);
    fprintf(stderr, "[%s] failed with: [%d] %s", fun, errorCode, buffer);
}

void XcpHw_TransmitDtos(void) {
    uint16_t len;
    uint8_t* dataOut = Xcp_GetDtoOutPtr();
    while (!XcpDaq_QueueEmpty()) {
        XcpDaq_QueueDequeue(&len, dataOut);
        // printf("\tDTO -- len: %d data: \t", len);
        Xcp_SetDtoOutLen(len);
        Xcp_SendDto();
    }
}

/*
 *
 * Sleep for `usec` microseconds.
 *
 */
void XcpHw_Sleep(uint64_t usec) {
    HANDLE timer;
    LARGE_INTEGER ft;

    ZeroMemory(&ft, sizeof(LARGE_INTEGER));
    ft.QuadPart = -(10 * usec);
    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
