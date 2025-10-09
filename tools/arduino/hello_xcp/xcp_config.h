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

/*
 *  For details on options refer to `documentation
 * <https://github.com/christoph2/cxcp/docs/options.rst>`_
 */

#if !defined(__XCP_CONFIG_H)
    #define __XCP_CONFIG_H

/*
**  General Options.
*/
    //    #define TP_CAN
    #define TP_SXI

    #define XCP_CAN_INTERFACE (XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD)

    #define XCP_GET_ID_0 "BlueParrot XCP running on Arduino"
    #define XCP_GET_ID_1 "BlueParrot_XCP_on_Arduino"

    // #define XCP_BUILD_TYPE                              XCP_RELEASE_BUILD
    #define XCP_BUILD_TYPE XCP_DEBUG_BUILD

    #define XCP_ENABLE_EXTERN_C_GUARDS (XCP_ON)

    #define XCP_ENABLE_SLAVE_BLOCKMODE  (XCP_ON)
    #define XCP_ENABLE_MASTER_BLOCKMODE (XCP_OFF)

    #define XCP_ENABLE_STIM (XCP_OFF)

    #define XCP_CHECKSUM_METHOD              XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT
    #define XCP_CHECKSUM_CHUNKED_CALCULATION (XCP_ON)
    #define XCP_CHECKSUM_CHUNK_SIZE          (64)
    #define XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE  (0) /* 0 ==> unlimited */

    #define XCP_BYTE_ORDER          XCP_BYTE_ORDER_INTEL
    #define XCP_ADDRESS_GRANULARITY XCP_ADDRESS_GRANULARITY_BYTE

    #define XCP_ENABLE_STATISTICS (XCP_ON)

    #define XCP_MAX_BS     (255)
    #define XCP_MIN_ST     (0)
    #define XCP_QUEUE_SIZE (0)

    #define XCP_MAIN_FUNCTION_PERIOD (2000) /* Period in microseconds. */

    /*
    ** Resource Protection.
    */
    #define XCP_PROTECT_CAL  (XCP_OFF)
    #define XCP_PROTECT_PAG  (XCP_ON)
    #define XCP_PROTECT_DAQ  (XCP_OFF)
    #define XCP_PROTECT_STIM (XCP_ON)
    #define XCP_PROTECT_PGM  (XCP_OFF)

    /*
    **  DAQ Settings.
    */
    #define XCP_DAQ_CONFIG_TYPE                         XCP_DAQ_CONFIG_TYPE_DYNAMIC
    #define XCP_DAQ_DTO_BUFFER_SIZE                     (40)
    #define XCP_DAQ_ENABLE_PREDEFINED_LISTS             (XCP_OFF)
    #define XCP_DAQ_ENABLE_TIMESTAMPING                 (XCP_ON)
    #define XCP_DAQ_TIMESTAMP_UNIT                      (XCP_DAQ_TIMESTAMP_UNIT_1US)
    #define XCP_DAQ_TIMESTAMP_SIZE                      (XCP_DAQ_TIMESTAMP_SIZE_4)
    #define XCP_DAQ_ENABLE_PRESCALER                    (XCP_OFF)
    #define XCP_DAQ_ENABLE_ADDR_EXT                     (XCP_OFF)
    #define XCP_DAQ_ENABLE_BIT_OFFSET                   (XCP_OFF)
    #define XCP_DAQ_ENABLE_PRIORITIZATION               (XCP_OFF)
    #define XCP_DAQ_ENABLE_ALTERNATING                  (XCP_OFF)
    #define XCP_DAQ_ENABLE_CLOCK_ACCESS_ALWAYS          (XCP_ON)
    #define XCP_DAQ_ENABLE_WRITE_THROUGH                (XCP_OFF)
    #define XCP_DAQ_QUEUE_SIZE                          (16)
    #define XCP_DAQ_MAX_DYNAMIC_ENTITIES                (16)
    #define XCP_DAQ_MAX_EVENT_CHANNEL                   (3)
    #define XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT (XCP_OFF)

    /*
    **  PGM Settings.
    */
    #define XCP_MAX_BS_PGM (0)
    #define XCP_MIN_ST_PGM (0)

    #define XCP_MAX_SECTOR_PGM UINT8(32)
    #define XCP_PGM_PROPERIES  XCP_PGM_ABSOLUTE_MODE

    /*
    **  Optional Services.
    */
    #define XCP_ENABLE_GET_COMM_MODE_INFO (XCP_ON)
    #define XCP_ENABLE_GET_ID             (XCP_ON)
    #define XCP_ENABLE_SET_REQUEST        (XCP_OFF)
    #define XCP_ENABLE_GET_SEED           (XCP_OFF)
    #define XCP_ENABLE_UNLOCK             (XCP_OFF)
    #define XCP_ENABLE_SET_MTA            (XCP_ON)
    #define XCP_ENABLE_UPLOAD             (XCP_ON)
    #define XCP_ENABLE_SHORT_UPLOAD       (XCP_ON)
    #define XCP_ENABLE_BUILD_CHECKSUM     (XCP_ON)
    #define XCP_ENABLE_USER_CMD           (XCP_OFF)

    #define XCP_ENABLE_CAL_COMMANDS (XCP_ON)

    #define XCP_ENABLE_DOWNLOAD_NEXT  (XCP_OFF)
    #define XCP_ENABLE_DOWNLOAD_MAX   (XCP_ON)
    #define XCP_ENABLE_SHORT_DOWNLOAD (XCP_ON)
    #define XCP_ENABLE_MODIFY_BITS    (XCP_ON)

    #define XCP_ENABLE_PAG_COMMANDS (XCP_OFF)

    #define XCP_ENABLE_GET_PAG_PROCESSOR_INFO (XCP_OFF)
    #define XCP_ENABLE_GET_SEGMENT_INFO       (XCP_OFF)
    #define XCP_ENABLE_GET_PAGE_INFO          (XCP_OFF)
    #define XCP_ENABLE_SET_SEGMENT_MODE       (XCP_OFF)
    #define XCP_ENABLE_GET_SEGMENT_MODE       (XCP_OFF)
    #define XCP_ENABLE_COPY_CAL_PAGE          (XCP_OFF)

    #define XCP_ENABLE_DAQ_COMMANDS (XCP_ON)

    #define XCP_ENABLE_GET_DAQ_CLOCK           (XCP_ON)
    #define XCP_ENABLE_READ_DAQ                (XCP_OFF)
    #define XCP_ENABLE_GET_DAQ_PROCESSOR_INFO  (XCP_ON)
    #define XCP_ENABLE_GET_DAQ_RESOLUTION_INFO (XCP_ON)
    #define XCP_ENABLE_GET_DAQ_LIST_INFO       (XCP_ON)
    #define XCP_ENABLE_GET_DAQ_EVENT_INFO      (XCP_ON)
    #define XCP_ENABLE_FREE_DAQ                (XCP_ON)
    #define XCP_ENABLE_ALLOC_DAQ               (XCP_ON)
    #define XCP_ENABLE_ALLOC_ODT               (XCP_ON)
    #define XCP_ENABLE_ALLOC_ODT_ENTRY         (XCP_ON)
    #define XCP_ENABLE_WRITE_DAQ_MULTIPLE      (XCP_OFF)

    #define XCP_ENABLE_PGM_COMMANDS (XCP_ON)

    #define XCP_ENABLE_GET_PGM_PROCESSOR_INFO (XCP_ON)
    #define XCP_ENABLE_GET_SECTOR_INFO        (XCP_ON)
    #define XCP_ENABLE_PROGRAM_PREPARE        (XCP_ON)
    #define XCP_ENABLE_PROGRAM_FORMAT         (XCP_OFF)
    #define XCP_ENABLE_PROGRAM_NEXT           (XCP_OFF)
    #define XCP_ENABLE_PROGRAM_MAX            (XCP_OFF)
    #define XCP_ENABLE_PROGRAM_VERIFY         (XCP_OFF)

    #define XCP_ENABLE_EVENT_PACKET_API    (XCP_ON)
    #define XCP_ENABLE_SERVICE_REQUEST_API (XCP_OFF)

    /*
    **  Transport-Layer specific Options (may not apply to every Transport).
    */
    #if defined(TP_CAN)
        #define XCP_TRANSPORT_LAYER XCP_ON_CAN

        #define XCP_ON_CAN_INBOUND_IDENTIFIER   (0x300)
        #define XCP_ON_CAN_OUTBOUND_IDENTIFIER  (0x301)
        #define XCP_ON_CAN_MAX_DLC_REQUIRED     (XCP_OFF)
        #define XCP_ON_CAN_BROADCAST_IDENTIFIER (0x222)

        // Chip-Select- and Interrupt-Pins / SeeedStudio CAN Shield v2.
        #define XCP_CAN_IF_MCP25XX_PIN_CS  (9)
        #define XCP_CAN_IF_MCP25XX_PIN_INT (2)

        #define XCP_ON_CAN_FREQ   (CAN_500KBPS)
        #define XCP_ON_CAN_BTQ    (16)
        #define XCP_ON_CAN_TSEG1  (14)
        #define XCP_ON_CAN_TSEG2  (2)
        #define XCP_ON_CAN_SJW    (2)
        #define XCP_ON_CAN_NOSAMP (1)

        /* Transport-Layer Commands.    */
        #define XCP_ENABLE_CAN_GET_SLAVE_ID (XCP_ON)
        #define XCP_ENABLE_CAN_GET_DAQ_ID   (XCP_ON)
        #define XCP_ENABLE_CAN_SET_DAQ_ID   (XCP_OFF)

    #elif defined(TP_BLUETOOTH)
        #define XCP_TRANSPORT_LAYER XCP_ON_BTH

        #define XCP_MAX_CTO (64)  // (16)
        #define XCP_MAX_DTO (64)

    #elif defined(TP_ETHER)
        #define XCP_TRANSPORT_LAYER XCP_ON_ETHERNET

        #define XCP_MAX_CTO (64)  // (16)
        #define XCP_MAX_DTO (64)

    #elif defined(TP_SXI) || (XCP_TRANSPORT_LAYER == XCP_ON_SXI)
        #define XCP_TRANSPORT_LAYER       XCP_ON_SXI
        #define XCP_ON_SXI_HEADER_FORMAT  (XCP_ON_SXI_HEADER_LEN_CTR_WORD)
        #define XCP_ON_SXI_BITRATE        (38400)
        #define XCP_ON_SXI_CONFIG         (SERIAL_8N1)
        #define XCP_MAX_CTO               (12)
        #define XCP_MAX_DTO               (12)
        #define XCP_ON_SXI_TAIL_CHECKSUM  (XCP_ON_SXI_CHECKSUM_BYTE)
        /* Framing and escaping (as used by xcp_tl.c) */
        #define XCP_ON_SXI_ENABLE_FRAMING (XCP_OFF)
        #define XCP_ON_SXI_SYNC_CHAR      (0xAA)
        #define XCP_ON_SXI_ESC_CHAR       (0xAB)
        #define XCP_ON_SXI_ESC_SYNC_CHAR  (0x01)
        #define XCP_ON_SXI_ESC_ESC_CHAR   (0x00)
    #else
        #error "No transport-layer. please define either TP_ETHER, TP_CAN, or TP_BLUETOOTH."
    #endif  // KVASER_CAN

    /*
    **  Customization Options.
    */
    #define XCP_ENABLE_ADDRESS_MAPPER      (XCP_OFF)
    #define XCP_ENABLE_CHECK_MEMORY_ACCESS (XCP_OFF)
    #define XCP_REPLACE_STD_COPY_MEMORY    (XCP_OFF)
    #define XCP_ENABLE_GET_ID_HOOK         (XCP_OFF)

    /*
    **
    ** Arduino Specific Options.
    **
    */

    /*
    **  Platform Specific Function-like Macros.
    */
    #define XCP_ENTER_CRITICAL()     XcpHw_AcquireLock(XCP_HW_LOCK_XCP)
    #define XCP_LEAVE_CRITICAL()     XcpHw_ReleaseLock(XCP_HW_LOCK_XCP)
    #define XCP_HW_ENTER_CRITICAL()  XcpHw_AcquireLock(XCP_HW_LOCK_HW)
    #define XCP_HW_LEAVE_CRITICAL()  XcpHw_ReleaseLock(XCP_HW_LOCK_HW)
    #define XCP_TL_ENTER_CRITICAL()  XcpHw_AcquireLock(XCP_HW_LOCK_TL)
    #define XCP_TL_LEAVE_CRITICAL()  XcpHw_ReleaseLock(XCP_HW_LOCK_TL)
    #define XCP_DAQ_ENTER_CRITICAL() XcpHw_AcquireLock(XCP_HW_LOCK_DAQ)
    #define XCP_DAQ_LEAVE_CRITICAL() XcpHw_ReleaseLock(XCP_HW_LOCK_DAQ)
    #define XCP_STIM_ENTER_CRITICAL()
    #define XCP_STIM_LEAVE_CRITICAL()
    #define XCP_PGM_ENTER_CRITICAL()
    #define XCP_PGM_LEAVE_CRITICAL()
    #define XCP_CAL_ENTER_CRITICAL()
    #define XCP_CAL_LEAVE_CRITICAL()
    #define XCP_PAG_ENTER_CRITICAL()
    #define XCP_PAG_LEAVE_CRITICAL()

    /*
    **  Application Settings.
    */
    #define XCP_APP_TIMEBASE (10)
/* Applications gets called every 'n' milliseconds,
** 0 ==> free running.
*/

#endif /* __XCP_CONFIG_H */
