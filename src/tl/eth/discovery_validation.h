/*
 * BlueParrot XCP
 *
 * (C) 2026 by Christoph Schueler <github.com/Christoph2,
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

#ifndef CXCP_DISCOVERY_VALIDATION_H
#define CXCP_DISCOVERY_VALIDATION_H

#include "xcp.h"
#include "xcp_eth.h"

static inline bool XcpTl_ValidateDiscoveryPacket(const uint8_t *buf, size_t n, const uint8_t source_ipv4[4]) {
    uint16_t declared_len;
    uint16_t response_port;
    uint8_t  subcommand;
    size_t   minimum_len;

    if ((buf == NULL) || (n < (XCP_ETH_HEADER_SIZE + 2u)) || (n > XCP_COMM_BUFLEN)) {
        return false;
    }

    declared_len = XCP_MAKEWORD(buf[1], buf[0]);
    if (((size_t)declared_len != (n - XCP_ETH_HEADER_SIZE)) || (buf[4] != (uint8_t)0xF2)) {
        return false;
    }

    subcommand = buf[5];
    if ((subcommand != (uint8_t)0xFF) && (subcommand != (uint8_t)0xFD) && (subcommand != (uint8_t)0xFC)) {
        return true;
    }

    minimum_len = (subcommand == (uint8_t)0xFC) ? 31u : 12u;
    if ((n < minimum_len) || (source_ipv4 == NULL)) {
        return false;
    }

    response_port = (uint16_t)((uint16_t)buf[6] | ((uint16_t)buf[7] << 8u));
    if ((response_port == 0u) || (source_ipv4[0] == 0u) || (source_ipv4[0] >= 224u) || (buf[8] != source_ipv4[0]) ||
        (buf[9] != source_ipv4[1]) || (buf[10] != source_ipv4[2]) || (buf[11] != source_ipv4[3])) {
        return false;
    }

    return true;
}

#endif
