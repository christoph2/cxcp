/*
 * pySART - Simplified AUTOSAR-Toolkit for Python.
 *
 * (C) 2007-2018 by Christoph Schueler <github.com/Christoph2,
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

#include <stdint.h>

#include "Arduino.h"

#include "xcp.h"

#define XCP_COMM_BUFLEN  (MAX(XCP_MAX_CTO, XCP_MAX_DTO))

#define XCP_SXI_MAKEWORD(buf, offs)  (*(buf+offs)) | (( *(buf+offs+1) << 8))

void ping(void);

void Xcp_DispatchCommand(Xcp_PDUType const * const pdu);

extern Xcp_PDUType Xcp_PduIn;
extern Xcp_PDUType Xcp_PduOut;

static uint8_t Xcp_PduOutBuffer[256] = {0};

typedef enum tagXcpTl_ReceiverStateType {
    XCP_RCV_IDLE,
    XCP_RCV_UNTIL_LENGTH,
    XCP_RCV_REMAINING,
} XcpTl_ReceiverStateType;


typedef struct tagXcpTl_ReceiverType {
    uint8_t Buffer[/*XCP_COMM_BUFLEN*/256];
    XcpTl_ReceiverStateType State;
    uint16_t Index;
    uint16_t Remaining;
    uint16_t Dlc;   // TODO: config!!!
    uint16_t Ctr;   // TODO: config!!!
} XcpTl_ReceiverType;


static XcpTl_ReceiverType XcpTl_Receiver;


void XcpTl_Init(void)
{
    XcpTl_Receiver.State = XCP_RCV_IDLE;
    XcpTl_Receiver.Index = 0u;
    XcpTl_Receiver.Dlc = 0u;
    XcpTl_Receiver.Ctr = 0u;
    XcpTl_Receiver.Remaining = 0u;

    Xcp_PduOut.data = &Xcp_PduOutBuffer[0];
}


void XcpTl_RxHandler(void)
{

}


//  DLC   CTR   PAYLOAD
// [02 00 00 00 fa 01]
void XcpTl_FeedReceiver(uint8_t octet)
{
    XcpTl_Receiver.Buffer[XcpTl_Receiver.Index] = octet;

    if (XcpTl_Receiver.State == XCP_RCV_IDLE) {
        XcpTl_Receiver.State = XCP_RCV_UNTIL_LENGTH;
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x01) {
            XcpTl_Receiver.Dlc = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x00);
            XcpTl_Receiver.State = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc + 2;
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        if (XcpTl_Receiver.Index == 0x03) {
            XcpTl_Receiver.Ctr = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x02);
        }
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {

            XcpTl_Receiver.State = XCP_RCV_IDLE;
            XcpTl_Receiver.Index = 0u;

            // TODO: ACTION!!!
            Xcp_PduIn.len = XcpTl_Receiver.Dlc;
            Xcp_PduIn.data = XcpTl_Receiver.Buffer + 4;
            Xcp_DispatchCommand(&Xcp_PduIn);
        }
    }
    if (XcpTl_Receiver.State != XCP_RCV_IDLE) {
        XcpTl_Receiver.Index++;
    }
}

int XcpTl_Send(uint8_t const * buf, uint16_t len)
{
    Serial.write(buf, len);
}

void XcpTl_SaveConnection(void)
{

}

void XcpTl_ReleaseConnection(void)
{

}

#if 0
void ping(void)
{
    Serial.write((uint8_t)0x08);
    Serial.write((uint8_t)0x00);
    Serial.write((uint8_t)0x00);
    Serial.write((uint8_t)0x00);
    Serial.print("SEND_XY");
}
#endif
