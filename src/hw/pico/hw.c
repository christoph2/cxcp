/*
 * pySART - Simplified AUTOSAR-Toolkit for Python.
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

#include <stdint.h>

#include "bsp/board_api.h"
#include "hardware/gpio.h"
#include "pico/critical_section.h"
#include "pico/stdlib.h"
#include "tusb.h"

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"

/*!!! END-INCLUDE-SECTION !!!*/

typedef struct tagHwStateType {
    uint32_t StartingTime;
} HwStateType;

recursive_mutex_t XcpHw_Locks[XCP_HW_LOCK_COUNT];

static HwStateType HwState = { 0 };

/*
**  Local Function Prototypes.
*/
static void XcpHw_InitLocks(void);
static void XcpHw_DeinitLocks();

static inline uint32_t XcpHw_GetMs32(void) {
    return (time_us_64() / 1000) & 0xffffffffu;
}

static void XcpHw_InitLocks(void) {
    uint8_t idx = 0;

    for (idx = 0; idx < XCP_HW_LOCK_COUNT; ++idx) {
        recursive_mutex_init(&XcpHw_Locks[idx]);
    }
}

static void XcpHw_DeinitLocks(void) {
    uint8_t idx = 0;

    for (idx = 0; idx < XCP_HW_LOCK_COUNT; ++idx) {
    }
}

void XcpHw_Init(void) {
    board_init();
    pico_led_init();
    XcpHw_InitLocks();
    HwState.StartingTime = XcpHw_GetMs32();
}

void XcpHw_Deinit(void) {
    XcpHw_DeinitLocks();
}

uint32_t XcpHw_GetTimerCounter(void) {
    uint64_t us = time_us_64();
#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1NS
    return (uint32_t)(us * 1000ULL);
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10NS
    return (uint32_t)(us * 100ULL);
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100NS
    return (uint32_t)(us * 10ULL);
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
    return (uint32_t)(us);
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10US
    return (uint32_t)(us / 10ULL);
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100US
    return (uint32_t)(us / 100ULL);
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
    return (uint32_t)(us / 1000ULL);
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10MS
    return (uint32_t)(us / 10000ULL);
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100MS
    return (uint32_t)(us / 100000ULL);
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1S
    return (uint32_t)(us / 1000000ULL);
#else
    #error Timestamp-unit not supported.
#endif  // XCP_DAQ_TIMESTAMP_UNIT
}

uint32_t XcpHw_GetTimerCounterMS(void) {
    return XcpHw_GetMs32();
}

void XcpHw_AcquireLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    recursive_mutex_enter_blocking(&XcpHw_Locks[lockIdx]);
}

void XcpHw_ReleaseLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    recursive_mutex_exit(&XcpHw_Locks[lockIdx]);
}

void XcpHw_Sleep(uint64_t usec) {
    sleep_us(usec);
}