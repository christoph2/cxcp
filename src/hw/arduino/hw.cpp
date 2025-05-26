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

#include <Arduino.h>
#include <stdint.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"

/*!!! END-INCLUDE-SECTION !!!*/

typedef struct tagHwStateType {
    uint32_t StartingTime;
} HwStateType;

static HwStateType HwState = { 0 };

void XcpHw_Init(void) {
    HwState.StartingTime = millis();
    pinMode(LED_BUILTIN, OUTPUT);
}

void XcpHw_Deinit(void) {
}

uint32_t XcpHw_GetTimerCounter(void) {
#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
    return micros();
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
    return millis();
#else
    #error Timestamp-unit not supported.
#endif  // XCP_DAQ_TIMESTAMP_UNIT
}

uint32_t XcpHw_GetTimerCounterMS(void) {
    return millis();
}

bool XcpHw_SxIAvailable(void) {
    return Serial.available();
}

uint8_t XcpHw_SxIRead(void) {
    return (uint8_t)Serial.read();
}

void XcpHw_AcquireLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    noInterrupts();
}

void XcpHw_ReleaseLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    interrupts();
}

void XcpHw_Sleep(uint64_t usec) {
    delayMicroseconds(usec);
}
