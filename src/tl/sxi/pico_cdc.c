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

#include <stdint.h>

#include "xcp_config.h"

// #include "bsp/board_api.h"
#include "tusb.h"

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
/*!!! END-INCLUDE-SECTION !!!*/

#include <stdarg.h>
#include <stdio.h>

#define DEBUG_CDC_ITF 1  // Interface 1 für Debug

// Use TinyUSB's tu_printf alias hooked via CFG_TUSB_DEBUG_PRINTF in xcp.h

int cdc_debug_printf(const char* fmt, ...) {
    char    buf[128];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // if (tud_cdc_n_connected(DEBUG_CDC_ITF)) {
    tud_cdc_n_write(DEBUG_CDC_ITF, buf, len);
    tud_cdc_n_write_flush(DEBUG_CDC_ITF);
    //}
    return len;
}

void pico_set_led(bool led_on);

enum {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED     = 1000,
    BLINK_SUSPENDED   = 2500,
};

static void cdc_task(void);

void Serial_Init(void) {
    tusb_rhport_init_t dev_init = { .role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
}

void Serial_DeInit(void) {
}

uint32_t Serial_Available(void) {
    return tud_cdc_n_available(0);
}

void Serial_Write(const uint8_t* data, uint32_t length) {
    tud_cdc_n_write(0, data, length);
}

void Serial_WriteByte(uint8_t out_byte) {
    tud_cdc_n_write(0, &out_byte, 1);
}

void Serial_WriteBuffer(uint8_t const * out_bytes, uint32_t size) {
    tud_cdc_n_write(0, out_bytes, size);
    tud_cdc_n_write_flush(0);
}

bool Serial_Read(uint8_t* in_byte) {
    return tud_cdc_n_read(0, in_byte, 1) == 1;
}

void Serial_MainFunction(void) {
    tud_task();
    // cdc_task();
}

static void cdc_task(void) {
    uint8_t        itf = 0;
    static uint8_t buf[64];
    bool           recv = false;

    // pico_set_led(false);

    for (itf = 0; itf < CFG_TUD_CDC; itf++) {
        // connected() check for DTR bit
        // Most but not all terminal client set this when making connection
        // if (tud_cdc_n_connected(itf)) {
        if (tud_cdc_n_available(itf)) {
            // uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));
            recv = true;
            // echo back to both serial ports
            // echo_serial_port(0, buf, count);
            // echo_serial_port(1, buf, count);
        }
    }
    // pico_set_led(true);
}
