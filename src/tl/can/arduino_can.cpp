
/*
 * BlueParrot XCP
 *
 * (C) 2022-2025 by Christoph Schueler <github.com/Christoph2,
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

#include "xcp_config.h"

#if XCP_TRANSPORT_LAYER == XCP_ON_CAN

    #if (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD) || (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD)
        #include <SPI.h>
        #include <can-serial.h>
        #include <mcp2515_can.h>
        #include <mcp2515_can_dfs.h>
        #include <mcp2518fd_can.h>
        #include <mcp2518fd_can_dfs.h>
        #include <mcp_can.h>
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_SPARKFUN_CAN_SHIELD
        #include <Canbus.h>
        #include <defaults.h>
        #include <global.h>
        #include <mcp2515.h>
        #include <mcp2515_defs.h>
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_MKR_ZERO_CAN_SHIELD
        #include <CAN.h>
    #else
        #error "Unsupported XCP_CAN_INTERFACE"
    #endif

    #include <stdint.h>

uint32_t              filter_mask(uint32_t identifier);
extern const uint32_t Xcp_DaqIDs[];
extern const uint16_t Xcp_DaqIDCount;

static const char XCP_MAGIC[] = "XCP";

static bool          connected = false;
static volatile bool XcpTl_FrameReceived{ false };

static unsigned char XcpTl_Buffer[64];
static unsigned char XcpTl_Dlc = 0;
static int           XcpTl_ID  = 0;

static void on_receive_seedstudio();
static void on_receive_mkr(int packetSize);

    #if (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD) || (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD)
        #if XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD
mcp2515_can CAN(XCP_CAN_IF_MCP25XX_PIN_CS);
        #elif XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD
mcp2518fd CAN(XCP_CAN_IF_MCP25XX_PIN_CS);
        #endif
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_MKR_ZERO_CAN_SHIELD
        /* MKR Zero CAN shield uses built-in CAN object; no explicit declaration needed here. */
    #else
        #error "Unsupported XCP_CAN_INTERFACE"
    #endif

void XcpTl_Init(void) {
    Serial.begin(9600);
    while (!Serial) {
    }

    Serial.println("Starting Blueparrot XCP...");

    #if (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD) || (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD)
    attachInterrupt(digitalPinToInterrupt(XCP_CAN_IF_MCP25XX_PIN_INT), on_receive_seedstudio, FALLING);

    while (CAN_OK != CAN.begin(XCP_ON_CAN_FREQ)) {
        Serial.println("CAN init (SeedStudio) failed!, retry...");
        delay(100);
    }

    Serial.println("CAN init (SeedStudio) OK!");

    CAN.init_Mask(0, XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER), filter_mask(XCP_ON_CAN_INBOUND_IDENTIFIER));
    CAN.init_Mask(
        1, XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_BROADCAST_IDENTIFIER), filter_mask(XCP_ON_CAN_BROADCAST_IDENTIFIER)
    );

    CAN.init_Filt(0, XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER), XCP_ON_CAN_INBOUND_IDENTIFIER);
    CAN.init_Filt(1, XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_BROADCAST_IDENTIFIER), XCP_ON_CAN_BROADCAST_IDENTIFIER);
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_SPARKFUN_CAN_SHIELD
    if (Canbus.init(CANSPEED_500)) { // TDO: Make speed configurable
        Serial.println("CAN init (SparkFun) OK!");
    } else {
        Serial.println("CAN init (SparkFun) failed!");
        while (1) {
        }
    }
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_MKR_ZERO_CAN_SHIELD

    if (!CAN.begin(XCP_ON_CAN_FREQ)) {
        Serial.println("Starting CAN (MKR Zero) failed!");
        while (1) {
        }
    } else {
        Serial.println("CAN init ((MKR Zero) OK!");
    }
    CAN.setTimeout(1000);
    CAN.onReceive(on_receive_mkr);
    #else
    #error "Unsupported XCP_CAN_INTERFACE"
    #endif
}

void XcpTl_DeInit(void) {
}

void *XcpTl_Thread(void *param) {
    XCP_FOREVER {
        XcpTl_MainFunction();
    }
    return NULL;
}

void XcpTl_MainFunction(void) {
    if (XcpTl_FrameAvailable(0, 1000) > 0) {
        XcpTl_FrameReceived = false;
        XcpTl_RxHandler();
    }
}

#if (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD) || (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD)
static void on_receive_seedstudio() {
    if (CAN.checkReceive() == CAN_MSGAVAIL) {
        XcpTl_FrameReceived = true;
        CAN.readMsgBuf(&XcpTl_Dlc, static_cast<byte *>(XcpTl_Buffer));
        // XcpTl_ID
        //  canBusPacket.id = CAN.getCanId();
    }
}
#elif XCP_CAN_INTERFACE == XCP_CAN_IF_MKR_ZERO_CAN_SHIELD
static void on_receive_mkr(int packetSize) {
    uint_least8_t idx = 0;
    XcpTl_Dlc         = packetSize;
    XcpTl_FrameReceived = true;
    if (CAN.packetExtended()) {
    }
    if (CAN.packetRtr()) {
    } else {
        while (CAN.available()) {
            XcpTl_Buffer[idx++] = CAN.read();
        }
    }
}
#endif

void XcpTl_RxHandler(void) {
    XcpUtl_MemCopy(Xcp_CtoIn.data, XcpTl_Buffer, XcpTl_Dlc);
    Xcp_DispatchCommand(&Xcp_CtoIn);
}

void XcpTl_TxHandler(void) {
}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec) {
    #if XCP_CAN_INTERFACE == XCP_CAN_IF_SPARKFUN_CAN_SHIELD
    if (mcp2515_check_message()) {
        if (mcp2515_get_message(&message)) {
            XcpTl_FrameReceived = true;
            XcpTl_Dlc           = message.header.length;
            XcpTl_ID            = message.id;
            for (int i = 0; i < message.header.length; i++) {
                XcpTl_Buffer[i] = message.data[i];
            }
            return 1;
        }
    }
    return 0;
    #else
    return static_cast<int16_t>(XcpTl_FrameReceived);
    #endif
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    uint32_t can_id;

    can_id = XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER);

    #if (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD) || (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD)

    CAN.sendMsgBuf(can_id, XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER), len, buf);

    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_SPARKFUN_CAN_SHIELD
    message.id = can_id;
    message.header.rtr = 0;
    message.header.length = len;
    for (int i = 0; i < len; i++) {
        message.data[i] = buf[i];
    }
    mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
    mcp2515_send_message(&message);
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_MKR_ZERO_CAN_SHIELD

    if (XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER)) {
        CAN.beginExtendedPacket(can_id, len);
    } else {
        CAN.beginPacket(can_id);
    }

    CAN.write(buf, len);
    CAN.endPacket();
    #else
    #error "Unsupported XCP_CAN_INTERFACE"
    #endif
}

uint32_t filter_mask(uint32_t identifier) {
    if (XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(identifier)) {
        return (2UL << (29 - 1)) - 1;
    } else {
        return (2UL << (11 - 1)) - 1;
    }
}

void XcpTl_SaveConnection(void) {
    connected = XCP_TRUE;
}

void XcpTl_ReleaseConnection(void) {
    connected = XCP_FALSE;
}

bool XcpTl_VerifyConnection(void) {
    return connected;
}

void XcpTl_PrintConnectionInformation(void) {
}

    #if XCP_ENABLE_TRANSPORT_LAYER_CMD == XCP_ON
        #if (XCP_ENABLE_CAN_GET_SLAVE_ID == XCP_ON)
void XcpTl_GetSlaveId_Res(Xcp_PduType const * const pdu) {
    uint8_t mask = 0x00;

    if (pdu->data[2]) {
    }

    if (pdu->data[5] == 1) {
        /*
         Mode
            0 = identify by echo
            1 = confirm by inverse echo
        */
        mask = 0xff;
    }

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0x58 ^ mask), UINT8(0x43 ^ mask), UINT8(0x50 ^ mask),
        XCP_LOBYTE(XCP_LOWORD(XCP_ON_CAN_OUTBOUND_IDENTIFIER)), XCP_HIBYTE(XCP_LOWORD(XCP_ON_CAN_OUTBOUND_IDENTIFIER)),
        XCP_LOBYTE(XCP_HIWORD(XCP_ON_CAN_OUTBOUND_IDENTIFIER)), XCP_HIBYTE(XCP_HIWORD(XCP_ON_CAN_OUTBOUND_IDENTIFIER))
    );
}
        #endif /* XCP_ENABLE_CAN_GET_SLAVE_ID */

        #if (XCP_ENABLE_CAN_GET_DAQ_ID == XCP_ON)
void XcpTl_GetDaqId_Res(Xcp_PduType const * const pdu) {
    uint8_t daq_id = pdu->data[2];

    if (daq_id > (Xcp_DaqIDCount - 1)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    };
    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(1), UINT8(0), UINT8(0), XCP_LOBYTE(XCP_LOWORD(Xcp_DaqIDs[daq_id])),
        XCP_HIBYTE(XCP_LOWORD(Xcp_DaqIDs[daq_id])), XCP_LOBYTE(XCP_HIWORD(Xcp_DaqIDs[daq_id])),
        XCP_HIBYTE(XCP_HIWORD(Xcp_DaqIDs[daq_id]))
    );
}
        #endif /* XCP_ENABLE_CAN_GET_DAQ_ID */

        #if (XCP_ENABLE_CAN_SET_DAQ_ID == XCP_ON)
void XcpTl_SetDaqId_Res(Xcp_PduType const * const pdu) {
}
        #endif /* XCP_ENABLE_CAN_SET_DAQ_ID */

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
        #if (XCP_ENABLE_CAN_GET_SLAVE_ID == XCP_ON)
    if (pdu->data[1] == UINT8(XCP_GET_SLAVE_ID)) {
        XcpTl_GetSlaveId_Res(pdu);  // TODO: This is a response to a broadcast message.
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
}
    #endif /* XCP_ENABLE_TRANSPORT_LAYER_CMD */

#endif /* XCP_TRANSPORT_LAYER == XCP_ON_CAN */

/////
#define SIZE 1000

// A class to store a queue
