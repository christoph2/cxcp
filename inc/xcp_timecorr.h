/*
 * BlueParrot XCP
 *
 * (C) 2025 by Christoph Schueler <github.com/Christoph2,
 *                                  cpu12.gems@googlemail.com>
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
 * XCP Advanced Time Correlation (spec chapters 7.5.6.1, 5.4, 7.7.8)
 *
 * Implements TIME_CORRELATION_PROPERTIES (0xC6) and GET_DAQ_CLOCK_MULTICAST
 * (transport layer cmd 0xF2/0xFA), plus the extended EV_TIME_SYNC event.
 *
 * External time sources (e.g. GPS/PPS modules) are plugged in via
 * XcpTimecorr_SetGrandmasterClockSource().
 */

#if !defined(__XCP_TIMECORR_H)
    #define __XCP_TIMECORR_H

    #include "xcp.h"

    #if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)

    /*
    ** ============================================================
    **  Configuration defaults (override in xcp_config.h)
    ** ============================================================
    */

    /* Observable XCP slave clock type:
     *   0 = free-running, can be read randomly
     *   1 = may be syntonized/synchronized to grandmaster, readable
     *   2 = no local clock (DAQ timestamps from external source)
     */
        #if !defined(XCP_TIMECORR_XCP_SLV_CLK)
            #define XCP_TIMECORR_XCP_SLV_CLK (0)
        #endif

    /* Observable grandmaster clock type:
     *   0 = no dedicated grandmaster clock
     *   1 = dedicated grandmaster clock, readable randomly
     *   2 = dedicated grandmaster clock, NOT readable randomly; slave generates EV_TIME_SYNC autonomously
     */
        #if !defined(XCP_TIMECORR_GRANDM_CLK)
            #define XCP_TIMECORR_GRANDM_CLK (0)
        #endif

    /* Observable ECU clock type:
     *   0 = no ECU clock
     *   1 = ECU clock readable randomly
     *   2 = ECU clock not readable randomly; slave generates EV_TIME_SYNC autonomously
     *   3 = ECU timestamps reported, ECU clock not readable
     */
        #if !defined(XCP_TIMECORR_ECU_CLK)
            #define XCP_TIMECORR_ECU_CLK (0)
        #endif

    /* Default cluster affiliation (determines multicast group).
     * Maps to IPv4 239.255.UPPER_BYTE_LE.LOWER_BYTE_LE
     * 0x0001 → 239.255.0.1
     */
        #if !defined(XCP_TIMECORR_DEFAULT_CLUSTER_ID)
            #define XCP_TIMECORR_DEFAULT_CLUSTER_ID (0x0001u)
        #endif

    /* Native timestamp size for the XCP slave clock (4 or 8 bytes). */
        #if !defined(XCP_TIMECORR_SLV_NATIVE_TS_SIZE)
            #define XCP_TIMECORR_SLV_NATIVE_TS_SIZE (4u)
        #endif

    /* XCP slave clock UUID (EUI-64, 8 bytes, most significant byte first).
     * Should be unique per device. Override in xcp_config.h. */
        #if !defined(XCP_TIMECORR_SLV_UUID)
            #define XCP_TIMECORR_SLV_UUID { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x01u }
        #endif

    /* XCP slave clock stratum level (255 = unknown). */
        #if !defined(XCP_TIMECORR_SLV_STRATUM)
            #define XCP_TIMECORR_SLV_STRATUM (255u)
        #endif

    /* Multicast UDP port (fixed by XCP spec). */
        #define XCP_TIMECORR_MULTICAST_PORT (5557u)

    /*
    ** ============================================================
    **  Constants
    ** ============================================================
    */

    /* Stratum level for unknown clock quality */
        #define XCP_TIMECORR_STRATUM_UNKNOWN (255u)

    /* Native timestamp size values */
        #define XCP_TIMECORR_NATIVE_TS_DWORD (4u)
        #define XCP_TIMECORR_NATIVE_TS_DLONG (8u)

    /* Epoch values (for grandmaster clock info) */
        #define XCP_TIMECORR_EPOCH_TAI       (0u)
        #define XCP_TIMECORR_EPOCH_UTC       (1u)
        #define XCP_TIMECORR_EPOCH_ARBITRARY (2u)

    /* TRIGGER_INITIATOR field values (bits [2:0] of TRIGGER_INFO) */
        #define XCP_TIMECORR_TRIG_HW          (0u) /* HW trigger / Vector Syncline */
        #define XCP_TIMECORR_TRIG_INDEPENDENT (1u) /* XCP-independent sync event (e.g. PPS) */
        #define XCP_TIMECORR_TRIG_MULTICAST   (2u) /* GET_DAQ_CLOCK_MULTICAST */
        #define XCP_TIMECORR_TRIG_BRIDGE      (3u) /* GET_DAQ_CLOCK_MULTICAST via Time Sync Bridge */
        #define XCP_TIMECORR_TRIG_SYNC_STATE  (4u) /* Syntonization/synchronization state change */
        #define XCP_TIMECORR_TRIG_LEAP_SECOND (5u) /* Leap second on grandmaster */
        #define XCP_TIMECORR_TRIG_ECU_RESET   (6u) /* Release of ECU reset */

    /* TIME_OF_TS_SAMPLING field values (bits [4:3] of TRIGGER_INFO) */
        #define XCP_TIMECORR_TS_SAMP_CMD (0u) /* During command processing */
        #define XCP_TIMECORR_TS_SAMP_ISR (1u) /* Low-jitter, high-priority interrupt */
        #define XCP_TIMECORR_TS_SAMP_TX  (2u) /* Upon physical transmission */
        #define XCP_TIMECORR_TS_SAMP_RX  (3u) /* Upon physical reception (recommended for multicast) */

    /* SLV_CLK_SYNC_STATE values (bits [2:0] of SYNC_STATE field) */
        #define XCP_TIMECORR_SLV_SYNC_IN_PROGRESS   (0u) /* Synchronization in progress */
        #define XCP_TIMECORR_SLV_SYNCED             (1u) /* Synchronized to grandmaster */
        #define XCP_TIMECORR_SLV_SYNTON_IN_PROGRESS (2u) /* Syntonization in progress */
        #define XCP_TIMECORR_SLV_SYNTONIZED         (3u) /* Syntonized to grandmaster */
        #define XCP_TIMECORR_SLV_NO_SYNC_SUPPORT    (7u) /* No sync/synton capability */

    /* GRANDM_CLK_SYNC_STATE values (bit [3] of SYNC_STATE field) */
        #define XCP_TIMECORR_GRANDM_NOT_SYNCED (0u)
        #define XCP_TIMECORR_GRANDM_SYNCED     (1u)

    /* ECU_CLK_SYNC_STATE values (bits [5:4] of SYNC_STATE field) */
        #define XCP_TIMECORR_ECU_NOT_SYNCED   (0u)
        #define XCP_TIMECORR_ECU_SYNCED       (1u)
        #define XCP_TIMECORR_ECU_SYNC_UNKNOWN (2u)

/*
** ============================================================
**  Types
** ============================================================
*/

/*
 * Clock identification and characteristics.
 * Used both in the upload data block and internally.
 */
typedef struct tagXcpTimecorr_ClockInfoType {
    uint8_t  uuid[8];       /* EUI-64 unique clock identifier (most significant byte first) */
    uint16_t ticks;         /* Timestamp ticks per unit */
    uint8_t  unit;          /* Timestamp unit (XCP_DAQ_TIMESTAMP_UNIT_*) */
    uint8_t  stratum;       /* Clock quality by stratum level; 255 = unknown */
    uint8_t  native_size;   /* Size of native counter: 4 (DWORD) or 8 (DLONG) */
    uint8_t  epoch;         /* Epoch: 0=TAI, 1=UTC, 2=arbitrary (only meaningful for grandmaster) */
    uint64_t max_timestamp; /* Last valid value before wrap-around */
} XcpTimecorr_ClockInfoType;

/*
 * External time source interface.
 *
 * Implement this struct to connect any external time source (GPS/PPS, IEEE 1588 PTP, etc.)
 * as a grandmaster clock observable by the XCP slave.
 *
 * Usage example (GPS module with 1PPS):
 *
 *   static uint32_t gps_get_ts(void) { return gps_read_counter(); }
 *   static uint8_t  gps_sync_state(void) { return gps_is_locked()
 *                                             ? XCP_TIMECORR_GRANDM_SYNCED
 *                                             : XCP_TIMECORR_GRANDM_NOT_SYNCED; }
 *   static const XcpTimecorr_TimeSourceType GPS_Source = {
 *       .info = {
 *           .uuid         = { 0xAA, 0xBB, 0xCC, 0xFF, 0xFE, 0x01, 0x02, 0x03 },
 *           .ticks        = 1,
 *           .unit         = XCP_DAQ_TIMESTAMP_UNIT_1US,
 *           .stratum      = 1,
 *           .native_size  = XCP_TIMECORR_NATIVE_TS_DWORD,
 *           .epoch        = XCP_TIMECORR_EPOCH_UTC,
 *           .max_timestamp = 0xFFFFFFFFULL,
 *       },
 *       .get_timestamp_u32 = gps_get_ts,
 *       .get_timestamp_u64 = NULL,
 *       .get_sync_state    = gps_sync_state,
 *   };
 *   XcpTimecorr_SetGrandmasterClockSource(&GPS_Source);
 */
typedef struct tagXcpTimecorr_TimeSourceType {
    /* Static clock characteristics (filled once at startup) */
    XcpTimecorr_ClockInfoType info;

    /* Read current 32-bit timestamp. Used when info.native_size == 4.
     * Must not be NULL when info.native_size == 4. */
    uint32_t (*get_timestamp_u32)(void);

    /* Read current 64-bit timestamp. Used when info.native_size == 8.
     * May be NULL when info.native_size == 4. */
    uint64_t (*get_timestamp_u64)(void);

    /* Return current synchronization state (XCP_TIMECORR_GRANDM_*).
     * May be NULL; treated as XCP_TIMECORR_GRANDM_NOT_SYNCED when NULL. */
    uint8_t (*get_sync_state)(void);
} XcpTimecorr_TimeSourceType;

/* Current response format (RESPONSE_FMT in SLAVE_CONFIG) */
typedef enum tagXcpTimecorr_ResponseFmtType {
    XCP_TIMECORR_FMT_LEGACY = 0, /* Legacy mode (after CONNECT, before master activates) */
    XCP_TIMECORR_FMT_EXT_1  = 1, /* Extended: EV_TIME_SYNC for TRIGGER_INITIATOR 0, 2, 3 only */
    XCP_TIMECORR_FMT_EXT_2  = 2, /* Extended: EV_TIME_SYNC for all trigger conditions */
} XcpTimecorr_ResponseFmtType;

/*
** ============================================================
**  Public API
** ============================================================
*/

/* Initialize time correlation module.
 * Called automatically from Xcp_Init() when XCP_ENABLE_TIME_CORRELATION is on. */
void XcpTimecorr_Init(void);

/* Register an external grandmaster clock source (e.g. GPS/PPS module).
 * Set source to NULL to remove grandmaster clock.
 * This maps to GRANDM_CLK in OBSERVABLE_CLOCKS (scenario 4a / 4b). */
void XcpTimecorr_SetGrandmasterClockSource(XcpTimecorr_TimeSourceType const *source);

/* Handle TIME_CORRELATION_PROPERTIES command (0xC6). Called from xcp.c. */
void XcpTimecorr_HandleTimeCorrelationProperties(Xcp_PduType const * const pdu);

/* Handle GET_DAQ_CLOCK_MULTICAST reception.
 * Builds and sends extended EV_TIME_SYNC event packet if XCP is connected
 * and the cluster_id matches the cluster affiliation (or cluster_affiliation == 0).
 * cluster_id : cluster identifier from multicast packet (Intel byte order)
 * counter    : counter byte copied from GET_DAQ_CLOCK_MULTICAST */
void XcpTimecorr_HandleMulticast(uint16_t cluster_id, uint8_t counter);

/* Send EV_TIME_SYNC for a non-multicast trigger (e.g. PPS signal, state change).
 * Only sent when RESPONSE_FMT == XCP_TIMECORR_FMT_EXT_2.
 * trigger_initiator: one of XCP_TIMECORR_TRIG_* */
void XcpTimecorr_SendTimeSyncEvent(uint8_t trigger_initiator);

/* Query current response format */
XcpTimecorr_ResponseFmtType XcpTimecorr_GetResponseFmt(void);

/* Query current cluster affiliation */
uint16_t XcpTimecorr_GetClusterAffiliation(void);

/* Notify the transport layer that the cluster affiliation has changed
 * so it can rejoin the correct multicast group.
 * Implemented in the transport layer (wineth.c / linuxeth.c). */
void XcpTl_UpdateMulticastGroup(uint16_t cluster_id);

    #endif /* XCP_ENABLE_TIME_CORRELATION */

#endif /* __XCP_TIMECORR_H */
