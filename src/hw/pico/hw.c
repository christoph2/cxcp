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

#include "bsp/board_api.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "pico/critical_section.h"
#include "hardware/gpio.h"
#include <stdint.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"

/*!!! END-INCLUDE-SECTION !!!*/

typedef struct tagHwStateType {
    uint32_t StartingTime;
} HwStateType;

critical_section_t XcpHw_Locks[XCP_HW_LOCK_COUNT];

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
        critical_section_init(&XcpHw_Locks[idx]);
    }
}

static void XcpHw_DeinitLocks(void) {
    uint8_t idx = 0;

    for (idx = 0; idx < XCP_HW_LOCK_COUNT; ++idx) {
        critical_section_deinit(&XcpHw_Locks[idx]);
    }
}

// Initialize the GPIO for the LED
void pico_led_init(void) {
    #ifdef PICO_DEFAULT_LED_PIN
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    #endif
}

// Turn the LED on or off
void pico_set_led(bool led_on) {
    #if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
    #endif
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
#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
    return time_us_32();
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
    return XcpHw_GetMs32();
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
     critical_section_enter_blocking(&XcpHw_Locks[lockIdx]);
}

void XcpHw_ReleaseLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    critical_section_exit(&XcpHw_Locks[lockIdx]);
}

void XcpHw_Sleep(uint64_t usec) {
    sleep_us(usec);
}