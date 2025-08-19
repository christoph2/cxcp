/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
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

#define _GNU_C_SOURCE

#include <ctype.h>
#include <dispatch/dispatch.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_hw.h"

/*!!! END-INCLUDE-SECTION !!!*/

/*
**  Local Types.
*/

typedef struct tagHwStateType {
    struct timespec StartingTime;
    bool            Initialized;
} HwStateType;


/*
** Local Defines.
*/
#define TIMER_PS_1NS   (1UL)
#define TIMER_PS_10NS  (10UL)
#define TIMER_PS_100NS (100UL)
#define TIMER_PS_1US   (1000UL)
#define TIMER_PS_10US  (10000UL)
#define TIMER_PS_100US (100000UL)
#define TIMER_PS_1MS   (1000000UL)
#define TIMER_PS_10MS  (10000000UL)
#define TIMER_PS_100MS (100000000UL)
#define TIMER_PS_1S    (1000000000UL)

/* Set timer prescaler */
#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1NS
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_1NS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10NS
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_10NS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100NS
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_100NS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
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


static dispatch_queue_t timer_queue;
static HwStateType HwState;
int i = 0;

dispatch_source_t timer1;
dispatch_source_t timer2;

void sigtrap(int sig) {
    dispatch_source_cancel(timer1);
    dispatch_source_cancel(timer2);
    printf("CTRL-C received, exiting program.\n");
    exit(EXIT_SUCCESS);
}

void vector1(dispatch_source_t timer) {
    printf("a: %d\n", i);
    i++;
    if (i >= 20) {
        dispatch_source_cancel(timer);
    }
}

void vector2(dispatch_source_t timer) {
    printf("b: %d\n", i);
    i++;
    if (i >= 20) {
        dispatch_source_cancel(timer);
    }
}

void XcpHw_Init(void) {

    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::clock_gettime()", errno);
        HwState.Initialized = false;
    } else {
        HwState.StartingTime = ts;
        HwState.Initialized  = true;
    }
    /* Optional: falls Dispatch-Timer benötigt werden, hier initialisieren – aber nicht exit() auf Cancel. */
    timer_queue = dispatch_queue_create("XCPTimerQueue", DISPATCH_QUEUE_SERIAL);
    XcpHw_PosixInit();
}


static uint64_t XcpHw_GetElapsedTime(uint32_t prescaler) {
    struct timespec now = {0};
    struct timespec dt  = {0};
    uint64_t        timestamp = 0ULL;

    if (!HwState.Initialized || prescaler == 0U) {
        return 0ULL;
    }
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &now) == -1) {
        XcpHw_ErrorMsg("XcpHw_GetElapsedTime::clock_gettime()", errno);
        return 0ULL;
    }

    if ((now.tv_nsec - HwState.StartingTime.tv_nsec) < 0) {
        dt.tv_sec  = now.tv_sec - HwState.StartingTime.tv_sec - 1;
        dt.tv_nsec = 1000000000L + now.tv_nsec - HwState.StartingTime.tv_nsec;
    } else {
        dt.tv_sec  = now.tv_sec - HwState.StartingTime.tv_sec;
        dt.tv_nsec = now.tv_nsec - HwState.StartingTime.tv_nsec;
    }

    uint64_t free_running =
        ((uint64_t)dt.tv_sec * (uint64_t)1000 * 1000 * 1000) + (uint64_t)dt.tv_nsec;
    timestamp = free_running / (uint64_t)prescaler;
    return timestamp;
}


uint32_t XcpHw_GetTimerCounter(void) {
    uint64_t timestamp = XcpHw_GetElapsedTime(XCP_HW_TIMER_PRESCALER);

    #if XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_1
    timestamp &= 0x000000FFULL;
    #elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_2
    timestamp &= 0x0000FFFFULL;
    #elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_4
    timestamp &= 0xFFFFFFFFULL;
    #else
    #error Timestamp-size not supported.
    #endif  // XCP_DAQ_TIMESTAMP_SIZE

    return (uint32_t)timestamp;
}


uint32_t XcpHw_GetTimerCounterMS(void) {
    uint64_t timestamp = XcpHw_GetElapsedTime(TIMER_PS_1MS);
    return (uint32_t)(timestamp & 0xFFFFFFFFULL);
}

