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


#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


#include "xcp.h"

#pragma comment(lib,"ws2_32.lib") // MSVC only.


#define XCP_COMM_PORT    (5555)

#define DEFAULT_FAMILY     PF_UNSPEC // Accept either IPv4 or IPv6
#define DEFAULT_SOCKTYPE   SOCK_STREAM //
#define DEFAULT_PORT       "5555"


typedef struct tagXcpTl_ConnectionType {
    SOCKADDR_STORAGE connectionAddress;
    SOCKADDR_STORAGE currentAddress;
    SOCKET boundSocket;
    SOCKET connectedSocket;
    bool connected;
 } XcpTl_ConnectionType;


int SocketType = SOCK_STREAM;  /* SOCK_DGRAM */

unsigned char buf[XCP_COMM_BUFLEN];
int addrSize = sizeof(SOCKADDR_STORAGE);


static XcpTl_ConnectionType XcpTl_Connection;

static uint8_t Xcp_PduOutBuffer[XCP_MAX_CTO] = {0};


void Xcp_DispatchCommand(Xcp_PDUType const * const pdu);

extern Xcp_PDUType Xcp_PduIn;
extern Xcp_PDUType Xcp_PduOut;

static boolean Xcp_EnableSocketOption(SOCKET sock, int option);
static boolean Xcp_DisableSocketOption(SOCKET sock, int option);


static  boolean Xcp_EnableSocketOption(SOCKET sock, int option)
{
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const char enable = 1;

    if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
        return XCP_FALSE;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){1}, sizeof(int)) < 0) {
        return XCP_FALSE;
    }
#endif
    return XCP_TRUE;
}

static boolean Xcp_DisableSocketOption(SOCKET sock, int option)
{
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const char enable = 0;

    if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
        return XCP_FALSE;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){0}, sizeof(int)) < 0) {
        return XCP_FALSE;
    }
#endif
    return XCP_TRUE;
}


void XcpTl_Init(void)
{
    WSADATA wsa;
    ADDRINFO Hints, *AddrInfo, *AI;
    char *Address = NULL;
    int Family = PF_INET;
    char *Port = DEFAULT_PORT;
    SOCKET serverSockets[FD_SETSIZE];
    int boundSocketNum = -1;
    int numSockets;
    int ret;
    int idx;
    DWORD dwTimeAdjustment = 0UL, dwTimeIncrement = 0UL;
    BOOL fAdjustmentDisabled = XCP_TRUE;

    GetSystemTimeAdjustment(&dwTimeAdjustment, &dwTimeIncrement, &fAdjustmentDisabled);
    DBG_PRINT4("%ld %ld %ld\n", dwTimeAdjustment, dwTimeIncrement, fAdjustmentDisabled);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        Win_ErrorMsg("XcpTl_Init:WSAStartup()", WSAGetLastError());
        exit(EXIT_FAILURE);
    } else {

    }
    Xcp_PduOut.data = &Xcp_PduOutBuffer[0];
    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = Family;
    Hints.ai_socktype = SocketType;
    Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    ret = getaddrinfo(Address, Port, &Hints, &AddrInfo);
    if (ret != 0) {
        Win_ErrorMsg("XcpTl_Init::getaddrinfo()", WSAGetLastError());
        WSACleanup();
        return;
    }
    for (idx = 0, AI = AddrInfo; AI != NULL; AI = AI->ai_next, ++idx) {
        if (idx == FD_SETSIZE) {
            printf("getaddrinfo returned more addresses than we could use.\n");
            break;
        }
        if ((AI->ai_family != PF_INET) && (AI->ai_family != PF_INET6)) {
            continue;
        }
        serverSockets[idx] = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);
        if (serverSockets[idx] == INVALID_SOCKET){
            Win_ErrorMsg("XcpTl_Init::socket()", WSAGetLastError());
            continue;
        }
        if (bind(serverSockets[idx], AI->ai_addr, AI->ai_addrlen) == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_Init::bind()", WSAGetLastError());
            continue;
        }
        if (SocketType == SOCK_STREAM) {
            if (listen(serverSockets[idx], 1) == SOCKET_ERROR) {
                Win_ErrorMsg("XcpTl_Init::listen()", WSAGetLastError());
                continue;
            }
        }
        DBG_PRINT5("Listening [%u] on port %s, protocol %s, protocol family %s\n",
               idx, Port, (SocketType == SOCK_STREAM) ? "TCP" : "UDP",
               (AI->ai_family == PF_INET) ? "PF_INET" : "PF_INET6");
        boundSocketNum = idx;
        XcpTl_Connection.boundSocket = serverSockets[boundSocketNum];
        break;  /* Grab first address. */
    }
    freeaddrinfo(AddrInfo);
    if (boundSocketNum == -1) {
        fprintf(stderr, "Fatal error: unable to serve on any address.\n");
        WSACleanup();
        return;
    }
    numSockets = idx;
    if (!Xcp_EnableSocketOption(XcpTl_Connection.boundSocket, SO_REUSEADDR)) {
        Win_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEADDR)", WSAGetLastError());
    }
#ifdef SO_REUSEPORT
    if (!Xcp_EnableSocketOption(boundSocket, SO_REUSEPORT)) {
        Win_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEPORT)", WSAGetLastError());
    }
#endif
}

void XcpTl_DeInit(void)
{
    closesocket(XcpTl_Connection.boundSocket);
    WSACleanup();
}

void XcpTl_MainFunction(void)
{
    if (XcpTl_FrameAvailable(0, 1000) > 0) {
        XcpTl_RxHandler();
    }
}

void XcpTl_RxHandler(void)
{
    int recv_len;
    uint16_t dlc;
    int FromLen;
    SOCKADDR_STORAGE From;
    char Hostname[NI_MAXHOST];

    memset(buf,'\0', XCP_COMM_BUFLEN);

    if (SocketType == SOCK_STREAM) {
        if (!XcpTl_Connection.connected) {
            FromLen = sizeof(From);
            XcpTl_Connection.connectedSocket = accept(XcpTl_Connection.boundSocket, (LPSOCKADDR)&XcpTl_Connection.currentAddress, &FromLen);
            if (XcpTl_Connection.connectedSocket == INVALID_SOCKET) {
                Win_ErrorMsg("XcpTl_RxHandler::accept()", WSAGetLastError());
                //WSACleanup();
                exit(1);
                return;
            }
            if (getnameinfo((LPSOCKADDR)&From, FromLen, Hostname, sizeof(Hostname), NULL, 0, NI_NUMERICHOST) != 0) {
                strcpy(Hostname, "<unknown>");
            }
            DBG_PRINT2("\nAccepted connection from %s\n", Hostname);
        }
        recv_len = recv(XcpTl_Connection.connectedSocket, (char*)buf, XCP_COMM_BUFLEN, 0);
        if (recv_len == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_RxHandler::recv()", WSAGetLastError());
            closesocket(XcpTl_Connection.connectedSocket);
            exit(1);
            return;
        }
        if (recv_len == 0) {
            DBG_PRINT1("Client closed connection\n");
            closesocket(XcpTl_Connection.connectedSocket);
            XcpTl_Connection.connected = XCP_FALSE;
            return;
        }
    } else {
        recv_len = recvfrom(XcpTl_Connection.boundSocket, (char*)buf, XCP_COMM_BUFLEN, 0,
            (LPSOCKADDR)&XcpTl_Connection.currentAddress, &addrSize
        );
        if (recv_len == SOCKET_ERROR)
        {
            Win_ErrorMsg("XcpTl_RxHandler:recvfrom()", WSAGetLastError());
            fflush(stdout);
        }
        printf("Received %d bytes from client: ", recv_len);
        Xcp_Hexdump(buf, recv_len);
    }
    if (recv_len > 0) {
        // TODO: Big-Endian!!!
        dlc = (uint16_t)*(buf + 0);
        if (!XcpTl_Connection.connected || (XcpTl_VerifyConnection())) {
            Xcp_PduIn.len = dlc;
            Xcp_PduIn.data = buf + 4;
            Xcp_DispatchCommand(&Xcp_PduIn);
        }
        if (recv_len < 5) {
            DBG_PRINT2("Error: frame to short: %d\n", recv_len);
        } else {

        }
        fflush(stdout);
    }
}


void XcpTl_TxHandler(void)
{

}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec)
{
    struct timeval timeout;
    fd_set fds;
    int16_t res;

    timeout.tv_sec = sec;
    timeout.tv_usec = usec;

    FD_ZERO(&fds);
    FD_SET(XcpTl_Connection.boundSocket, &fds);


    // Return value:
    // -1: error occurred
    // 0: timed out
    // > 0: data ready to be read

    if (((SocketType == SOCK_STREAM) && (!XcpTl_Connection.connected)) || (SocketType == SOCK_DGRAM)) {
        res = select(0, &fds, 0, 0, &timeout);
        if (res == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_FrameAvailable:select()", WSAGetLastError());
        }
        return res;
    } else {
        return 1;
    }
}

void XcpTl_Send(uint8_t const * buf, uint16_t len)
{
    if (SocketType == SOCK_DGRAM) {
        if (sendto(XcpTl_Connection.boundSocket, (char const *)buf, len, 0,
            (struct sockaddr*)&XcpTl_Connection.connectionAddress, addrSize) == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_Send:sendto()", WSAGetLastError());
        }
    } else if (SocketType == SOCK_STREAM) {
        if (send(XcpTl_Connection.connectedSocket, (char const *)buf, len, 0) == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_Send:send()", WSAGetLastError());
            closesocket(XcpTl_Connection.connectedSocket);
        }
    }
}


void XcpTl_SaveConnection(void)
{
    CopyMemory(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_in));
    XcpTl_Connection.connected = XCP_TRUE;
}

void XcpTl_ReleaseConnection(void)
{
    DBG_PRINT1("XcpTl_ReleaseConnection()\n");
    XcpTl_Connection.connected = XCP_FALSE;
}


bool XcpTl_VerifyConnection(void)
{
    return memcmp(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_in)) == 0;
}
