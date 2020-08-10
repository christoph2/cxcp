/*
 * BlueParrot XCP
 *
 * (C) 2007-2019 by Christoph Schueler <github.com/Christoph2,
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

#if !defined(__XCP_CONFIG_H)
#define __XCP_CONFIG_H


#define XCP_STATION_ADDRESS                         (1)
#define XCP_STATION_ID                              "description_of_test_ecu.a2l"

#define XCP_BUILD_TYPE                              XCP_DEBUG_BUILD

#define XCP_EXTERN_C_GUARDS                         XCP_OFF

#define XCP_ENABLE_SLAVE_BLOCKMODE                  XCP_OFF
#define XCP_ENABLE_MASTER_BLOCKMODE                 XCP_OFF

#define XCP_ENABLE_STIM                             XCP_OFF

#define XCP_CHECKSUM_METHOD                         XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT
#define XCP_CHECKSUM_CHUNKED_CALCULATION            XCP_ON
#define XCP_CHECKSUM_CHUNK_SIZE                     (64)
#define XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE             (0)     /* 0 ==> unlimited */

#define XCP_BYTE_ORDER                              XCP_BYTE_ORDER_INTEL
#define XCP_ADDRESS_GRANULARITY                     XCP_ADDRESS_GRANULARITY_BYTE

#define XCP_MAX_CTO                                 (8)
#define XCP_MAX_DTO                                 (8)


#endif /* __XCP_CONFIG_H */
