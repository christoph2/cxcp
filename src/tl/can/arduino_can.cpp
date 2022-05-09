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

static bool connected = false;

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
  CAN.setTimeout(1000);
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
    Serial.println(dlc);

    int actual = CAN.readBytes(buffer, dlc);
    Serial.print("actaul length: ");
    Serial.print(actual);
    Serial.println();

    Xcp_CtoIn.len = dlc;
    // Xcp_CtoIn.data = buffer + XCP_TRANSPORT_LAYER_BUFFER_OFFSET;

    XcpUtl_MemCopy(Xcp_CtoIn.data, &buffer, dlc);

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
  int res;
  XCP_UNREFERENCED_PARAMETER(sec);
  XCP_UNREFERENCED_PARAMETER(usec);

  res = CAN.parsePacket();
  if (res != 0) {
    Serial.print("Frames avail!!!");
    Serial.println();
  }

  return res;
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

void XcpTl_SaveConnection(void) { connected = XCP_TRUE; }

void XcpTl_ReleaseConnection(void) { connected = XCP_FALSE; }

bool XcpTl_VerifyConnection(void) { return connected; }

void XcpTl_PrintConnectionInformation(void) {}

#endif /* XCP_TRANSPORT_LAYER == XCP_ON_CAN */
