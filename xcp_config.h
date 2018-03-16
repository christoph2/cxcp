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

#if !defined(__XCP_CONFIG_H)
#define __XCP_CONFIG_H

#define XCP_STATION_ADDRESS         (1)
#define XCP_STATION_ID              "description_of_test_ecu.a2l"

#define XCP_BUILD_TYPE              XCP_DEBUG_BUILD

#define XCP_ENABLE_SLAVE_BLOCKMODE  XCP_OFF
#define XCP_ENABLE_MASTER_BLOCKMODE XCP_OFF

#define XCP_ENABLE_STIM             XCP_OFF

#define XCP_CHECKSUM_METHOD         XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT
#define XCP_BYTE_ORDER              XCP_BYTE_ORDER_INTEL
#define XCP_ADDRESS_GRANULARITY     XCP_ADDRESS_GRANULARITY_BYTE

#define XCP_MAX_BS                  (0)
#define XCP_MIN_ST                  (0)
#define XCP_QUEUE_SIZE              (0)

#define XCP_DAQ_TIMESTAMP_UNIT      (XCP_DAQ_TIMESTAMP_UNIT_1US)
#define XCP_DAQ_TIMESTAMP_SIZE      (XCP_DAQ_TIMESTAMP_SIZE_4)

/*
** Resource Protection.
*/

#define XCP_PROTECT_CAL             XCP_ON
#define XCP_PROTECT_PAG             XCP_ON
#define XCP_PROTECT_DAQ             XCP_ON
#define XCP_PROTECT_STIM            XCP_ON
#define XCP_PROTECT_PGM             XCP_ON


/*
**  Enable optional Commands.
*/
    #define XCP_ENABLE_GET_COMM_MODE_INFO           XCP_ON
    #define XCP_ENABLE_GET_ID                       XCP_ON
    #define XCP_ENABLE_SET_REQUEST                  XCP_OFF
    #define XCP_ENABLE_GET_SEED                     XCP_OFF
    #define XCP_ENABLE_UNLOCK                       XCP_OFF
    #define XCP_ENABLE_SET_MTA                      XCP_ON
    #define XCP_ENABLE_UPLOAD                       XCP_ON
    #define XCP_ENABLE_SHORT_UPLOAD                 XCP_ON
    #define XCP_ENABLE_BUILD_CHECKSUM               XCP_ON
    #define XCP_ENABLE_TRANSPORT_LAYER_CMD          XCP_OFF /* TODO: TL dependend include file! */
    #define XCP_ENABLE_USER_CMD                     XCP_OFF

#define XCP_ENABLE_CAL_COMMANDS                     XCP_OFF

    #define XCP_ENABLE_DOWNLOAD_NEXT                XCP_OFF
    #define XCP_ENABLE_DOWNLOAD_MAX                 XCP_OFF
    #define XCP_ENABLE_SHORT_DOWNLOAD               XCP_OFF
    #define XCP_ENABLE_MODIFY_BITS                  XCP_OFF

#define XCP_ENABLE_PAG_COMMANDS                     XCP_OFF

    #define XCP_ENABLE_GET_PAG_PROCESSOR_INFO       XCP_OFF
    #define XCP_ENABLE_GET_SEGMENT_INFO             XCP_OFF
    #define XCP_ENABLE_GET_PAGE_INFO                XCP_OFF
    #define XCP_ENABLE_SET_SEGMENT_MODE             XCP_OFF
    #define XCP_ENABLE_GET_SEGMENT_MODE             XCP_OFF
    #define XCP_ENABLE_COPY_CAL_PAGE                XCP_OFF

#define XCP_ENABLE_DAQ_COMMANDS                     XCP_ON

    #define XCP_ENABLE_GET_DAQ_CLOCK                XCP_ON
    #define XCP_ENABLE_READ_DAQ                     XCP_OFF
    #define XCP_ENABLE_GET_DAQ_PROCESSOR_INFO       XCP_OFF
    #define XCP_ENABLE_GET_DAQ_RESOLUTION_INFO      XCP_ON
    #define XCP_ENABLE_GET_DAQ_LIST_INFO            XCP_OFF
    #define XCP_ENABLE_GET_DAQ_EVENT_INFO           XCP_OFF
    #define XCP_ENABLE_FREE_DAQ                     XCP_OFF
    #define XCP_ENABLE_ALLOC_DAQ                    XCP_OFF
    #define XCP_ENABLE_ALLOC_ODT                    XCP_OFF
    #define XCP_ENABLE_ALLOC_ODT_ENTRY              XCP_OFF

#define XCP_ENABLE_PGM_COMMANDS                     XCP_OFF

    #define XCP_ENABLE_GET_PGM_PROCESSOR_INFO       XCP_OFF
    #define XCP_ENABLE_GET_SECTOR_INFO              XCP_OFF
    #define XCP_ENABLE_PROGRAM_PREPARE              XCP_OFF
    #define XCP_ENABLE_PROGRAM_FORMAT               XCP_OFF
    #define XCP_ENABLE_PROGRAM_NEXT                 XCP_OFF
    #define XCP_ENABLE_PROGRAM_MAX                  XCP_OFF
    #define XCP_ENABLE_PROGRAM_VERIFY               XCP_OFF

/*
**  Transport-Layser specific Options (may not apply to every Transport).
*/
#define XCP_TRANSPORT_LAYER_LENGTH_SIZE             (1)     /* [0 | 1 | 2] */
#define XCP_TRANSPORT_LAYER_COUNTER_SIZE            (1)     /* [0 | 1 | 2] */
#define XCP_TRANSPORT_LAYER_CHECKSUM_SIZE           (0)     /* [0 | 1 | 2] */

#endif /* __XCP_CONFIG_H */

