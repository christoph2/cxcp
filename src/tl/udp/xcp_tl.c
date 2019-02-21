/*
 * BlueParrot XCP
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

#define XCP_COMM_BUFLEN  (XCP_MAX(XCP_MAX_CTO, XCP_MAX_DTO))
#define XCP_COMM_PORT    (5555)

#define DEFAULT_FAMILY     PF_UNSPEC // Accept either IPv4 or IPv6
#define DEFAULT_SOCKTYPE   SOCK_STREAM // TCP
#define DEFAULT_PORT       "5001" // Arbitrary, albiet a historical test port


typedef struct tagXcpTl_ConnectionType {
    struct sockaddr_in connectionAddress;
    struct sockaddr_in currentAddress;
    bool connected;
 } XcpTl_ConnectionType;


static SOCKET sock = INVALID_SOCKET;
struct sockaddr_in server = {0};
unsigned char buf[XCP_COMM_BUFLEN];
int addrSize = sizeof(struct sockaddr_in);


static XcpTl_ConnectionType XcpTl_Connection;

static uint8_t Xcp_PduOutBuffer[XCP_MAX_CTO] = {0};

///
void Xcp_DispatchCommand(Xcp_PDUType const * const pdu);
///

extern Xcp_PDUType Xcp_PduIn;
extern Xcp_PDUType Xcp_PduOut;

static boolean Xcp_EnableSocketOption(SOCKET sock, int option);
static boolean Xcp_DisableSocketOption(SOCKET sock, int option);


void Win_ErrorMsg(char * const fun, DWORD errorCode)
{
    //LPWSTR buffer = NULL;
    char * buffer = XCP_NULL;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, XCP_NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &buffer, 0, XCP_NULL);
    if (buffer != XCP_NULL) {
        fprintf(stderr, "[%s] failed with: [%d] %s", fun, errorCode, buffer);
        LocalFree((HLOCAL)buffer);
    } else {
        //DBG_PRINT("FormatMessage failed!\n");
    }
}

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
    int SocketType = SOCK_DGRAM;
    char *Port = DEFAULT_PORT;
    int ret;
    DWORD dwTimeAdjustment = 0, dwTimeIncrement = 0, dwClockTick;
    BOOL fAdjustmentDisabled = XCP_TRUE;

    GetSystemTimeAdjustment(&dwTimeAdjustment, &dwTimeIncrement, &fAdjustmentDisabled);
    DBG_PRINT4("%ld %ld %ld\n", dwTimeAdjustment, dwTimeIncrement, fAdjustmentDisabled);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        Win_ErrorMsg("XcpTl_Init:WSAStartup()", WSAGetLastError());
        exit(EXIT_FAILURE);
    } else {

    }

///////////////////////////////
    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = Family;
    Hints.ai_socktype = SocketType;
    Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    ret = getaddrinfo(Address, Port, &Hints, &AddrInfo);
    if (ret != 0) {
        Win_ErrorMsg("XcpTl_Init::getaddrinfo", WSAGetLastError());
        WSACleanup();
        return -1;
    }


    freeaddrinfo(AddrInfo);
///////////////////////////////

    sock = socket(AF_INET, SOCK_DGRAM, 0);  // AF_INET6
    if (sock == INVALID_SOCKET) {
        Win_ErrorMsg("XcpTl_Init:socket()", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    } else {
        //DBG_PRINT1("UDP Socket created!\n");
    }

    // IN6ADDR_SETV4MAPPED

    if (!Xcp_EnableSocketOption(sock, SO_REUSEADDR)) {
        Win_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEADDR)", WSAGetLastError());
    }

    //Xcp_DisableSocketOption(sock, IPV6_V6ONLY);
    //getaddrinfo();


#ifdef SO_REUSEPORT
    if (!Xcp_EnableSocketOption(sock, SO_REUSEPORT)) {
        Win_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEPORT)", WSAGetLastError());
    }
#endif

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(XCP_COMM_PORT);

    if(bind(sock ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR) {
        Win_ErrorMsg("XcpTl_Init:bind()", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    getsockname(sock, (SOCKADDR *)&server, (int *)sizeof(server));
    DBG_PRINT3("UDP-Server bound to %s:%d\n", inet_ntoa(server.sin_addr), htons(server.sin_port));

    Xcp_PduOut.data = &Xcp_PduOutBuffer[0];
    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
}

void XcpTl_DeInit(void)
{
    closesocket(sock);
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

    memset(buf,'\0', XCP_COMM_BUFLEN);

    recv_len = recvfrom(sock, buf, XCP_COMM_BUFLEN, 0, (struct sockaddr *)&XcpTl_Connection.currentAddress, &addrSize);
    if (recv_len == SOCKET_ERROR)
    {
        Win_ErrorMsg("XcpTl_RxHandler:recvfrom()", WSAGetLastError());
        fflush(stdout);
    } else if (recv_len > 0) {
        // TODO: Big-Endian!!!
        dlc = (uint16_t)*(buf + 0);

//        DBG_PRINT4("Received packet from %s:%d [%d]\n", inet_ntoa(XcpTl_Connection.currentAddress.sin_addr), ntohs(XcpTl_Connection.currentAddress.sin_port), recv_len);
//        hexdump(buf, recv_len);

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

    timeout.tv_sec = sec;
    timeout.tv_usec = usec;

    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    // Return value:
    // -1: error occurred
    // 0: timed out
    // > 0: data ready to be read
    return select(0, &fds, 0, 0, &timeout);
}

void XcpTl_Send(uint8_t const * buf, uint16_t len)
{
#if 0
    printf("LEN: %u\t\t", len);
    Xcp_Hexdump(buf, len);
#endif // 0
    if (sendto(sock, (char const *)buf, len, 0, (struct sockaddr*)&XcpTl_Connection.connectionAddress, addrSize) == SOCKET_ERROR) {
        Win_ErrorMsg("XcpTl_Send:sendto()", WSAGetLastError());
    }
}


void XcpTl_SaveConnection(void)
{
    CopyMemory(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_in));
    XcpTl_Connection.connected = XCP_TRUE;

    //DBG_PRINT3("CONN %s:%d\n", inet_ntoa(XcpTl_Connection.connectionAddress.sin_addr), ntohs(XcpTl_Connection.connectionAddress.sin_port));
    //DBG_PRINT3("CURR %s:%d\n", inet_ntoa(XcpTl_Connection.currentAddress.sin_addr), ntohs(XcpTl_Connection.currentAddress.sin_port));
}

void XcpTl_ReleaseConnection(void)
{
    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
}


bool XcpTl_VerifyConnection(void)
{
    return memcmp(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_in)) == 0;
}

