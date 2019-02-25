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

#if defined(ARDUINO)
#include "Arduino.h"
#endif

#include "xcp.h"
#include "xcp_util.h"


#define XCP_SXI_MAKEWORD(buf, offs)  (*(buf+offs)) | (( *(buf+offs+1) << 8))


void Xcp_DispatchCommand(Xcp_PDUType const * const pdu);

void XcpTl_SignalTimeout(void);

static void XcpTl_ResetSM(void);

extern Xcp_PDUType Xcp_PduIn;
extern Xcp_PDUType Xcp_PduOut;

static uint8_t Xcp_PduOutBuffer[XCP_COMM_BUFLEN] = {0};

typedef enum tagXcpTl_ReceiverStateType {
    XCP_RCV_IDLE,
    XCP_RCV_UNTIL_LENGTH,
    XCP_RCV_REMAINING,
} XcpTl_ReceiverStateType;


typedef struct tagXcpTl_ReceiverType {
    uint8_t Buffer[XCP_COMM_BUFLEN];
    XcpTl_ReceiverStateType State;
    uint16_t Index;
    uint16_t Remaining;
    uint16_t Dlc;   // TODO: config!!!
    uint16_t Ctr;   // TODO: config!!!
} XcpTl_ReceiverType;


static XcpTl_ReceiverType XcpTl_Receiver;


void XcpTl_Init(void)
{
    XcpTl_ResetSM();

    Xcp_PduOut.data = &Xcp_PduOutBuffer[0];
}

/********************************************//**
 * \brief Initialize, i.e. reset receiver state
 *        Machine
 *
 * \param void
 * \return void
 *
 ***********************************************/
static void XcpTl_ResetSM(void)
{
    XcpTl_Receiver.State = XCP_RCV_IDLE;
    XcpTl_Receiver.Index = 0u;
    XcpTl_Receiver.Dlc = 0u;
    XcpTl_Receiver.Ctr = 0u;
    XcpTl_Receiver.Remaining = 0u;
}


void XcpTl_RxHandler(void)
{

}


void XcpTl_SignalTimeout(void)
{
    XCP_TL_ENTER_CRITICAL();
    XcpTl_ResetSM();
    XCP_TL_LEAVE_CRITICAL();
}


void XcpTl_FeedReceiver(uint8_t octet)
{
    XCP_TL_ENTER_CRITICAL();

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

            XcpTl_ResetSM();

            Xcp_PduIn.len = XcpTl_Receiver.Dlc;
            Xcp_PduIn.data = XcpTl_Receiver.Buffer + 4;
            Xcp_DispatchCommand(&Xcp_PduIn);
        }
    }
    if (XcpTl_Receiver.State != XCP_RCV_IDLE) {
        XcpTl_Receiver.Index++;
    }
    XCP_TL_LEAVE_CRITICAL();
}

#include <stdio.h>

void XcpTl_Send(uint8_t const * buf, uint16_t len)
{
#if defined(ARDUINO)
    Serial.write(buf, len);
#endif
    uint8_t ch[6];
    uint16_t idx;

    for (idx = 0; idx < len; ++idx) {
        Xcp_Itoa((uint32_t)buf[idx], 16, (uint8_t*)ch);
        fputs(ch, stdout);
        fputs(" ", stdout);
    }
    fputs("\n", stdout);
}

void XcpTl_SaveConnection(void)
{

}

void XcpTl_ReleaseConnection(void)
{

}

