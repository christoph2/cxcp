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

//#define WIN32_LEAN_AND_MEAN

//#define _WIN32_WINNT 0x0600

#include <winsock2.h>
#include <Ws2tcpip.h>
//#include <windows.h>

#if 0
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#ifndef IPPROTO_IPV6
#include <tpipv6.h> // For IPv6 Tech Preview.
#endif
#endif

#include <Mstcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

// Needed for the Windows 2000 IPv6 Tech Preview.
#if (_WIN32_WINNT == 0x0500)
//#include <tpipv6.h>
#endif

#include "xcp.h"

#pragma comment(lib,"ws2_32.lib") // MSVC


#if !defined(XCP_MAX)
#define XCP_MAX(a, b)   (((a) > (b)) ? (a) : (b))
#endif

#define XCP_COMM_PORT    (5555)

#define DEFAULT_FAMILY     PF_UNSPEC // Accept either IPv4 or IPv6
#define DEFAULT_SOCKTYPE   SOCK_STREAM //
#define DEFAULT_PORT       "5555"


typedef struct tagXcpTl_ConnectionType {
    /*struct sockaddr_in*/ SOCKADDR_STORAGE connectionAddress;
    /*struct sockaddr_in*/ SOCKADDR_STORAGE currentAddress;
    SOCKET boundSocket;
    SOCKET connectedSocket;
    bool connected;
 } XcpTl_ConnectionType;


int SocketType = SOCK_STREAM;  /* SOCK_DGRAM */

unsigned char buf[XCP_COMM_BUFLEN];
int addrSize = sizeof(SOCKADDR_STORAGE);


static XcpTl_ConnectionType XcpTl_Connection;

static uint8_t Xcp_PduOutBuffer[XCP_MAX_CTO] = {0};

///
void Xcp_DispatchCommand(Xcp_PDUType const * const pdu);
///

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
        printf("Listening [%u] on port %s, protocol %s, protocol family %s\n",
               idx, Port, (SocketType == SOCK_STREAM) ? "TCP" : "UDP",
               (AI->ai_family == PF_INET) ? "PF_INET" : "PF_INET6");
        boundSocketNum = idx;
        XcpTl_Connection.boundSocket = serverSockets[boundSocketNum];
        break;  /* NOTE: this is certainly not the best solution. */
    }
    freeaddrinfo(AddrInfo);
    if (boundSocketNum == -1) {
        fprintf(stderr, "Fatal error: unable to serve on any address.\n");
        WSACleanup();
        return;
    }
    numSockets = idx;
#if 0
    sock = socket(AF_INET, SOCK_DGRAM, 0);  // AF_INET6
    if (sock == INVALID_SOCKET) {
        Win_ErrorMsg("XcpTl_Init:socket()", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    } else {
        //DBG_PRINT1("UDP Socket created!\n");
    }
#endif // 0

    // IN6ADDR_SETV4MAPPED

    if (!Xcp_EnableSocketOption(XcpTl_Connection.boundSocket, SO_REUSEADDR)) {
        Win_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEADDR)", WSAGetLastError());
    }

    //Xcp_DisableSocketOption(sock, IPV6_V6ONLY);
    //getaddrinfo();


#ifdef SO_REUSEPORT
    if (!Xcp_EnableSocketOption(boundSocket, SO_REUSEPORT)) {
        Win_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEPORT)", WSAGetLastError());
    }
#endif

#if 0
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(XCP_COMM_PORT);

    if(bind(sock ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR) {
        Win_ErrorMsg("XcpTl_Init:bind()", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    getsockname(sock, (SOCKADDR *)&server, (int *)sizeof(server));
    DBG_PRINT3("UDP-Server bound to %s:%d\n", inet_ntoa(server.sin_addr), htons(server.sin_port));
#endif // 0
}

void XcpTl_DeInit(void)
{
    closesocket(XcpTl_Connection.boundSocket);
    WSACleanup();
}

void XcpTl_MainFunction(void)
{
    static uint32_t cnt = 0;

    if (XcpTl_FrameAvailable(0, 1000) > 0) {
        //printf("\tFrameAvailable!!!\n");
        XcpTl_RxHandler();
    }
    cnt++;
    if ((cnt % 1000) == 0) {
        printf("Bound socket: %d\n", XcpTl_Connection.boundSocket);
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
            printf("\nAccepted connection from %s\n", Hostname);
        }
        //////////////////
        recv_len = recv(XcpTl_Connection.connectedSocket, (char*)buf, XCP_COMM_BUFLEN, 0);
        if (recv_len == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_RxHandler::recv()", WSAGetLastError());
            closesocket(XcpTl_Connection.connectedSocket);
            return;
        }
        if (recv_len == 0) {
            printf("Client closed connection\n");
            closesocket(XcpTl_Connection.connectedSocket);
            return;
        }
        //printf("Received %d bytes from client: ", recv_len);
        //Xcp_Hexdump(buf, recv_len);
        //////////////////
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
            fprintf(stdout, "Error: frame to short: %d\n", recv_len);
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

    if (SocketType == SOCK_STREAM) {
        return 1;
    } else if (SocketType == SOCK_DGRAM) {
        res = select(0, &fds, 0, 0, &timeout);
        if (res == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_FrameAvailable:select()", WSAGetLastError());
        }
        return res;
    }
}

void XcpTl_Send(uint8_t const * buf, uint16_t len)
{
    //printf("Sending... ");
    //Xcp_Hexdump(buf, len);

    if (SocketType == SOCK_DGRAM) {
        printf(" DGRAM\n");
        if (sendto(XcpTl_Connection.boundSocket, (char const *)buf, len, 0,
            (struct sockaddr*)&XcpTl_Connection.connectionAddress, addrSize) == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_Send:sendto()", WSAGetLastError());
        }
    } else if (SocketType == SOCK_STREAM) {
        //printf(" STREAM\n");
        if (send(XcpTl_Connection.connectedSocket, (char const *)buf, len, 0) == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_Send:send()", WSAGetLastError());
            closesocket(XcpTl_Connection.connectedSocket);
        }
        //printf("After send.\n");
    }
}


void XcpTl_SaveConnection(void)
{
    CopyMemory(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_in));
    XcpTl_Connection.connected = XCP_TRUE;
}

void XcpTl_ReleaseConnection(void)
{
    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
}


bool XcpTl_VerifyConnection(void)
{
    return memcmp(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_in)) == 0;
}
