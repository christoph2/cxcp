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

bool Serial_Available(void) {
    return tud_cdc_n_available(0) > 0;
}

void Serial_Write(const uint8_t* data, uint32_t length) {
    tud_cdc_n_write(0, data, length);
}

void Serial_WriteByte(uint8_t out_byte) {
    tud_cdc_n_write(0, &out_byte, 1);
}

void Serial_WriteBuffer(uint8_t const * out_bytes, uint32_t size) {
    tud_cdc_n_write(0, out_bytes, size);
}

bool Serial_Read(uint8_t* in_byte) {
    return tud_cdc_n_read(0, in_byte, 1) == 1;
}

void Serial_MainFunction(void) {
    tud_task();
    cdc_task();
}

static void cdc_task(void) {
    uint8_t        itf = 0;
    static uint8_t buf[64];

    // pico_set_led(false);

    for (itf = 0; itf < CFG_TUD_CDC; itf++) {
        // connected() check for DTR bit
        // Most but not all terminal client set this when making connection
        // if (tud_cdc_n_connected(itf)) {
        if (tud_cdc_n_available(itf)) {
            uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

            // echo back to both serial ports
            // echo_serial_port(0, buf, count);
            // echo_serial_port(1, buf, count);
        }
    }
    // pico_set_led(true);
}
