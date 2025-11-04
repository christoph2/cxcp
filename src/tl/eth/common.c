/*
 * BlueParrot XCP
 *
 * (C) 2021-2025 by Christoph Schueler <github.com/Christoph2,
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

/*
**
** Socket routines independent of operating system and protocol.
**
*/

#include <stdio.h>
#include <stdlib.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_eth.h"
/*!!! END-INCLUDE-SECTION !!!*/

void XcpHw_ErrorMsg(char * const function, int errorCode);

void XcpThrd_EnableAsyncCancellation(void);
bool XcpThrd_IsShuttingDown(void);

void XcpTl_PrintBtDetails(void);

static void XcpTl_Accept(void);
static int  XcpTl_ReadHeader(uint16_t *len, uint16_t *counter);
static int  XcpTl_ReadData(uint8_t *data, uint16_t len);
static int  XcpTl_ReceiveFrom(void);

#if XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET
static char *Curl_inet_ntop(int af, const void *src, char *buf, size_t size);
#endif

XcpTl_ConnectionType XcpTl_Connection;

static uint8_t XcpTl_RxBuffer[XCP_COMM_BUFLEN];

void *XcpTl_Thread(void *param) {
    XCP_UNREFERENCED_PARAMETER(param);
    XcpThrd_EnableAsyncCancellation();
    XCP_FOREVER {
        XcpTl_MainFunction();
    }
    return NULL;
}

void XcpTl_MainFunction(void) {
    XcpTl_Accept();
    XcpTl_RxHandler();
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

static int XcpTl_ReadHeader(uint16_t *len, uint16_t *counter) {
    uint8_t   header_buffer[8];
    uint8_t   bytes_remaining = XCP_ETH_HEADER_SIZE;
    uint8_t   offset          = 0;
    int       nbytes          = 0;
    socklen_t from_len        = (socklen_t)sizeof(XcpTl_Connection.currentAddress);

    XCP_FOREVER {
        if (XcpTl_Connection.socketType == SOCK_DGRAM) {
            /* For UDP, the header is already in the buffer. */
            XcpUtl_MemCopy(header_buffer, XcpTl_RxBuffer, XCP_ETH_HEADER_SIZE);
            nbytes = XCP_ETH_HEADER_SIZE;
        } else if (XcpTl_Connection.socketType == SOCK_STREAM) {
            nbytes = recv(XcpTl_Connection.connectedSocket, (char *)header_buffer + offset, bytes_remaining, 0);
            if (XcpThrd_IsShuttingDown()) {
                return 0;
            }
        }
        if (nbytes <= 0) {
            return nbytes;
        }
        bytes_remaining -= (uint8_t)nbytes;
        if (bytes_remaining == 0) {
            break;
        }
        offset = (uint8_t)(offset + nbytes);
    }

    *len     = XCP_MAKEWORD(header_buffer[1], header_buffer[0]);
    *counter = XCP_MAKEWORD(header_buffer[3], header_buffer[2]);
    return 1;
}

static int XcpTl_ReadData(uint8_t *data, uint16_t len) {
    uint16_t  bytes_remaining = len;
    uint16_t  offset          = 0;
    int       nbytes          = 0;
    socklen_t from_len        = (socklen_t)sizeof(XcpTl_Connection.currentAddress);

    XCP_FOREVER {
        if (XcpTl_Connection.socketType == SOCK_DGRAM) {
            /* For UDP, the data is already in the buffer. */
            XcpUtl_MemCopy(data, XcpTl_RxBuffer + XCP_ETH_HEADER_SIZE, len);
            nbytes = len;
        } else if (XcpTl_Connection.socketType == SOCK_STREAM) {
            nbytes = recv(XcpTl_Connection.connectedSocket, (char *)data + offset, bytes_remaining, 0);
        }
        if (nbytes <= 0) {
            return nbytes;
        }
        bytes_remaining = (uint16_t)(bytes_remaining - (uint16_t)nbytes);
        if (bytes_remaining == 0) {
            break;
        }
        offset = (uint16_t)(offset + (uint16_t)nbytes);
    }
    return 1;
}

static int XcpTl_ReceiveFrom(void) {
    socklen_t from_len = (socklen_t)sizeof(XcpTl_Connection.currentAddress);
    int       nbytes   = recvfrom(
        XcpTl_Connection.boundSocket, (char *)XcpTl_RxBuffer, (int)XCP_COMM_BUFLEN, 0,
        (struct sockaddr *)&XcpTl_Connection.currentAddress, &from_len
    );
    if (XcpThrd_IsShuttingDown()) {
        return 0;
    }
    return nbytes;
}

void XcpTl_RxHandler(void) {
    int      res;
    uint16_t dlc     = 0U;
    uint16_t counter = 0U;

    /* Avoid unnecessary buffer clearing; we read directly into destination. */

    XCP_FOREVER {
        if (XcpTl_Connection.socketType == SOCK_DGRAM) {
            res = XcpTl_ReceiveFrom();
            if (res <= 0) {
                XcpTl_ReleaseConnection();
                return;
            }
        }

        res = XcpTl_ReadHeader(&dlc, &counter);
        if (res == -1) {
#if defined(_WIN32)
            XcpHw_ErrorMsg("XcpTl_RxHandler:XcpTl_ReadHeader()", WSAGetLastError());
#elif defined(__unix__) || defined(__APPLE__)
            XcpHw_ErrorMsg("XcpTl_RxHandler:XcpTl_ReadHeader()", errno);
#endif
            XcpTl_ReleaseConnection();
            return;
        } else if (res == 0) {
            XcpTl_ReleaseConnection();
            return;
        }

        /* DLC-Sanity-Check. */
        if (dlc > XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE || dlc > (uint16_t)XCP_COMM_BUFLEN) {
            XcpHw_ErrorMsg("XcpTl_RxHandler: DLC too large", EINVAL);
            XcpTl_ReleaseConnection();
            return;
        }

        /* For UDP: learn peer address on first packet so replies go back via sendto(). */
        if ((XcpTl_Connection.socketType == SOCK_DGRAM) && (!XcpTl_Connection.connected)) {
            XcpTl_SaveConnection();
        }

        if (!XcpThrd_IsShuttingDown()) {
            Xcp_CtoIn.len = dlc;
            /* Read payload directly into target buffer to avoid an extra copy. */
            res = XcpTl_ReadData(Xcp_CtoIn.data, dlc);
            if (res == -1) {
#if defined(_WIN32)
                XcpHw_ErrorMsg("XcpTl_RxHandler:XcpTl_ReadData()", WSAGetLastError());
#elif defined(__unix__) || defined(__APPLE__)
                XcpHw_ErrorMsg("XcpTl_RxHandler:XcpTl_ReadData()", errno);
#endif
                XcpTl_ReleaseConnection();
                return;
            } else if (res == 0) {
                /* Connection closed /no data. */
                XcpTl_ReleaseConnection();
                return;
            }
            XCP_ASSERT_LE(dlc, XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE);
            // XcpUtl_MemCopy(Xcp_CtoIn.data, XcpTl_RxBuffer, dlc);
            Xcp_DispatchCommand(&Xcp_CtoIn);
        } else {
            /* Shutting down */
            return;
        }
    }
}

static void XcpTl_Accept(void) {
    socklen_t               FromLen = 0;
    struct sockaddr_storage From;
    uint32_t                err;

    /* Avoid clearing Rx buffer on each accept iteration; no data is read into it here. */

    if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (!XcpTl_Connection.connected) {
            FromLen = (socklen_t)sizeof(From);
            XcpTl_Connection.connectedSocket =
                accept(XcpTl_Connection.boundSocket, (struct sockaddr *)&XcpTl_Connection.currentAddress, &FromLen);
            if (XcpTl_Connection.connectedSocket == INVALID_SOCKET) {
#if defined(_WIN32)
                err = WSAGetLastError();
                if (err != WSAEINTR) {
                    XcpHw_ErrorMsg("XcpTl_Accept::accept()", err);
                }
#elif defined(__unix__) || defined(__APPLE__)
                XcpHw_ErrorMsg("XcpTl_Accept::accept()", errno);
#endif
            } else {
                XcpTl_SaveConnection();
            }
        } else {
        }
    }
}

void XcpTl_TxHandler(void) {
}

void XcpTl_SaveConnection(void) {
    XcpUtl_MemCopy(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_storage));
    XcpTl_Connection.connected = XCP_TRUE;
}

void XcpTl_ReleaseConnection(void) {
    XcpTl_Connection.connected = XCP_FALSE;
}

bool XcpTl_VerifyConnection(void) {
    return memcmp(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_storage)) == 0;
}

#if XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET
void XcpTl_PrintConnectionInformation(void) {
    char buf[128];

    printf(
        "XCPonEth -- Listening on port %u / %s [%s]\n\r", XCP_ETH_DEFAULT_PORT, Xcp_Options.tcp ? "TCP" : "UDP",
        Xcp_Options.ipv6 ? "IPv6" : "IPv4"
    );
}
#elif XCP_TRANSPORT_LAYER == XCP_ON_BTH
void XcpTl_PrintConnectionInformation(void) {
    XcpTl_PrintBtDetails();
}
#endif

#if XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET

#endif
