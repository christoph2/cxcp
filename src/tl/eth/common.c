/*
 * BlueParrot XCP
 *
 * (C) 2021 by Christoph Schueler <github.com/Christoph2,
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


#include "xcp.h"
#include "xcp_eth.h"

#if defined(__unix__)
    #define SOCKET_ERROR        (-1)
    #define ZeroMemory(b,l )    memset((b), 0, (l))
#endif

static void XcpTl_Accept(void);
static int XcpTl_ReadHeader(uint16_t * len, uint16_t * counter);
static int XcpTl_ReadData(uint8_t * data, uint8_t len);

XcpTl_ConnectionType XcpTl_Connection;

static uint8_t XcpTl_RxBuffer[XCP_COMM_BUFLEN];


void * XcpTl_Thread(void * param)
{
    XCP_UNREFERENCED_PARAMETER(param);
    XCP_FOREVER {
        XcpTl_MainFunction();
    }
    return NULL;
}

void XcpTl_MainFunction(void)
{
    XcpTl_Accept();
    XcpTl_RxHandler();
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

static int XcpTl_ReadHeader(uint16_t * len, uint16_t * counter)
{
    uint8_t header_buffer[8];
    uint8_t bytes_remaining = XCP_ETH_HEADER_SIZE;
    uint8_t offset = 0;
    uint8_t nbytes = 0;

    XCP_FOREVER {
        nbytes = recv(XcpTl_Connection.connectedSocket, (char*)header_buffer + offset, bytes_remaining, 0);
        if (nbytes < 1) {
            return nbytes;
        }
        bytes_remaining -= nbytes;
        if (bytes_remaining == 0) {
            break;
        }
        offset += nbytes;
    }
    *len = XCP_MAKEWORD(header_buffer[offset + 1], header_buffer[offset + 0]);
    *counter = XCP_MAKEWORD(header_buffer[offset + 3], header_buffer[offset + 2]);
    return 1;
}

static int XcpTl_ReadData(uint8_t * data, uint8_t len)
{
    uint8_t bytes_remaining = len;
    uint8_t offset = 0;
    uint8_t nbytes = 0;

    XCP_FOREVER {
        nbytes = recv(XcpTl_Connection.connectedSocket, (char*)data + offset, bytes_remaining, 0);
        if (nbytes < 1) {
            return nbytes;
        }
        bytes_remaining -= nbytes;
        if (bytes_remaining == 0) {
            break;
        }
        offset += nbytes;
    }
}

void XcpTl_RxHandler(void)
{
    int res;
    uint16_t dlc = 0U;
    uint16_t counter = 0U;

    ZeroMemory(XcpTl_RxBuffer, XCP_COMM_BUFLEN);

    XCP_FOREVER {
        res = XcpTl_ReadHeader(&dlc, &counter);
        if (res == -1) {
#if defined(_WIN32)
            XcpHw_ErrorMsg("XcpTl_RxHandler:XcpTl_ReadHeader()", WSAGetLastError());
#elif defined(__unix__)
            XcpHw_ErrorMsg("XcpTl_RxHandler:XcpTl_ReadHeader()", errno);
#endif
            exit(2);
        } else if (res == 0) {
            return;
        }
        res = XcpTl_ReadData(&XcpTl_RxBuffer[0], dlc);
        if (res == -1) {
#if defined(_WIN32)
            XcpHw_ErrorMsg("XcpTl_RxHandler:XcpTl_ReadData()", WSAGetLastError());
#elif defined(__unix__)
            XcpHw_ErrorMsg("XcpTl_RxHandler:XcpTl_ReadData()", errno);
#endif
            exit(2);
        } else if (res == 0) {
            return;
        }
        XcpUtl_MemCopy(Xcp_CtoIn.data, XcpTl_RxBuffer, dlc);
        Xcp_DispatchCommand(&Xcp_CtoIn);
    }
}

static void XcpTl_Accept(void)
{
    socklen_t FromLen = 0;
    struct sockaddr_storage From;

    XcpUtl_ZeroMem(XcpTl_RxBuffer, XCP_COMM_BUFLEN);

    if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (!XcpTl_Connection.connected) {
            FromLen = sizeof(From);
            XcpTl_Connection.connectedSocket = accept(XcpTl_Connection.boundSocket, (struct sockaddr *)&XcpTl_Connection.currentAddress, &FromLen);
            if (XcpTl_Connection.connectedSocket == -1) {
                XcpHw_ErrorMsg("XcpTl_Accept::accept()", errno);
                exit(1);
            }
        } else {
        }
    }
}


void XcpTl_TxHandler(void)
{

}


void XcpTl_SaveConnection(void)
{
    XcpUtl_MemCopy(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_storage));
    XcpTl_Connection.connected = XCP_TRUE;
}


void XcpTl_ReleaseConnection(void)
{
    XcpTl_Connection.connected = XCP_FALSE;
}


bool XcpTl_VerifyConnection(void)
{
    return memcmp(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_storage)) == 0;
}

void XcpTl_PrintConnectionInformation(void)
{
    printf("XCPonEth -- Listening on port %u / %s [%s]\n\r",
        XCP_ETH_DEFAULT_PORT,
        Xcp_Options.tcp ? "TCP" : "UDP",
        Xcp_Options.ipv6 ? "IPv6" : "IPv4"
    );
}

