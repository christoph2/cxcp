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
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_10US=
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100US
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_100US)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_1MS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10MS
    #define XCP_HW_TIMER_PRESCALER (TIMER_PS_10MS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100MS
    #define XCP_HW_TIMER_PRESCALRE (TIMER_PS_100MS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1S
    #define XCP_HW_TIMER_PRESCALRE (TIMER_PS_1S)
#else
    #error Timestamp-unit not supported.
#endif  // XCP_DAQ_TIMESTAMP_UNIT

static dispatch_queue_t timer_queue;

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
    timer_queue = dispatch_queue_create("XCPTimerQueue", DISPATCH_QUEUE_SERIAL);

    // Create dispatch timer sources
    timer1 = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, timer_queue);
    timer2 = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, timer_queue);

    // Set event handlers for the timers
    dispatch_source_set_event_handler(timer1, ^{
        vector1(timer1);
    });
    dispatch_source_set_event_handler(timer2, ^{
        vector2(timer2);
    });

    // Set cancel handlers for the timers
    dispatch_source_set_cancel_handler(timer1, ^{
        dispatch_release(timer1);
        dispatch_release(timer_queue);
        printf("end\n");
        exit(0);
    });
    dispatch_source_set_cancel_handler(timer2, ^{
        dispatch_release(timer2);
        dispatch_release(timer_queue);
        printf("end\n");
        // exit(0);
    });

    dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC);  // Start after 1 second

    // Set timer intervals (0.2 sec and 0.5 sec)
    dispatch_source_set_timer(timer1, start, NSEC_PER_SEC / 5, 0);  // 0.2 sec
    dispatch_source_set_timer(timer2, start, NSEC_PER_SEC / 2, 0);  // 0.5 sec

    printf("start\n");
    dispatch_resume(timer1);
    dispatch_resume(timer2);

    XcpHw_PosixInit();
}

static uint64_t XcpHw_GetElapsedTime(uint32_t prescaler) {
}

uint32_t XcpHw_GetTimerCounter(void) {
}

uint32_t XcpHw_GetTimerCounterMS(void) {
}
