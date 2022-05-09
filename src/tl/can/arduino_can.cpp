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

#include <stdint.h>

#include <CAN.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_hw.h"
/*!!! END-INCLUDE-SECTION !!!*/

// CAN TX Variables
unsigned long prevTX = 0;         // Variable to store last execution time
const unsigned int invlTX = 1000; // One second interval constant
byte data[] = {0xAA, 0x55, 0x01, 0x10,
               0xFF, 0x12, 0x34, 0x56}; // Generic CAN data to send

// MCP2515 INT and CS Pins.
#define CAN0_INT (2)
#define CAN0_CS (9) // 10

XcpTl_ConnectionType XcpTl_Connection;

void XcpTl_Init(void) {
  Serial.begin(9600);
  while (!Serial) {
  }

  Serial.println("Starting Blueparrot XCP...");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1) {
    }
  }
}

void XcpTl_DeInit(void) {}

void *XcpTl_Thread(void *param) {
  XCP_FOREVER { XcpTl_MainFunction(); }
  return NULL;
}

void XcpTl_MainFunction(void) {
  if (XcpTl_FrameAvailable(0, 1000) > 0) {
    XcpTl_RxHandler();
  }
}

void XcpTl_RxHandler(void) {
  static byte buffer[64];
  byte dlc;

  Serial.print("Received ");

  dlc = CAN.packetDlc();
  if (CAN.packetExtended()) {
    Serial.print("extended ");
  }

  if (CAN.packetRtr()) {
    // Remote transmission request, packet contains no data
    Serial.print("RTR ");
  }

  Serial.print("packet with id 0x");
  Serial.print(CAN.packetId(), HEX);

  if (CAN.packetRtr()) {
    Serial.print(" and requested length ");
    Serial.println(dlc);
    return;
  } else {
    Serial.print(" and length ");
    Serial.println(packetSize);

    CAN.readBytes(buffer, 64);

    Xcp_CtoIn.len = dlc;
    Xcp_CtoIn.data = buffer + XCP_TRANSPORT_LAYER_BUFFER_OFFSET;
    Xcp_DispatchCommand(&Xcp_CtoIn);
    // only print packet data for non-RTR packets
    // while (CAN.available()) {
    //    Serial.print((char)CAN.read());
    //}
    Serial.println();
  }

  Serial.println();
}

void XcpTl_TxHandler(void) {}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec) {

  XCP_UNREFERENCED_PARAMETER(sec);
  XCP_UNREFERENCED_PARAMETER(usec);

  return CAN.parsePacket();
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
  // CAN.beginPacket(id, dlc);
  // CAN.beginExtendedPacket(id, dlc);

  // CAN.write(buf, len);
  // CAN.endPacket();
}

void XcpTl_SaveConnection(void) { XcpTl_Connection.connected = XCP_TRUE; }

void XcpTl_ReleaseConnection(void) { XcpTl_Connection.connected = XCP_FALSE; }

bool XcpTl_VerifyConnection(void) { return XCP_TRUE; }

void XcpTl_PrintConnectionInformation(void) {}

#endif /* XCP_TRANSPORT_LAYER == XCP_ON_CAN */
