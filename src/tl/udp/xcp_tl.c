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

//#define WIN32_LEAN_AND_MEAN

//#define _WIN32_WINNT 0x0600

#include <winsock2.h>
#include <Ws2tcpip.h>
//#include <windows.h>
#include <Mstcpip.h>
#include <stdio.h>
#include <stdlib.h>

// Needed for the Windows 2000 IPv6 Tech Preview.
#if (_WIN32_WINNT == 0x0500)
//#include <tpipv6.h>
#endif

#include "xcp_tl.h"

#pragma comment(lib,"ws2_32.lib") // MSVC


#if !defined(MAX)
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#endif

#define XCP_COMM_BUFLEN  (MAX(XCP_MAX_CTO, XCP_MAX_DTO))
#define XCP_COMM_PORT    (5555)

typedef struct tagXcpTl_ConnectionType {
    struct sockaddr_in connectionAddress;
    struct sockaddr_in currentAddress;
    bool connected;
 } XcpTl_ConnectionType;


static SOCKET sock = INVALID_SOCKET;
struct sockaddr_in server = {0};
//struct sockaddr_in remote = {0};
unsigned char buf[XCP_COMM_BUFLEN];
int addrSize = sizeof(struct sockaddr_in);


/// TODO: HEADER!

void XcpTl_RxHandler(void);


static XcpTl_ConnectionType XcpTl_Connection;

static Xcp_PDUType Xcp_PduIn;
static Xcp_PDUType Xcp_PduOut;

static uint8_t Xcp_PduOutBuffer[XCP_MAX_CTO] = {0};

///
void Xcp_DispatchCommand(Xcp_PDUType const * const pdu);
///


static boolean Xcp_EnableSocketOption(SOCKET sock, int option);
static boolean Xcp_DisableSocketOption(SOCKET sock, int option);


void Win_ErrorMsg(char * const fun, DWORD errorCode)
{
    //LPWSTR buffer = NULL;
    char * buffer = NULL;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &buffer, 0, NULL);
    if (buffer != NULL) {
        fprintf(stderr, "[%s] failed with: [%d] %s", fun, errorCode, buffer);
        LocalFree((HLOCAL)buffer);
    } else {
        //printf("FormatMessage failed!\n");
    }
}

static  boolean Xcp_EnableSocketOption(SOCKET sock, int option)
{
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const int enable = 1;

    if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
        return FALSE;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){1}, sizeof(int)) < 0) {
        return FALSE;
    }
#endif
    return TRUE;
}

static boolean Xcp_DisableSocketOption(SOCKET sock, int option)
{
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const int enable = 0;

    if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
        return FALSE;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){0}, sizeof(int)) < 0) {
        return FALSE;
    }
#endif
    return TRUE;
}


void XcpTl_Init(void)
{
    WSADATA wsa;

    DWORD dwTimeAdjustment = 0, dwTimeIncrement = 0, dwClockTick;
    BOOL fAdjustmentDisabled = TRUE;

    GetSystemTimeAdjustment(&dwTimeAdjustment, &dwTimeIncrement, &fAdjustmentDisabled);
    printf("%ld %ld %ld\n", dwTimeAdjustment, dwTimeIncrement, fAdjustmentDisabled);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        Win_ErrorMsg("XcpTl_Init:WSAStartup()", WSAGetLastError());
        exit(EXIT_FAILURE);
    } else {
        //printf("Winsock started!\n");
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);  // AF_INET6
    if (sock == INVALID_SOCKET) {
        Win_ErrorMsg("XcpTl_Init:socket()", WSAGetLastError());
        exit(EXIT_FAILURE);
    } else {
        //printf("UDP Socket created!\n");
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
    printf("UDP-Server bound to %s:%d\n", inet_ntoa(server.sin_addr), htons(server.sin_port));

    Xcp_PduOut.data = &Xcp_PduOutBuffer[0];
    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
}

void XcpTl_DeInit(void)
{
    closesocket(sock);
    WSACleanup();
}

void XcpTl_Task(void)
{
    if (XcpTl_FrameAvailable(0, 1000) > 0) {
        XcpTl_RxHandler();
    }
}

void hexdump(unsigned char const * buf, int sz)
{
    for (int idx = 0; idx < sz; ++idx) {
        printf("%02X ", buf[idx]);
    }
    printf("\n");
}

void XcpTl_RxHandler(void)
{
    int recv_len;

    uint16_t dlc;
    uint16_t ctr;

    memset(buf,'\0', XCP_COMM_BUFLEN);

    recv_len = recvfrom(sock, buf, XCP_COMM_BUFLEN, 0, (struct sockaddr *)&XcpTl_Connection.currentAddress, &addrSize);
    if (recv_len == SOCKET_ERROR)
    {
        Win_ErrorMsg("XcpTl_RxHandler:recvfrom()", WSAGetLastError());
        fflush(stdout);
    } else if (recv_len > 0) {
        // TODO: Big-Ehdian!!!
        dlc = (uint16_t)*(buf + 0);
        ctr = (uint16_t)*(buf + 2);

        printf("Received packet from %s:%d [%d] FL: %d\n", inet_ntoa(XcpTl_Connection.currentAddress.sin_addr), ntohs(XcpTl_Connection.currentAddress.sin_port), recv_len, dlc);
        hexdump(buf, recv_len);

        if (!XcpTl_Connection.connected || (XcpTl_Connection.connected && XcpTl_VerifyConnection())) {
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

int XcpTl_FrameAvailable(long sec, long usec)
{
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = usec;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    // Return value:
    // -1: error occurred
    // 0: timed out
    // > 0: data ready to be read
    return select(0, &fds, 0, 0, &timeout);
}

int XcpTl_Send(uint8_t const * buf, uint16_t len)
{
    if (sendto(sock, (char const *)buf, len, 0, (struct sockaddr*)&XcpTl_Connection.connectionAddress, addrSize) == SOCKET_ERROR) {
        Win_ErrorMsg("XcpTl_Send:sendto()", WSAGetLastError());
        return 0;
    }
    return 1;
}

void XcpTl_SendPdu(void)
{
    uint16_t len = Xcp_PduOut.len;

    Xcp_PduOut.data[0] = LOBYTE(len);
    Xcp_PduOut.data[1] = HIBYTE(len);
    Xcp_PduOut.data[2] = 0;
    Xcp_PduOut.data[3] = 0;

    //printf("Sending PDU: ");
    //hexdump(Xcp_PduOut.data, Xcp_PduOut.len + 4);

    XcpTl_Send(Xcp_PduOut.data, Xcp_PduOut.len + 4);
}


// SendXcpPdu
void XcpTl_Send8(uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7)
{
    uint8_t * dataOut = XcpTl_GetOutPduPtr();

    XcpTl_SetPduOutLen(len);

    /* Controlled fall-through */
    switch(len) {
        case 8:
            dataOut[7] = b7;
        case 7:
            dataOut[6] = b6;
        case 6:
            dataOut[5] = b5;
        case 5:
            dataOut[4] = b4;
        case 4:
            dataOut[3] = b3;
        case 3:
            dataOut[2] = b2;
        case 2:
            dataOut[1] = b1;
        case 1:
            dataOut[0] = b0;
    }

    XcpTl_SendPdu();
}

#if 0
LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
LARGE_INTEGER Frequency;

QueryPerformanceFrequency(&Frequency);
QueryPerformanceCounter(&StartingTime);

// Activity to be timed

QueryPerformanceCounter(&EndingTime);
ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;


//
// We now have the elapsed number of ticks, along with the
// number of ticks-per-second. We use these values
// to convert to the number of elapsed microseconds.
// To guard against loss-of-precision, we convert
// to microseconds *before* dividing by ticks-per-second.
//

ElapsedMicroseconds.QuadPart *= 1000000;
ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
#endif // 0


uint8_t * XcpTl_GetOutPduPtr(void)
{
    return Xcp_PduOut.data + 4;
}

void XcpTl_SetPduOutLen(uint16_t len)
{
    Xcp_PduOut.len = len;
}

void XcpTl_SaveConnection(void)
{
    CopyMemory(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_in));
    XcpTl_Connection.connected = TRUE;

    //printf("CONN %s:%d\n", inet_ntoa(XcpTl_Connection.connectionAddress.sin_addr), ntohs(XcpTl_Connection.connectionAddress.sin_port));
    //printf("CURR %s:%d\n", inet_ntoa(XcpTl_Connection.currentAddress.sin_addr), ntohs(XcpTl_Connection.currentAddress.sin_port));
}

void XcpTl_ReleaseConnection(void)
{
    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
}


bool XcpTl_VerifyConnection(void)
{
    return memcmp(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_in)) == 0;
}

