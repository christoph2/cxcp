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

#include "Arduino.h"
#include "xcp_config.h"

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_tl_timeout.h"
#include "xcp_util.h"
/*!!! END-INCLUDE-SECTION !!!*/

extern "C" {

    void Serial_Init(void) {
        Serial.begin(XCP_ON_SXI_BITRATE, XCP_ON_SXI_CONFIG);
    }

    void Serial_DeInit(void) {
    }

    bool Serial_Available(void) {
        return Serial.available() > 0;
    }

    bool Serial_Read(uint8_t *in_byte) {
        uint8_t octet;

        if (Serial.available() > 0) {
            octet    = Serial.read();
            *in_byte = octet;
            return true;
        }
        return false;
    }

    void Serial_WriteByte(uint8_t out_byte) {
        Serial.write(out_byte);
    }

    void Serial_WriteBuffer(uint8_t const * out_bytes, uint32_t size) {
        Serial.write(out_bytes, size);
    }
}
