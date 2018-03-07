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

#if !defined(__XCP_TL_H)
#define __XCP_TL_H


#include <stdint.h>
#include <stdbool.h>

#define XCP_MAX_CTO (0xff)
#define XCP_MAX_DTO (512)

#define XCP_TRANSPORT_LAYER_VERSION_MAJOR       (1)
#define XCP_TRANSPORT_LAYER_VERSION_RELEASE     (0)

typedef struct tagXcp_PDUType {
    uint16_t len;
    //uint8_t data[XCP_MAX_CTO - 1];
    uint8_t * data;
} Xcp_PDUType;

void XcpTl_Init(void);
void XcpTl_DeInit(void);
int XcpTl_FrameAvailable(long sec, long usec);
void XcpTl_RxHandler(void);

void XcpTl_Task(void);

void Xcp_SendPdu(void);

uint8_t * Xcp_GetOutPduPtr(void);
void Xcp_SetPduOutLen(uint16_t len);
void Xcp_Send8(uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7);

int XcpTl_Send(uint8_t const * buf, uint16_t len);

void XcpTl_SaveConnection(void);
void XcpTl_ReleaseConnection(void);
bool XcpTl_VerifyConnection(void);

void XcpTl_FeedReceiver(uint8_t octet);

void XcpTl_TransportLayerCmd_Res(Xcp_PDUType const * const pdu);


#endif // __XCP_TL_H
