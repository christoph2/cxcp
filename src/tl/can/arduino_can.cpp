/*
 * BlueParrot XCP
 *
 * (C) 2022 by Christoph Schueler <github.com/Christoph2,
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

    #include <CAN.h>

    #include <atomic>
    #include <cstdint>

static bool             connected = false;
static std::atomic_bool XcpTl_FrameReceived{ false };

static char XcpTl_Buffer[64];
static int  XcpTl_Dlc = 0;

static void on_receive(int packetSize);

void XcpTl_Init(void) {
    Serial.begin(9600);
    while (!Serial) {
    }

    Serial.println("Starting Blueparrot XCP...");

    // start the CAN bus at 500 kbps
    if (!CAN.begin(250E3)) {
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

#endif /* XCP_TRANSPORT_LAYER == XCP_ON_CAN */
