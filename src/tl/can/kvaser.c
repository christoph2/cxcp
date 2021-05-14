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


#include "Xcp.h"
#include "Xcp_hw.h"
#include "canlib.h"

#include <stdio.h>


#if _MSC_VER
#pragma warning(disable: 4996)
#endif /* _MSC_VER */


#define KV_MAX_DLC     ((uint16_t)8)   /* TODO: depends on classic or FD. */


#if 0
static const uint8_t GET_SLAVE_ID_ECHO[] = {UINT8(0x58), UINT8(0x43), UINT8(0x50)};
static const uint8_t GET_SLAVE_ID_INVERSE_ECHO[] = {UINT8(0xA7), UINT8(0xBC), UINT8(0xAF)};
#endif

typedef struct tagXcpTl_ConnectionType {
    int virtualChannel;
    int handle;
 } XcpTl_ConnectionType;

extern Xcp_PDUType Xcp_CtoIn;
extern Xcp_PDUType Xcp_CtoOut;

static XcpTl_ConnectionType XcpTl_Connection;
static uint8_t Xcp_PduOutBuffer[XCP_MAX_CTO] = {0};

static uint16_t XcpTl_SetDLC(uint16_t len);
static void XcpTl_SetCANFilter(void);
static void Kv_Notification(int hnd, void * context, unsigned int notifyEvent);
static bool XcpTl_MatchingAddress(uint32_t id0, uint32_t id1, uint16_t flag);

void Kv_Error(const char * msg)
{
    printf("ERROR[Kvaser]:: %s\n", msg);
}

void Kv_Info(const char * msg)
{
    printf("INFO[Kvaser]:: %s\n", msg);
}

void Kv_Check(char* id, canStatus stat)
{
    char err_buf[128];
    char msg_buf[128];

    if (stat != canOK) {
        err_buf[0] = '\0';
        canGetErrorText(stat, err_buf, sizeof(err_buf));
        sprintf(msg_buf, "%s: failed, stat=%d (%s)\n", id, (int)stat, err_buf);
        Kv_Error(msg_buf);
    }
}


static bool XcpTl_MatchingAddress(uint32_t id0, uint32_t id1, uint16_t flag)
{
    uint16_t ext0;
    uint16_t ext1;
    uint32_t identifier;

    ext0 = XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(id0);
    identifier = XCP_ON_CAN_STRIP_IDENTIFIER(id0);
    ext1 = (flag == canMSG_EXT) ? 1 : 0;

    return (identifier == id1) && (ext0 == ext1);
}


static void Kv_Notification(int hnd, void * context, unsigned int notifyEvent)
{
    canStatus stat;
    long id;
    uint8_t msg[64];
    unsigned int dlc;
    unsigned int flag;
    unsigned long time;
    unsigned long status;
    //char msg_buffer[128];

    stat = canReadStatus(hnd, &status);
    Kv_Check("canReadStatus", stat);

    switch (notifyEvent) {
        case canNOTIFY_TX:
            //Kv_Info("CAN message sent\n");
            break;
        case canNOTIFY_RX:
            //Kv_Info("CAN message received\n");
            stat = canRead(hnd, &id, msg, &dlc, &flag, &time);
            Kv_Check("canRead", stat);
#if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
            if (dlc != XCP_MAX_CTO) {
                break;
            }
#endif
            if (dlc > 0) {
                if (XcpTl_MatchingAddress(XCP_ON_CAN_INBOUND_IDENTIFIER, id, flag)) {
                    Xcp_CtoIn.len = dlc;
                    Xcp_CtoIn.data = msg + XCP_TRANSPORT_LAYER_BUFFER_OFFSET;
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else if (XcpTl_MatchingAddress(XCP_ON_CAN_BROADCAST_IDENTIFIER, id, flag)) {
                    printf("BROADCAST request!!!\n");
                }
            }
#if 0
#define  canMSG_MASK   0x00ff
#define  canMSG_RTR   0x0001
#define  canMSG_STD   0x0002
#define  canMSG_EXT   0x0004
#define  canMSG_WAKEUP   0x0008
#define  canMSG_NERR   0x0010
#define  canMSG_ERROR_FRAME   0x0020
#define  canMSG_TXACK   0x0040
#define  canMSG_TXRQ   0x0080
#endif
            //printf("ID: %08x DLC: %d Flags: %08x Timer: %d\n", id, dlc, flag, time);
            break;
        case canNOTIFY_STATUS:
            /* printf("CAN status %u\n", stat); */
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
                /** Kv_Info("Error-Active"); */
            } else {
                printf("BusOn/BusOff %lu\n", status);  // ???
            }
            break;
        default:
            printf("Unknown event %d\n", notifyEvent);
            break;
        }
}

void XcpTl_Init(void)
{
    canStatus stat;
    uint32_t ts;
    //int code;
    //int ext;
    int hnd;

    Xcp_CtoOut.data = &Xcp_PduOutBuffer[0];
    canInitializeLibrary();
    hnd = canOpenChannel(0, canOPEN_ACCEPT_VIRTUAL | canOPEN_NO_INIT_ACCESS /* | canOPEN_ACCEPT_LARGE_DLC*/);
    if(hnd < 0){
        Kv_Check("canOpenChannel", hnd);
        exit(1);
    }

    XcpTl_Connection.handle = hnd;

    stat = kvSetNotifyCallback(hnd, (kvCallback_t)&Kv_Notification, NULL,
        canNOTIFY_RX | canNOTIFY_TX | canNOTIFY_ERROR | canNOTIFY_STATUS | canNOTIFY_BUSONOFF
    );
    Kv_Check("kvSetNotifyCallback", stat);

    stat = canSetBusParams(hnd, XCP_ON_CAN_FREQ, XCP_ON_CAN_TSEG1, XCP_ON_CAN_TSEG2, XCP_ON_CAN_SJW, XCP_ON_CAN_NOSAMP, 0);
    Kv_Check("SetBusParams", stat);

    ts = 10;  /* We are using 10µS timer resolution. */
    stat = canIoCtl(hnd, canIOCTL_SET_TIMER_SCALE, &ts, 4);
    Kv_Check("SET_TIME_SCALE", stat);

    XcpTl_SetCANFilter();

    canFlushReceiveQueue(hnd);
    canFlushTransmitQueue(hnd);

    stat = canBusOn(hnd);
    Kv_Check("canBusOn", stat);
}


void XcpTl_DeInit(void)
{
    canStatus stat;

    Kv_Info("Going off bus and closing channel");
    stat = canBusOff(XcpTl_Connection.handle);
    Kv_Check("canBusOff", stat);
    //stat = canClose(hnd);
    //Kv_Check("canClose", stat);
}

#if 0
int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec)
{
    return 1;
}
#endif


static uint16_t XcpTl_SetDLC(uint16_t len)
{
    static const uint16_t FD_DLCS[] = {12, 16, 20, 24, 32, 48, 64};
    uint8_t idx;

    if (len <= 8) {
        return len;
    } else {
        for (idx = 0; idx < sizeof(FD_DLCS); ++idx) {
            if (len <= FD_DLCS[idx]) {
                return FD_DLCS[idx];
            }
        }
    }
}


static void XcpTl_SetCANFilter(void)
{
    uint32_t i0;
    uint32_t i1;
    uint32_t filter;
    uint32_t mask;
    int ext;
    int stat;

    i0 = XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER);
    i1 = XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_BROADCAST_IDENTIFIER);

    filter = i0 & i1;
    mask = (i0 | i1) ^ filter;

    if ((XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER)) || (XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_BROADCAST_IDENTIFIER))) {
        ext = 1;
        mask ^= 0x1fffffff;
    } else {
        ext = 0;
        mask ^= 0x7ff;
    }
    stat = canSetAcceptanceFilter(XcpTl_Connection.handle, filter, mask, ext);
    Kv_Check("canSetAcceptanceFilter", stat);
}

void XcpTl_Send(uint8_t const * buf, uint16_t len)
{
    int stat;
    int id;
    int ext;
    int flag;
    char msg_buf[256];

    XCP_TL_ENTER_CRITICAL();
    if (len > KV_MAX_DLC) {
        sprintf(msg_buf, "XcpTl_Send -- at most %d bytes supported, got %d.\n", KV_MAX_DLC, len);
        Kv_Error((const char *)msg_buf);
        return;
    }

    id = XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER);
    ext = XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER);
    flag = ext ? canMSG_EXT : canMSG_STD;
    stat = canWrite(XcpTl_Connection.handle, id, (void*)buf, len, flag);
    Kv_Check("canWrite", stat);
    XCP_TL_LEAVE_CRITICAL();
}


void XcpTl_MainFunction(void)
{
    Sleep(1);
#if 0
    if (XcpTl_FrameAvailable(0, 1000) > 0) {
        XcpTl_RxHandler();
    }
#endif
}

#if 0
void XcpTl_RxHandler(void)
{

}
#endif

void XcpTl_SaveConnection(void)
{

}


void XcpTl_ReleaseConnection(void)
{

}

void XcpTl_PostQuitMessage(void)
{

}

bool XcpTl_VerifyConnection(void)
{
    return XCP_TRUE;
}


void XcpTl_FeedReceiver(uint8_t octet)
{

}

void XcpTl_TransportLayerCmd_Res(Xcp_PDUType const * const pdu)
{

}

void XcpTl_SetOptions(Xcp_OptionsType const * options)
{
    CopyMemory(&Xcp_Options, options, sizeof(Xcp_OptionsType));
}

void XcpTl_PrintConnectionInformation(void)
{
    int stat;
    int num;
    char name[128];
    bool ext;

    ext = XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER);
    stat = canGetChannelData(XcpTl_Connection.handle, canCHANNELDATA_CHAN_NO_ON_CARD, &num, sizeof(num));
    Kv_Check("canGetChannelData", stat);
    stat = canGetChannelData(XcpTl_Connection.handle, canCHANNELDATA_DEVDESCR_ASCII, &name, sizeof(name));
    Kv_Check("canGetChannelData", stat);
    printf("\nXCPonCAN -- %s (channel %d), listening on 0x%X [%s]\n", name, num,
        XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER), ext ? "EXT" : "STD"
    );
}


