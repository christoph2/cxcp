/*
 * pySART - Simplified AUTOSAR-Toolkit for Python.
 *
 * (C) 2007-2018 by Christoph Schueler <github.com/Christoph2,
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

 #include <windows.h>
 #include <stdint.h>

 #include "xcp.h"

typedef struct tagHwStateType {
    LARGE_INTEGER StartingTime;
    LARGE_INTEGER TicksPerSecond;
} HwStateType;


 /*
 ** Prescalers.
 */
#define TIMER_PS_1US   (1000000UL)
#define TIMER_PS_10US  (100000UL)
#define TIMER_PS_100US (10000UL)

#define TIMER_PS_1MS   (1000UL)
#define TIMER_PS_10MS  (100UL)
#define TIMER_PS_100MS (10UL)

static HwStateType HwState = {0};

void XcpHw_Init(void)
{
    LARGE_INTEGER EndingTime, ElapsedMicroseconds;

    QueryPerformanceFrequency(&HwState.TicksPerSecond);
    printf("Freq: %lu\n", HwState.TicksPerSecond);

    QueryPerformanceCounter(&HwState.StartingTime);

// Activity to be timed
    Sleep(2000);

    QueryPerformanceCounter(&EndingTime);
    ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - HwState.StartingTime.QuadPart;
    printf("ETA: %f\n", (float)ElapsedMicroseconds.QuadPart / (float)HwState.TicksPerSecond.QuadPart);
    printf("Eta/new: %u\n", XcpHw_GetTimerCounter());
//////
//////
//////

    ElapsedMicroseconds.QuadPart *= 1000000;
    ElapsedMicroseconds.QuadPart /= HwState.TicksPerSecond.QuadPart;
}

uint32_t XcpHw_GetTimerCounter(void)
{
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
#error Timestamp unit not supported
#endif // XCP_DAQ_TIMESTAMP_UNIT

    Elapsed.QuadPart /= HwState.TicksPerSecond.QuadPart;

    return (uint32_t)Elapsed.QuadPart;
}
