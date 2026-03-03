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

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp_timecorr.h"

#include "xcp.h"
#include "xcp_hw.h"
/*!!! END-INCLUDE-SECTION !!!*/

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)

/*
** ============================================================
**  Local types
** ============================================================
*/

typedef struct tagXcpTimecorr_StateType {
    XcpTimecorr_ResponseFmtType       response_fmt;
    uint16_t                          cluster_affiliation;
    XcpTimecorr_TimeSourceType const *grandmaster_source; /* NULL if no grandmaster registered */
} XcpTimecorr_StateType;

    /*
    ** ============================================================
    **  Clock info data block (uploaded by master via UPLOAD cmd)
    ** ============================================================
    **
    ** Layout (all fields Intel byte order):
    **   [0 .. 23]  XCP slave clock info   (always present when SLV_CLK_INFO bit set)
    **   [24 .. 47] Grandmaster clock info (present when GRANDM_CLK_INFO bit set)
    **   [48 .. 63] Clock relation         (present when CLK_RELATION bit set, grandmaster required)
    **   [64 .. 87] ECU clock info         (present when ECU_CLK_INFO bit set)
    **   [88 .. 95] ECU grandmaster UUID   (present when ECU_GRANDM_CLK_INFO bit set)
    */

    /* Individual block sizes (per XCP spec) */
    #define CLOCK_INFO_BLOCK_SIZE      (24u) /* UUID(8) + ticks(2) + unit(1) + stratum(1) + native(1) + epoch(1) + rsv(2) + maxts(8) */
    #define CLOCK_RELATION_BLOCK_SIZE  (16u) /* origin DLONG(8) + slv_ts DLONG(8) */
    #define ECU_GRANDM_UUID_BLOCK_SIZE (8u)  /* UUID(8) only */

    #define MAX_CLOCK_INFO_BLOCK                                                                                                   \
        (CLOCK_INFO_BLOCK_SIZE + CLOCK_INFO_BLOCK_SIZE + CLOCK_RELATION_BLOCK_SIZE + CLOCK_INFO_BLOCK_SIZE +                       \
         ECU_GRANDM_UUID_BLOCK_SIZE) /* 96 bytes */

static uint8_t XcpTimecorr_ClockInfoBlock[MAX_CLOCK_INFO_BLOCK];
static uint8_t XcpTimecorr_ClockInfoBlockLen = UINT8(0);

/*
** ============================================================
**  Module state
** ============================================================
*/

static XcpTimecorr_StateType XcpTimecorr_State = {
    .response_fmt        = XCP_TIMECORR_FMT_LEGACY,
    .cluster_affiliation = XCP_TIMECORR_DEFAULT_CLUSTER_ID,
    .grandmaster_source  = XCP_NULL,
};

/* Built-in XCP slave clock info (uses XcpHw_GetTimerCounter) */
static const XcpTimecorr_ClockInfoType XcpTimecorr_SlaveClockInfo = {
    .uuid          = XCP_TIMECORR_SLV_UUID,
    .ticks         = UINT16(1),
    .unit          = XCP_DAQ_TIMESTAMP_UNIT,
    .stratum       = XCP_TIMECORR_SLV_STRATUM,
    .native_size   = XCP_TIMECORR_SLV_NATIVE_TS_SIZE,
    .epoch         = XCP_TIMECORR_EPOCH_ARBITRARY,
    .max_timestamp = (XCP_TIMECORR_SLV_NATIVE_TS_SIZE == 4u) ? UINT64(0xFFFFFFFFu) : UINT64(0xFFFFFFFFFFFFFFFFu),
};

/*
** ============================================================
**  Forward declarations
** ============================================================
*/

static void XcpTimecorr_BuildClockInfoBlock(bool include_grandm, bool include_relation);
static void XcpTimecorr_SendTimeSyncInternal(
    uint8_t trigger_initiator, bool include_cluster_id, uint16_t cluster_id, uint8_t counter
);
static void    XcpTimecorr_WriteClockInfoStruct(uint8_t *dst, XcpTimecorr_ClockInfoType const *info);
static uint8_t XcpTimecorr_BuildObservableClocks(void);
static uint8_t XcpTimecorr_BuildSyncState(void);

/*
** ============================================================
**  Helpers: little-endian write utilities
** ============================================================
*/

static void WriteU16LE(uint8_t *dst, uint16_t v) {
    dst[0] = (uint8_t)(v & 0xFFu);
    dst[1] = (uint8_t)((v >> 8u) & 0xFFu);
}

static void WriteU32LE(uint8_t *dst, uint32_t v) {
    dst[0] = (uint8_t)(v & 0xFFu);
    dst[1] = (uint8_t)((v >> 8u) & 0xFFu);
    dst[2] = (uint8_t)((v >> 16u) & 0xFFu);
    dst[3] = (uint8_t)((v >> 24u) & 0xFFu);
}

static void WriteU64LE(uint8_t *dst, uint64_t v) {
    dst[0] = (uint8_t)(v & 0xFFu);
    dst[1] = (uint8_t)((v >> 8u) & 0xFFu);
    dst[2] = (uint8_t)((v >> 16u) & 0xFFu);
    dst[3] = (uint8_t)((v >> 24u) & 0xFFu);
    dst[4] = (uint8_t)((v >> 32u) & 0xFFu);
    dst[5] = (uint8_t)((v >> 40u) & 0xFFu);
    dst[6] = (uint8_t)((v >> 48u) & 0xFFu);
    dst[7] = (uint8_t)((v >> 56u) & 0xFFu);
}

/*
** ============================================================
**  Public API implementation
** ============================================================
*/

void XcpTimecorr_Init(void) {
    XcpTimecorr_State.response_fmt        = XCP_TIMECORR_FMT_LEGACY;
    XcpTimecorr_State.cluster_affiliation = XCP_TIMECORR_DEFAULT_CLUSTER_ID;
    XcpTimecorr_State.grandmaster_source  = XCP_NULL;
    XcpTimecorr_ClockInfoBlockLen         = UINT8(0);
}

void XcpTimecorr_SetGrandmasterClockSource(XcpTimecorr_TimeSourceType const *source) {
    XCP_ENTER_CRITICAL();
    XcpTimecorr_State.grandmaster_source = source;
    XCP_LEAVE_CRITICAL();
}

XcpTimecorr_ResponseFmtType XcpTimecorr_GetResponseFmt(void) {
    return XcpTimecorr_State.response_fmt;
}

uint16_t XcpTimecorr_GetClusterAffiliation(void) {
    return XcpTimecorr_State.cluster_affiliation;
}

/*
** ============================================================
**  TIME_CORRELATION_PROPERTIES command handler (0xC6)
** ============================================================
*/

void XcpTimecorr_HandleTimeCorrelationProperties(Xcp_PduType const * const pdu) {
    /*
     * Command layout (Intel byte order):
     *   [0] 0xC6  command code
     *   [1]       SET_PROPERTIES
     *   [2]       GET_PROPERTIES_REQUEST
     *   [3]       RESERVED
     *   [4..5]    CLUSTER_ID (WORD)
     */
    uint8_t  set_props  = Xcp_GetByte(pdu, UINT8(1));
    uint8_t  get_req    = Xcp_GetByte(pdu, UINT8(2));
    uint16_t cluster_id = Xcp_GetWord(pdu, UINT8(4));

    uint8_t response_fmt_req = (uint8_t)(set_props & UINT8(0x03));         /* bits [1:0] */
    uint8_t time_sync_bridge = (uint8_t)((set_props >> 2u) & UINT8(0x03)); /* bits [3:2] */
    uint8_t set_cluster_id   = (uint8_t)((set_props >> 4u) & UINT8(0x01)); /* bit  [4]  */
    bool    get_clk_info     = ((get_req & UINT8(0x01)) != UINT8(0));      /* bit  [0]  */

    DBG_TRACE(
        "TIME_CORRELATION_PROPERTIES [set_props: 0x%02X get_req: 0x%02X cluster_id: 0x%04X]\n\r", set_props, get_req, cluster_id
    );

    /* --- Process SET_PROPERTIES --- */

    /* RESPONSE_FMT: value 0 = do not change */
    if (response_fmt_req != UINT8(0)) {
        if (response_fmt_req <= UINT8(2)) {
            XcpTimecorr_State.response_fmt = (XcpTimecorr_ResponseFmtType)response_fmt_req;
        }
        /* value 3 is reserved; silently ignore */
    }

    /* TIME_SYNC_BRIDGE: silently ignored – we do not offer a bridge feature.
     * The spec requires silent ignore when the slave does not support this. */
    (void)time_sync_bridge;

    /* SET_CLUSTER_ID */
    if (set_cluster_id != UINT8(0)) {
        if (cluster_id != XcpTimecorr_State.cluster_affiliation) {
            XcpTimecorr_State.cluster_affiliation = cluster_id;
            /* Notify transport layer to rejoin the correct multicast group */
            XcpTl_UpdateMulticastGroup(cluster_id);
        }
    }

    /* --- Process GET_PROPERTIES_REQUEST --- */

    uint8_t clock_info_byte  = UINT8(0);
    bool    include_grandm   = false;
    bool    include_relation = false;

    if (get_clk_info) {
        /* Decide which clock info blocks to include */
        clock_info_byte |= UINT8(0x01); /* SLV_CLK_INFO always present */

    #if XCP_TIMECORR_GRANDM_CLK > 0
        clock_info_byte |= UINT8(0x02); /* GRANDM_CLK_INFO */
        include_grandm = true;
        if (XcpTimecorr_State.grandmaster_source != XCP_NULL) {
            clock_info_byte |= UINT8(0x04); /* CLK_RELATION */
            include_relation = true;
        }
    #endif

    #if XCP_TIMECORR_ECU_CLK > 0
        clock_info_byte |= UINT8(0x08); /* ECU_CLK_INFO */
    #endif

        XcpTimecorr_BuildClockInfoBlock(include_grandm, include_relation);

        /* Set MTA to start of clock info data block */
        Xcp_MtaType mta;
        mta.ext     = UINT8(0);
        mta.address = (Xcp_PointerSizeType)(uintptr_t)XcpTimecorr_ClockInfoBlock;
        Xcp_SetMta(mta);
    }

    /* --- Build positive response --- */

    /*
     * Response layout:
     *   [0] 0xFF  positive response
     *   [1]       SLAVE_CONFIG
     *   [2]       OBSERVABLE_CLOCKS
     *   [3]       SYNC_STATE
     *   [4]       CLOCK_INFO
     *   [5]       RESERVED
     *   [6..7]    CLUSTER_ID (WORD, Intel byte order)
     */

    /* SLAVE_CONFIG byte:
     *   bits [1:0] RESPONSE_FMT
     *   bit  [2]   DAQ_TS_RELATION (0 = XCP slave clock, 1 = ECU clock)
     *   bits [4:3] TIME_SYNC_BRIDGE (0 = not offered)
     */
    uint8_t slave_config = (uint8_t)XcpTimecorr_State.response_fmt; /* RESPONSE_FMT */
    #if XCP_TIMECORR_ECU_CLK > 0
    slave_config |= UINT8(0x04); /* DAQ_TS_RELATION = 1 (ECU clock) */
    #endif
    /* TIME_SYNC_BRIDGE = 0 (not offered) */

    uint8_t observable_clocks = XcpTimecorr_BuildObservableClocks();
    uint8_t sync_state        = XcpTimecorr_BuildSyncState();

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), slave_config, observable_clocks, sync_state, clock_info_byte,
        UINT8(0), /* RESERVED */
        XCP_LOBYTE(XcpTimecorr_State.cluster_affiliation), XCP_HIBYTE(XcpTimecorr_State.cluster_affiliation)
    );
}

/*
** ============================================================
**  GET_DAQ_CLOCK_MULTICAST handler
** ============================================================
*/

void XcpTimecorr_HandleMulticast(uint16_t cluster_id, uint8_t counter) {
    /* Only respond if cluster_id matches affiliation (or default affiliation == 0 accepts all) */
    if ((XcpTimecorr_State.cluster_affiliation != UINT16(0)) && (cluster_id != XcpTimecorr_State.cluster_affiliation)) {
        return;
    }

    /* Only send EV_TIME_SYNC if XCP is connected (master must receive the unicast response) */
    if (Xcp_GetConnectionState() != XCP_CONNECTED) {
        return;
    }

    /* In legacy mode, do not respond to GET_DAQ_CLOCK_MULTICAST */
    if (XcpTimecorr_State.response_fmt == XCP_TIMECORR_FMT_LEGACY) {
        return;
    }

    XcpTimecorr_SendTimeSyncInternal(
        XCP_TIMECORR_TRIG_MULTICAST, true, /* include cluster id + counter */
        cluster_id, counter
    );
}

void XcpTimecorr_SendTimeSyncEvent(uint8_t trigger_initiator) {
    if (Xcp_GetConnectionState() != XCP_CONNECTED) {
        return;
    }

    /* Internal triggers (TRIGGER_INITIATOR != 2 or 3) are only sent when RESPONSE_FMT == 2 */
    if ((trigger_initiator != XCP_TIMECORR_TRIG_MULTICAST) && (trigger_initiator != XCP_TIMECORR_TRIG_BRIDGE)) {
        if (XcpTimecorr_State.response_fmt != XCP_TIMECORR_FMT_EXT_2) {
            return;
        }
    }

    if (XcpTimecorr_State.response_fmt == XCP_TIMECORR_FMT_LEGACY) {
        return;
    }

    XcpTimecorr_SendTimeSyncInternal(trigger_initiator, false, UINT16(0), UINT8(0));
}

/*
** ============================================================
**  Internal helpers
** ============================================================
*/

static uint8_t XcpTimecorr_BuildObservableClocks(void) {
    uint8_t obs = UINT8(0);

    /* XCP_SLV_CLK: bits [1:0] */
    obs |= (uint8_t)(((uint8_t)XCP_TIMECORR_XCP_SLV_CLK) & UINT8(0x03));

    /* GRANDM_CLK: bits [3:2] – reflect actual capability */
    #if XCP_TIMECORR_GRANDM_CLK > 0
    {
        uint8_t grandm_val = UINT8(XCP_TIMECORR_GRANDM_CLK);
        obs |= (uint8_t)((grandm_val & UINT8(0x03)) << 2u);
    }
    #endif

    /* ECU_CLK: bits [5:4] */
    #if XCP_TIMECORR_ECU_CLK > 0
    {
        uint8_t ecu_val = UINT8(XCP_TIMECORR_ECU_CLK);
        obs |= (uint8_t)((ecu_val & UINT8(0x03)) << 4u);
    }
    #endif

    return obs;
}

static uint8_t XcpTimecorr_BuildSyncState(void) {
    uint8_t sync = UINT8(0);

    /*
     * SLV_CLK_SYNC_STATE (bits [2:0]):
     * Only relevant if XCP_SLV_CLK == 1.
     * For XCP_SLV_CLK == 0 (free running), report "no sync support" (7).
     */
    #if XCP_TIMECORR_XCP_SLV_CLK == 1
    /* XCP slave clock can be syntonized/synchronized; report actual state */
    sync |= XCP_TIMECORR_SLV_NO_SYNC_SUPPORT; /* TODO: override when sync is implemented */
    #else
    sync |= XCP_TIMECORR_SLV_NO_SYNC_SUPPORT;
    #endif

    /* GRANDM_CLK_SYNC_STATE (bit [3]): only relevant if GRANDM_CLK > 0 */
    #if XCP_TIMECORR_GRANDM_CLK > 0
    {
        uint8_t grandm_state = XCP_TIMECORR_GRANDM_NOT_SYNCED;
        if (XcpTimecorr_State.grandmaster_source != XCP_NULL && XcpTimecorr_State.grandmaster_source->get_sync_state != XCP_NULL) {
            grandm_state = XcpTimecorr_State.grandmaster_source->get_sync_state();
        }
        sync |= (uint8_t)((grandm_state & UINT8(0x01)) << 3u);
    }
    #endif

    /* ECU_CLK_SYNC_STATE (bits [5:4]): only relevant if ECU_CLK > 0 */
    #if XCP_TIMECORR_ECU_CLK > 0
    sync |= (uint8_t)(XCP_TIMECORR_ECU_SYNC_UNKNOWN << 4u);
    #endif

    return sync;
}

/*
 * Serialize a single 24-byte clock info block into dst.
 */
static void XcpTimecorr_WriteClockInfoStruct(uint8_t *dst, XcpTimecorr_ClockInfoType const *info) {
    XcpUtl_MemCopy(dst, info->uuid, UINT32(8)); /* UUID (8 bytes) */
    WriteU16LE(dst + 8u, info->ticks);          /* Timestamp Ticks */
    dst[10u] = info->unit;                      /* Timestamp Unit */
    dst[11u] = info->stratum;                   /* Stratum Level */
    dst[12u] = info->native_size;               /* Native timestamp size */
    dst[13u] = info->epoch;                     /* Epoch */
    dst[14u] = UINT8(0);                        /* Reserved (WORD) */
    dst[15u] = UINT8(0);
    WriteU64LE(dst + 16u, info->max_timestamp); /* MAX_TIMESTAMP_VALUE_BEFORE_WRAP_AROUND */
}

/*
 * Build the clock info data block that the master uploads via UPLOAD command.
 * The MTA is set to XcpTimecorr_ClockInfoBlock after this call.
 */
static void XcpTimecorr_BuildClockInfoBlock(bool include_grandm, bool include_relation) {
    uint8_t *p   = XcpTimecorr_ClockInfoBlock;
    uint8_t  len = UINT8(0);

    /* XCP slave clock info (always included) */
    XcpTimecorr_WriteClockInfoStruct(p, &XcpTimecorr_SlaveClockInfo);
    p += CLOCK_INFO_BLOCK_SIZE;
    len += CLOCK_INFO_BLOCK_SIZE;

    /* Grandmaster clock info */
    if (include_grandm) {
        if (XcpTimecorr_State.grandmaster_source != XCP_NULL) {
            XcpTimecorr_WriteClockInfoStruct(p, &XcpTimecorr_State.grandmaster_source->info);
        } else {
            /* No grandmaster connected: UUID = 0 */
            XcpUtl_ZeroMem(p, CLOCK_INFO_BLOCK_SIZE);
        }
        p += CLOCK_INFO_BLOCK_SIZE;
        len += CLOCK_INFO_BLOCK_SIZE;

        /* Clock relation (origin + slave timestamp pair, sampled simultaneously) */
        if (include_relation && XcpTimecorr_State.grandmaster_source != XCP_NULL) {
            uint64_t grandm_ts = UINT64(0);
            uint64_t slv_ts    = UINT64(0);

            /* Sample both clocks as simultaneously as possible */
            XCP_ENTER_CRITICAL();
            if (XcpTimecorr_State.grandmaster_source->info.native_size == UINT8(4)) {
                grandm_ts = (uint64_t)XcpTimecorr_State.grandmaster_source->get_timestamp_u32();
            } else if (XcpTimecorr_State.grandmaster_source->get_timestamp_u64 != XCP_NULL) {
                grandm_ts = XcpTimecorr_State.grandmaster_source->get_timestamp_u64();
            }
            slv_ts = (uint64_t)XcpHw_GetTimerCounter();
            XCP_LEAVE_CRITICAL();

            WriteU64LE(p, grandm_ts);   /* Origin (grandmaster timestamp) */
            WriteU64LE(p + 8u, slv_ts); /* XCP slave timestamp */
        } else {
            XcpUtl_ZeroMem(p, CLOCK_RELATION_BLOCK_SIZE);
        }
        p += CLOCK_RELATION_BLOCK_SIZE;
        len += CLOCK_RELATION_BLOCK_SIZE;
    }

    XcpTimecorr_ClockInfoBlockLen = len;
}

/*
 * Build and send the extended EV_TIME_SYNC event packet.
 *
 * Payload layout (after the 2-byte event header FD 08):
 *   [0]         TRIGGER_INFO
 *   [1]         PAYLOAD_FMT
 *   [2..5/9]    XCP slave clock timestamp (DWORD or DLONG per FMT_XCP_SLV)
 *   [+4/8]      Grandmaster timestamp     (DWORD or DLONG per FMT_GRANDM, if present)
 *   [+2]        Cluster Identifier (WORD)  (if CLUSTER_IDENTIFIER bit set)
 *   [+1]        Counter                    (if CLUSTER_IDENTIFIER bit set)
 *   [+1]        SYNC_STATE                 (if any clock can be sync'd)
 */
static void XcpTimecorr_SendTimeSyncInternal(
    uint8_t trigger_initiator, bool include_cluster_id, uint16_t cluster_id, uint8_t counter
) {
    uint8_t payload[32]; /* sufficient for all scenarios */
    uint8_t pos             = UINT8(0);
    uint8_t fmt_xcp_slv     = UINT8(0);
    uint8_t fmt_grandm      = UINT8(0);
    bool    send_sync_state = false;

    /* Determine timestamp format for XCP slave clock */
    #if XCP_TIMECORR_XCP_SLV_CLK != 2
    fmt_xcp_slv = (XcpTimecorr_SlaveClockInfo.native_size == UINT8(4)) ? UINT8(1) : UINT8(2);
    #endif

    /* Determine timestamp format for grandmaster clock */
    #if XCP_TIMECORR_GRANDM_CLK > 0
    if (XcpTimecorr_State.grandmaster_source != XCP_NULL) {
        fmt_grandm = (XcpTimecorr_State.grandmaster_source->info.native_size == UINT8(4)) ? UINT8(1) : UINT8(2);
    }
    #endif

    /* SYNC_STATE is sent if any observable clock supports synchronization */
    #if (XCP_TIMECORR_XCP_SLV_CLK == 1) || (XCP_TIMECORR_GRANDM_CLK > 0)
    send_sync_state = true;
    #endif

    /* TRIGGER_INFO:  bits[2:0] = trigger_initiator, bits[4:3] = TIME_OF_TS_SAMPLING
     * For multicast, use RX sampling (3) per spec recommendation. */
    uint8_t ts_sampling  = (trigger_initiator == XCP_TIMECORR_TRIG_MULTICAST || trigger_initiator == XCP_TIMECORR_TRIG_BRIDGE) ?
                               XCP_TIMECORR_TS_SAMP_RX :
                               XCP_TIMECORR_TS_SAMP_CMD;
    uint8_t trigger_info = (uint8_t)((trigger_initiator & UINT8(0x07)) | (uint8_t)((ts_sampling & UINT8(0x03)) << 3u));

    /* PAYLOAD_FMT:
     *   bits[1:0] FMT_XCP_SLV
     *   bits[3:2] FMT_GRANDM
     *   bits[5:4] FMT_ECU (always 0 here, ECU support via separate extension)
     *   bit[6]    CLUSTER_IDENTIFIER
     */
    uint8_t payload_fmt = (uint8_t)(fmt_xcp_slv | (uint8_t)(fmt_grandm << 2u) | (include_cluster_id ? UINT8(0x40) : UINT8(0)));

    payload[pos++] = trigger_info;
    payload[pos++] = payload_fmt;

    /* Sample all timestamps as close together as possible */
    uint32_t slv_ts_32    = UINT32(0);
    uint64_t slv_ts_64    = UINT64(0);
    uint32_t grandm_ts_32 = UINT32(0);
    uint64_t grandm_ts_64 = UINT64(0);

    XCP_ENTER_CRITICAL();

    if (fmt_xcp_slv == UINT8(1)) {
        slv_ts_32 = XcpHw_GetTimerCounter();
    } else if (fmt_xcp_slv == UINT8(2)) {
        slv_ts_64 = (uint64_t)XcpHw_GetTimerCounter();
    }

    #if XCP_TIMECORR_GRANDM_CLK > 0
    if (fmt_grandm != UINT8(0) && XcpTimecorr_State.grandmaster_source != XCP_NULL) {
        if (fmt_grandm == UINT8(1)) {
            grandm_ts_32 = XcpTimecorr_State.grandmaster_source->get_timestamp_u32();
        } else {
            grandm_ts_64 = (XcpTimecorr_State.grandmaster_source->get_timestamp_u64 != XCP_NULL) ?
                               XcpTimecorr_State.grandmaster_source->get_timestamp_u64() :
                               (uint64_t)XcpTimecorr_State.grandmaster_source->get_timestamp_u32();
        }
    }
    #endif

    XCP_LEAVE_CRITICAL();

    /* Append XCP slave timestamp */
    if (fmt_xcp_slv == UINT8(1)) {
        WriteU32LE(payload + pos, slv_ts_32);
        pos += UINT8(4);
    } else if (fmt_xcp_slv == UINT8(2)) {
        WriteU64LE(payload + pos, slv_ts_64);
        pos += UINT8(8);
    }

    /* Append grandmaster timestamp */
    if (fmt_grandm == UINT8(1)) {
        WriteU32LE(payload + pos, grandm_ts_32);
        pos += UINT8(4);
    } else if (fmt_grandm == UINT8(2)) {
        WriteU64LE(payload + pos, grandm_ts_64);
        pos += UINT8(8);
    }

    /* Cluster Identifier + Counter (only for TRIGGER_INITIATOR 2 or 3) */
    if (include_cluster_id) {
        payload[pos++] = XCP_LOBYTE(cluster_id);
        payload[pos++] = XCP_HIBYTE(cluster_id);
        payload[pos++] = counter;
    }

    /* SYNC_STATE (only if at least one clock supports synchronization) */
    if (send_sync_state) {
        payload[pos++] = XcpTimecorr_BuildSyncState();
    }

    Xcp_SendEventPacket(EV_TIME_SYNC, payload, pos);
}

#endif /* XCP_ENABLE_TIME_CORRELATION */
