/*
 * BlueParrot XCP
 *
 * (C) 2022-2024 by Christoph Schueler <github.com/Christoph2,
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

/*!!! START-INCLUDE-SECTION !!!*/
#include "queue.h"
/*!!! END-INCLUDE-SECTION !!!*/

#if XCP_TRANSPORT_LAYER == XCP_ON_CAN

    #include <CAN.h>
    #include <stdint.h>

extern const uint32_t Xcp_DaqIDs[];
extern const uint16_t Xcp_DaqIDCount;

static const char XCP_MAGIC[] = "XCP";

static bool          connected = false;
static volatile bool XcpTl_FrameReceived{ false };

static char XcpTl_Buffer[64];
static int  XcpTl_Dlc = 0;

static void on_receive(int packetSize);

void XcpTl_Init(void) {
    Serial.begin(9600);
    while (!Serial) {
    }

    Serial.println("Starting Blueparrot XCP...");

    // start the CAN bus at 500 kbps
    if (!CAN.begin(XCP_ON_CAN_FREQ)) {
        Serial.println("Starting CAN failed!");
        while (1) {
        }
    }
    CAN.setTimeout(1000);
    CAN.onReceive(on_receive);
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

// ARDUINO_API_VERSION

static void on_receive(int packetSize) {
    uint_least8_t idx = 0;

    XcpTl_Dlc           = packetSize;
    XcpTl_FrameReceived = true;

    // received a packet
    // Serial.print("Received ");

    if (CAN.packetExtended()) {
        // Serial.print("extended ");
    }

    if (CAN.packetRtr()) {
        // Remote transmission request, packet contains no data
        // Serial.print("RTR ");
    }

    // Serial.print("packet with id 0x");
    // Serial.print(CAN.packetId(), HEX);

    if (CAN.packetRtr()) {
        // Serial.print(" and requested length ");
        // Serial.println(CAN.packetDlc());
    } else {
        // Serial.print(" and length ");
        // Serial.println(packetSize);

        // only print packet data for non-RTR packets
        while (CAN.available()) {
            XcpTl_Buffer[idx++] = CAN.read();
        }
    }
    // Serial.println();
}

void XcpTl_RxHandler(void) {
    // Serial.print(" Buffer: ");
    // XcpUtl_Hexdump((uint8_t*)XcpTl_Buffer, XcpTl_Dlc);
    // Serial.print(" DLC: ");
    // Serial.print(XcpTl_Dlc);
    // Serial.println();
    XcpUtl_MemCopy(Xcp_CtoIn.data, &XcpTl_Buffer, XcpTl_Dlc);
    Xcp_DispatchCommand(&Xcp_CtoIn);
}

void XcpTl_TxHandler(void) {
}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec) {
    return static_cast<int16_t>(XcpTl_FrameReceived);
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    uint32_t can_id;

    can_id = XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER);

    if (XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER)) {
        CAN.beginExtendedPacket(can_id, len);
    } else {
        CAN.beginPacket(can_id);
    }

    CAN.write(buf, len);
    CAN.endPacket();
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
