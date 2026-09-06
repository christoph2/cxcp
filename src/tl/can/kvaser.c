/*
 * BlueParrot XCP
 *
 * (C) 2007-2026 by Christoph Schueler <github.com/Christoph2,
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

/*!!! START-INCLUDE-SECTION !!!*/
#include "Xcp.h"
#include "Xcp_hw.h"
/*!!! END-INCLUDE-SECTION !!!*/

#include <stdio.h>

#include "canlib.h"

#if _MSC_VER
    #pragma warning(disable: 4996)
#endif /* _MSC_VER */

#define KV_MAX_DLC ((uint16_t)8) /* TODO: depends on classic or FD. */

#if XCP_ENABLE_TRANSPORT_LAYER_CMD == XCP_ON

    #if (XCP_ENABLE_CAN_GET_SLAVE_ID == XCP_ON)
static bool XcpTl_SlaveIdEchoReceived = false;

static void XcpTl_GetSlaveId_Res(Xcp_PduType const * const pdu) {
    /* Validate "XCP" identification pattern at positions 2-4 */
    if (pdu->data[2] != UINT8(0x58) || pdu->data[3] != UINT8(0x43) || pdu->data[4] != UINT8(0x50)) {
        return;
    }

    if (pdu->data[5] == 0) {
        /* Mode 0: identify by echo */
        XcpTl_SlaveIdEchoReceived = true;
        Xcp_Send8(
            UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0x58), UINT8(0x43), UINT8(0x50),
            XCP_LOBYTE(XCP_LOWORD(XCP_ON_CAN_INBOUND_IDENTIFIER)), XCP_HIBYTE(XCP_LOWORD(XCP_ON_CAN_INBOUND_IDENTIFIER)),
            XCP_LOBYTE(XCP_HIWORD(XCP_ON_CAN_INBOUND_IDENTIFIER)), XCP_HIBYTE(XCP_HIWORD(XCP_ON_CAN_INBOUND_IDENTIFIER))
        );
    } else if (pdu->data[5] == 1) {
        /* Mode 1: confirm by inverse echo — only if a prior identify-by-echo was received */
        if (!XcpTl_SlaveIdEchoReceived) {
            return;
        }
        XcpTl_SlaveIdEchoReceived = false;
        Xcp_Send8(
            UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0xA7), UINT8(0xBC), UINT8(0xAF),
            XCP_LOBYTE(XCP_LOWORD(XCP_ON_CAN_INBOUND_IDENTIFIER)), XCP_HIBYTE(XCP_LOWORD(XCP_ON_CAN_INBOUND_IDENTIFIER)),
            XCP_LOBYTE(XCP_HIWORD(XCP_ON_CAN_INBOUND_IDENTIFIER)), XCP_HIBYTE(XCP_HIWORD(XCP_ON_CAN_INBOUND_IDENTIFIER))
        );
    }
}
    #endif /* XCP_ENABLE_CAN_GET_SLAVE_ID */

    #if (XCP_ENABLE_CAN_GET_DAQ_ID == XCP_ON)
extern const uint32_t Xcp_DaqIDs[];
extern const uint16_t Xcp_DaqIDCount;

static void XcpTl_GetDaqId_Res(Xcp_PduType const * const pdu) {
    uint16_t daq_id = (uint16_t)((uint16_t)pdu->data[2] | ((uint16_t)pdu->data[3] << 8u));

    if (daq_id > (Xcp_DaqIDCount - 1)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }
    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(1), UINT8(0), UINT8(0), XCP_LOBYTE(XCP_LOWORD(Xcp_DaqIDs[daq_id])),
        XCP_HIBYTE(XCP_LOWORD(Xcp_DaqIDs[daq_id])), XCP_LOBYTE(XCP_HIWORD(Xcp_DaqIDs[daq_id])),
        XCP_HIBYTE(XCP_HIWORD(Xcp_DaqIDs[daq_id]))
    );
}
    #endif /* XCP_ENABLE_CAN_GET_DAQ_ID */

    #if (XCP_ENABLE_CAN_SET_DAQ_ID == XCP_ON)
extern uint32_t       Xcp_DaqIDs[];
extern const uint16_t Xcp_DaqIDCount;

static void XcpTl_SetDaqId_Res(Xcp_PduType const * const pdu) {
    uint16_t daq_id = (uint16_t)((uint16_t)pdu->data[2] | ((uint16_t)pdu->data[3] << 8u));
    uint32_t can_id = (uint32_t)((uint32_t)pdu->data[4] | ((uint32_t)pdu->data[5] << 8u) | ((uint32_t)pdu->data[6] << 16u) |
                                 ((uint32_t)pdu->data[7] << 24u));

    if (daq_id > (Xcp_DaqIDCount - 1)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }
    Xcp_DaqIDs[daq_id] = can_id;
    Xcp_Send8(UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0));
}
    #endif /* XCP_ENABLE_CAN_SET_DAQ_ID */

#endif /* XCP_ENABLE_TRANSPORT_LAYER_CMD */

typedef struct tagXcpTl_ConnectionType {
    canHandle handle;
    bool      connected;
} XcpTl_ConnectionType;

extern Xcp_PduType Xcp_CtoIn;
extern Xcp_PduType Xcp_CtoOut;

static XcpTl_ConnectionType XcpTl_Connection;

static uint16_t XcpTl_SetDLC(uint16_t len);
static void     XcpTl_SetCANFilter(void);
static void     Kv_Notification(int hnd, void *context, unsigned int notifyEvent);
static bool     XcpTl_MatchingAddress(uint32_t id0, uint32_t id1, uint16_t flag);

void Kv_Error(const char *msg) {
    printf("ERROR[Kvaser]:: %s\n", msg);
}

void Kv_Info(const char *msg) {
    printf("INFO[Kvaser]:: %s\n", msg);
}

void Kv_Check(char *id, canStatus stat) {
    char err_buf[128];
    char msg_buf[128];

    if (stat != canOK) {
        err_buf[0] = '\0';
        canGetErrorText(stat, err_buf, sizeof(err_buf));
        snprintf(msg_buf, sizeof(msg_buf), "%s: failed, stat=%d (%s)\n", id, (int)stat, err_buf);
        Kv_Error(msg_buf);
    }
}

static bool XcpTl_MatchingAddress(uint32_t id0, uint32_t id1, uint16_t flag) {
    uint16_t ext0;
    uint16_t ext1;
    uint32_t identifier;

    ext0       = XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(id0);
    identifier = XCP_ON_CAN_STRIP_IDENTIFIER(id0);
    ext1       = (flag == canMSG_EXT) ? 1 : 0;

    return (identifier == id1) && (ext0 == ext1);
}

static void Kv_Notification(int hnd, void *context, unsigned int notifyEvent) {
    canStatus      stat;
    long           id;
    static uint8_t msg[64];
    unsigned int   dlc;
    unsigned int   flag;
    unsigned long  time;
    unsigned long  status;

    stat = canReadStatus(hnd, &status);
    Kv_Check("canReadStatus", stat);

    switch (notifyEvent) {
        case canNOTIFY_TX:
            // Kv_Info("CAN message sent\n");
            break;
        case canNOTIFY_RX:
            // Kv_Info("CAN message received\n");
            stat = canRead(hnd, &id, msg, &dlc, &flag, &time);
            Kv_Check("canRead", stat);
#if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
            if (dlc != XCP_MAX_CTO) {
                break;
            }
#endif
            if (dlc > 0) {
                if (XcpTl_MatchingAddress(XCP_ON_CAN_INBOUND_IDENTIFIER, id, flag)) {
                    Xcp_CtoIn.len  = dlc;
                    Xcp_CtoIn.data = msg + XCP_TRANSPORT_LAYER_BUFFER_OFFSET;
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else if (XcpTl_MatchingAddress(XCP_ON_CAN_BROADCAST_IDENTIFIER, id, flag)) {
                    printf("BROADCAST request!!!\n");
                }
            }
            // printf("ID: %08x DLC: %d Flags: %08x Timer: %d\n", id, dlc, flag, time);
            break;
        case canNOTIFY_STATUS:
            printf("CAN status %u\n", stat);
            break;
        case canNOTIFY_ERROR:
            Kv_Error("Error frame received");
            break;
        case canNOTIFY_BUSONOFF:
            if (status & canSTAT_ERROR_PASSIVE) {
                Kv_Info("Error-Passive");
            } else if (status & canSTAT_BUS_OFF) {
                Kv_Info("BusOff");
            } else if (status & canSTAT_ERROR_WARNING) {
                Kv_Info("Error-Warning");
            } else if (status & canSTAT_ERROR_ACTIVE) {
                // Kv_Info("Error-Active");
            } else {
                printf("BusOn/BusOff %lu\n", status);
                // ???
            }
            break;
        default:
            printf("Unknown event %d\n", notifyEvent);
            break;
    }
}

void XcpTl_Init(void) {
    canStatus status = canOK;
    uint32_t  ts     = 0UL;
    canHandle hnd    = 0;
    int       chan   = 0;

    canInitializeLibrary();
    chan = atoi(Xcp_Options.can_interf);

    hnd = canOpenChannel(
        0, canOPEN_ACCEPT_VIRTUAL | canOPEN_REQUIRE_INIT_ACCESS
    );  // /* | canOPEN_NO_INIT_ACCESS | canOPEN_ACCEPT_LARGE_DLC*/
    if (hnd < 0) {
        Kv_Check("XcpTl_Init::canOpenChannel", hnd);
        exit(1);
    }

    XcpTl_Connection.handle = hnd;

#if 0
    st = canSetBusParams(hnd, Xcp_Options.can_bitrate, 0, 0, 0, 0, 0);
    if (st != canOK) {
        XcpHw_ErrorMsg("XcpTl_Init::canSetBusParams()", (int)st);
        canClose(hnd);
        return;
    }
    ('sjw', 2), ('tseg1', 5), ('tseg2', 2)
#endif

    status = canSetBusParams(hnd, canBITRATE_500K, 11, 4, 2, 1, 0);
    Kv_Check("canSetBusParams", status);

#if 0
    status = canBusOn(hnd);
    if (status != canOK) {
        XcpHw_ErrorMsg("XcpTl_Init::canBusOn()", status);
        canClose(hnd);
        return;
    }
#endif

    status = kvSetNotifyCallback(
        hnd, (kvCallback_t)&Kv_Notification, NULL,
        canNOTIFY_RX | canNOTIFY_TX | canNOTIFY_ERROR | canNOTIFY_STATUS | canNOTIFY_BUSONOFF
    );
    Kv_Check("kvSetNotifyCallback", status);

    // status = canSetBusParams(hnd, XCP_ON_CAN_FREQ, XCP_ON_CAN_TSEG1, XCP_ON_CAN_TSEG2, XCP_ON_CAN_SJW, XCP_ON_CAN_NOSAMP, 0);
    // Kv_Check("SetBusParams", status);

    ts     = 10; /* We are using 10µS timer resolution. */
    status = canIoCtl(hnd, canIOCTL_SET_TIMER_SCALE, &ts, 4);
    Kv_Check("SET_TIME_SCALE", status);

    // XcpTl_SetCANFilter();

    canFlushReceiveQueue(hnd);
    canFlushTransmitQueue(hnd);

    status = canBusOn(hnd);
    Kv_Check("canBusOn", status);
}

void XcpTl_DeInit(void) {
    canStatus stat;

    Kv_Info("Going off bus and closing channel");
    stat = canBusOff(XcpTl_Connection.handle);
    Kv_Check("canBusOff", stat);
    canHandle hnd = XcpTl_Connection.handle;
    if (hnd > 0) {
        (void)canBusOff(hnd);
        (void)canClose(hnd);
        XcpTl_Connection.handle = 0;
    }
}

static uint16_t XcpTl_SetDLC(uint16_t len) {
    static const uint16_t FD_DLCS[] = { 12, 16, 20, 24, 32, 48, 64 };
    uint8_t               idx;

    if (len <= 8) {
        return len;
    } else {
        for (idx = 0; idx < XCP_ARRAY_SIZE(FD_DLCS); ++idx) {
            if (len <= FD_DLCS[idx]) {
                return FD_DLCS[idx];
            }
        }
    }
}

static void XcpTl_SetCANFilter(void) {
    uint32_t  i0;
    uint32_t  i1;
    uint32_t  filter;
    uint32_t  mask;
    int       ext;
    canStatus stat;

    i0 = XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER);
    i1 = XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_BROADCAST_IDENTIFIER);

    filter = i0 & i1;
    mask   = (i0 | i1) ^ filter;

    if ((XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER)) ||
        (XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_BROADCAST_IDENTIFIER))) {
        ext = 1;
        mask ^= 0x1fffffff;
    } else {
        ext = 0;
        mask ^= 0x7ff;
    }
    stat = canSetAcceptanceFilter(XcpTl_Connection.handle, filter, mask, ext);
    Kv_Check("canSetAcceptanceFilter", stat);
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    canStatus stat;
    int       id;
    int       ext;
    int       flag;
    char      msg_buf[256];

    if (len > KV_MAX_DLC) {
        snprintf(msg_buf, sizeof(msg_buf), "XcpTl_Send -- at most %d bytes supported, got %d.\n", KV_MAX_DLC, len);
        Kv_Error((const char *)msg_buf);
        return;
    }

    XCP_TL_ENTER_CRITICAL();
#if (XCP_DAQ_ENABLE_PID_OFF == XCP_ON)
    if (Xcp_DtoCanId != 0) {
        id           = (int)Xcp_DtoCanId;
        ext          = 0;
        Xcp_DtoCanId = 0;
    } else {
        id  = XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER);
        ext = XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER);
    }
#else
    id  = XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER);
    ext = XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER);
#endif
    flag = ext ? canMSG_EXT : canMSG_STD;
    printf("Send to: %08x DLC: %d\n", id, len);
    stat = canWrite(XcpTl_Connection.handle, id, (void *)&buf[0], len, flag);
    Kv_Check("canWrite", stat);

    stat = canWriteSync(stat, 0xFFFFFFFF); /* 100 ms Timeout */
    Kv_Check("canWriteSync", stat);

    XCP_TL_LEAVE_CRITICAL();
    if (buf == NULL || len == 0) {
        return;
    }
    if (len > 8U) {
        XcpHw_ErrorMsg("XcpTl_Send (Kvaser): DLC > 8 not supported", EINVAL);
        return;
    }

#if 0
    long         id  = Xcp_Options.can_id; /* Annahme: Standard-ID */
    unsigned int dlc = (unsigned int)len;
    canStatus    st  = canWrite(hnd, id, buf, dlc, 0);
    if (st != canOK) {
        XcpHw_ErrorMsg("XcpTl_Send::canWrite()", (int)st);
        return;
    }

#endif
}

void *XcpTl_Thread(void *param) {
    XCP_UNREFERENCED_PARAMETER(param);
    XCP_FOREVER {
        XcpHw_Sleep(20 * 1000);
        // XcpTl_MainFunction();
    }
    return NULL;
}

void XcpTl_SaveConnection(void) {
}

void XcpTl_ReleaseConnection(void) {
}

void XcpTl_PostQuitMessage(void) {
}

bool XcpTl_VerifyConnection(void) {
    return XCP_TRUE;
}

void XcpTl_FeedReceiver(uint8_t octet) {
}

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
#if XCP_ENABLE_TRANSPORT_LAYER_CMD == XCP_ON
    #if (XCP_ENABLE_CAN_GET_SLAVE_ID == XCP_ON)
    if (pdu->data[1] == UINT8(XCP_GET_SLAVE_ID)) {
        XcpTl_GetSlaveId_Res(pdu);
        return;
    }
    #endif /* XCP_ENABLE_CAN_GET_SLAVE_ID */

    #if (XCP_ENABLE_CAN_GET_DAQ_ID == XCP_ON)
    if (pdu->data[1] == UINT8(XCP_GET_DAQ_ID)) {
        XcpTl_GetDaqId_Res(pdu);
        return;
    }
    #endif /* XCP_ENABLE_CAN_GET_DAQ_ID */

    #if (XCP_ENABLE_CAN_SET_DAQ_ID == XCP_ON)
    if (pdu->data[1] == UINT8(XCP_SET_DAQ_ID)) {
        XcpTl_SetDaqId_Res(pdu);
        return;
    }
    #endif /* XCP_ENABLE_CAN_SET_DAQ_ID */

    Xcp_ErrorResponse(UINT8(ERR_CMD_UNKNOWN));
#else

#endif /* XCP_ENABLE_TRANSPORT_LAYER_CMD */
}

void XcpTl_SetOptions(Xcp_OptionsType const *options) {
    CopyMemory(&Xcp_Options, options, sizeof(Xcp_OptionsType));
}

void XcpTl_PrintConnectionInformation(void) {
    canStatus stat;
    int       num;
    char      name[128];
    bool      ext;

    ext  = XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER);
    stat = canGetChannelData(XcpTl_Connection.handle, canCHANNELDATA_CHAN_NO_ON_CARD, &num, sizeof(num));
    Kv_Check("canGetChannelData", stat);
    stat = canGetChannelData(XcpTl_Connection.handle, canCHANNELDATA_DEVDESCR_ASCII, &name, sizeof(name));
    Kv_Check("canGetChannelData", stat);
    printf(
        "\nXCPonCAN -- %s (channel %d), listening on 0x%X [%s]\n", name, num,
        XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER), ext ? "EXT" : "STD"
    );
}

void XcpTl_RxHandler(void) {
    canHandle hnd    = XcpTl_Connection.handle;
    canStatus status = canOK;
    if (hnd <= 0) {
        return;
    }
    long          id;
    unsigned int  dlc   = 0;
    unsigned int  flags = 0;
    unsigned long time  = 0;
    uint8_t       data[8];

    while (1) {
        status = canReadWait(hnd, &id, data, &dlc, &flags, &time, 0);
        if (status == canERR_NOMSG) {
            return;
        }
        if (status != canOK) {
            XcpHw_ErrorMsg("XcpTl_RxHandler::canReadWait()", status);
            return;
        }
        if (dlc > 8U) {
            XcpHw_ErrorMsg("XcpTl_RxHandler (Kvaser): DLC > 8", EPROTO);
            continue;
        }
        if (dlc > XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE) {
            XcpHw_ErrorMsg("XcpTl_RxHandler (Kvaser): DLC exceeds TL buffer", EOVERFLOW);
            continue;
        }
        Xcp_CtoIn.len = (uint16_t)dlc;
        XcpUtl_MemCopy(Xcp_CtoIn.data, data, dlc);
        Xcp_DispatchCommand(&Xcp_CtoIn);
    }
}
