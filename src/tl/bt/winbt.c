/*
 * BlueParrot XCP
 *
 * (C) 2021-2025 by Christoph Schueler <github.com/Christoph2,
 *                                 cpu12.gems@googlemail.com>
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
#include "xcp.h"
#include "xcp_eth.h"
#include "xcp_hw.h"
/*!!! END-INCLUDE-SECTION !!!*/

#include <BluetoothAPIs.h>
#include <Winsock2.h>
#include <Ws2bth.h>
#include <initguid.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

DEFINE_GUID(ASAM_XCP_ON_BT_GUID, 0xDC3CCEDC, 0xF7DE, 0x56E8, 0x87, 0x6F, 0x78, 0x1C, 0x07, 0x3C, 0x9B, 0x77);

extern XcpTl_ConnectionType XcpTl_Connection;
SOCKADDR_BTH                XcpTl_LocalBtAddress;

static void PrintBthAddr(SOCKADDR_BTH *addr);

void XcpTl_Init(void) {
    WSADATA     wsa;
    GUID        Xcp_ServiceID = ASAM_XCP_ON_BT_GUID;
    WSAQUERYSET service;
    CSADDR_INFO csAddr;
    bool        winsock_ok = false;

    ZeroMemory(&service, sizeof(service));
    ZeroMemory(&csAddr, sizeof(csAddr));
    ZeroMemory(&XcpTl_LocalBtAddress, sizeof(XcpTl_LocalBtAddress));
    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));

    XcpTl_Connection.socketType = SOCK_STREAM;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        XcpHw_ErrorMsg("XcpTl_Init:WSAStartup()", WSAGetLastError());
        /* Kein exit() hier: aufruferseitiges Fehlerhandling erlauben. */
        return;
    } else {
        winsock_ok                   = true;
        XcpTl_Connection.boundSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
        if (XcpTl_Connection.boundSocket == INVALID_SOCKET) {
            XcpHw_ErrorMsg("XcpTl_Init::socket()", WSAGetLastError());
            if (winsock_ok) {
                WSACleanup();
            }
            return;
        }
    }

    XcpTl_LocalBtAddress.addressFamily  = AF_BTH;
    XcpTl_LocalBtAddress.btAddr         = 0;
    XcpTl_LocalBtAddress.serviceClassId = GUID_NULL;
    XcpTl_LocalBtAddress.port           = BT_PORT_ANY;

    if (bind(XcpTl_Connection.boundSocket, (struct sockaddr *)&XcpTl_LocalBtAddress, sizeof(SOCKADDR_BTH)) == SOCKET_ERROR) {
        XcpHw_ErrorMsg("XcpTl_Init::bind()", WSAGetLastError());
        return;
        /* } else {
            int length = sizeof(SOCKADDR_BTH);

            getsockname(XcpTl_Connection.boundSocket, (struct sockaddr *)&XcpTl_LocalBtAddress, &length);
            printf("Local Bluetooth device is: [NAP: %04x SAP: %08x] Server channel = %04x\n", GET_NAP(XcpTl_LocalBtAddress.btAddr),
           GET_SAP(XcpTl_LocalBtAddress.btAddr), XcpTl_LocalBtAddress.port); printf("MAC: 0x%012x Port: %x\n",
           XcpTl_LocalBtAddress.btAddr, XcpTl_LocalBtAddress.port);
        */
    }

    int size = sizeof(SOCKADDR_BTH);

    if (getsockname(XcpTl_Connection.boundSocket, (struct sockaddr *)&XcpTl_LocalBtAddress, &size) == SOCKET_ERROR) {
        XcpHw_ErrorMsg("XcpTl_Init::getsockname()", WSAGetLastError());
        closesocket(XcpTl_Connection.boundSocket);
        XcpTl_Connection.boundSocket = INVALID_SOCKET;
        if (winsock_ok) {
            WSACleanup();
        }
        return;
    }

    if (listen(XcpTl_Connection.boundSocket, 1) == SOCKET_ERROR) {
        XcpHw_ErrorMsg("XcpTl_Init::listen()", WSAGetLastError());
        closesocket(XcpTl_Connection.boundSocket);
        XcpTl_Connection.boundSocket = INVALID_SOCKET;
        if (winsock_ok) {
            WSACleanup();
        }
        return;
    }

    service.dwSize                   = sizeof(service);
    service.lpszServiceInstanceName  = "XCPonBth";
    service.lpszComment              = "Measure and calibrate with XCP on Bluetooth -- Have fun :-)";
    service.lpServiceClassId         = &Xcp_ServiceID;
    service.dwNumberOfCsAddrs        = 1;
    service.dwNameSpace              = NS_BTH;
    csAddr.LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
    csAddr.LocalAddr.lpSockaddr      = (struct sockaddr *)&XcpTl_LocalBtAddress;
    csAddr.iSocketType               = SOCK_STREAM;
    csAddr.iProtocol                 = BTHPROTO_RFCOMM;
    service.lpcsaBuffer              = &csAddr;

    if (WSASetService(&service, RNRSERVICE_REGISTER, 0) == SOCKET_ERROR) {
        XcpHw_ErrorMsg("XcpTl_Init::WSASetService()", WSAGetLastError());
        /* Kein Abbruch: BT-Dienst nicht registriert, jedoch weiter lauschend. */
    }
}

void XcpTl_DeInit(void) {
    /* Optional: Dienst deregistrieren; hier minimal closing. */
    if (XcpTl_Connection.connectedSocket != INVALID_SOCKET && XcpTl_Connection.connectedSocket != 0) {
        closesocket(XcpTl_Connection.connectedSocket);
        XcpTl_Connection.connectedSocket = INVALID_SOCKET;
    }
    if (XcpTl_Connection.boundSocket != INVALID_SOCKET && XcpTl_Connection.boundSocket != 0) {
        closesocket(XcpTl_Connection.boundSocket);
        XcpTl_Connection.boundSocket = INVALID_SOCKET;
    }
    WSACleanup();
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    if (buf == NULL || len == 0) {
        return;
    }
    if (len > XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE) {
        XcpHw_ErrorMsg("XcpTl_Send: length exceeds TL buffer", EINVAL);
        return;
    }
    if (XcpTl_Connection.connectedSocket == INVALID_SOCKET || XcpTl_Connection.connectedSocket == 0) {
        XcpHw_ErrorMsg("XcpTl_Send: no connected socket", ENOTCONN);
        return;
    }

    XCP_TL_ENTER_CRITICAL();

    int sent = send(XcpTl_Connection.connectedSocket, (char const *)buf, len, 0);
    if (sent == SOCKET_ERROR) {
        XcpHw_ErrorMsg("XcpTl_Send:send()", WSAGetLastError());
        /* Verbindung nicht hart schließen – Caller entscheidet über Reconnect. */
    } else if (sent != (int)len) {
        XcpHw_ErrorMsg("XcpTl_Send: short send", EIO);
    }

    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_PrintBtDetails(void) {
    printf("XCPonBth -- Listening on ");
    PrintBthAddr(&XcpTl_LocalBtAddress);
    printf(" channel %u\n\r", (unsigned int)XcpTl_LocalBtAddress.port);
}

static void PrintBthAddr(SOCKADDR_BTH *addr) {
    uint8_t mac[6] = { 0 };
    uint8_t idx    = 0;

    mac[0] = (addr->btAddr >> 40) & 0xff;
    mac[1] = (addr->btAddr >> 32) & 0xff;
    mac[2] = (addr->btAddr >> 24) & 0xff;
    mac[3] = (addr->btAddr >> 16) & 0xff;
    mac[4] = (addr->btAddr >> 8) & 0xff;
    mac[5] = (uint8_t)((addr->btAddr) & 0xff);

    for (idx = 0; idx < 6; ++idx) {
        printf("%02X", mac[idx]);
        if (idx < 5) {
            printf(":");
        }
    }
}
