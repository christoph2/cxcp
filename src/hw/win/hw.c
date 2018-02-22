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


 /*
 ** Prescalers.
 */
 #define TIMER_PS_1US   (1000000UL)
 #define TIMER_PS_10US  (100000UL)
 #define TIMER_PS_100US (10000UL)

 #define TIMER_PS_1MS   (1000UL)
 #define TIMER_PS_10MS  (100UL)
 #define TIMER_PS_100MS (10UL)

void XcpHw_Init(void)
{
    LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
    LARGE_INTEGER Frequency;

    QueryPerformanceFrequency(&Frequency);
    printf("Freq: %lu\n", Frequency.QuadPart);

    QueryPerformanceCounter(&StartingTime);

// Activity to be timed
    Sleep(2000);

    QueryPerformanceCounter(&EndingTime);
    ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
    printf("ETA: %f\n", (float)ElapsedMicroseconds.QuadPart / (float)Frequency.QuadPart);
//////
//////
//////

//
// We now have the elapsed number of ticks, along with the
// number of ticks-per-second. We use these values
// to convert to the number of elapsed microseconds.
// To guard against loss-of-precision, we convert
// to microseconds *before* dividing by ticks-per-second.
//

    ElapsedMicroseconds.QuadPart *= 1000000;
    ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
}

uint64_t XcpHw_GetTimerCounter(void)
{

}
