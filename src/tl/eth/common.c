/*
 * BlueParrot XCP
 *
 * (C) 2021-2026 by Christoph Schueler <github.com/Christoph2,
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
#include <string.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_eth.h"
/*!!! END-INCLUDE-SECTION !!!*/

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    #include "xcp_timecorr.h"
#endif

#define MAX_IDENTIFIER_LEN (96)

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

static bool XcpTl_ParseIpv4(const char *ip, struct in_addr *out) {
#if defined(_WIN32)
    return InetPtonA(AF_INET, ip, out) == 1;
#else
    return inet_pton(AF_INET, ip, out) == 1;
#endif
}

static bool XcpTl_FindPrimaryIpv4(uint8_t out_ip[4]) {
    struct addrinfo  hints;
    struct addrinfo *res  = NULL;
    struct addrinfo *curr = NULL;
    char             host[256];
    uint32_t         fallback_ip = 0u;

    memset(out_ip, 0, 4);
    memset(host, 0, sizeof(host));
    memset(&hints, 0, sizeof(hints));

    if (gethostname(host, sizeof(host) - 1) != 0) {
        return false;
    }
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, NULL, &hints, &res) != 0) {
        return false;
    }

    for (curr = res; curr != NULL; curr = curr->ai_next) {
        const struct sockaddr_in *sin = (const struct sockaddr_in *)curr->ai_addr;
        uint32_t                  ip  = ntohl(sin->sin_addr.s_addr);

        if (ip == 0u) {
            continue;
        }
        /* Remember loopback as fallback, but prefer non-loopback. */
        if ((ip & 0xFF000000u) == 0x7F000000u) {
            if (fallback_ip == 0u) {
                fallback_ip = ip;
            }
            continue;
        }

        out_ip[0] = (uint8_t)((ip >> 24u) & 0xFFu);
        out_ip[1] = (uint8_t)((ip >> 16u) & 0xFFu);
        out_ip[2] = (uint8_t)((ip >> 8u) & 0xFFu);
        out_ip[3] = (uint8_t)(ip & 0xFFu);
        freeaddrinfo(res);
        return true;
    }

    if (fallback_ip != 0u) {
        out_ip[0] = (uint8_t)((fallback_ip >> 24u) & 0xFFu);
        out_ip[1] = (uint8_t)((fallback_ip >> 16u) & 0xFFu);
        out_ip[2] = (uint8_t)((fallback_ip >> 8u) & 0xFFu);
        out_ip[3] = (uint8_t)(fallback_ip & 0xFFu);
        freeaddrinfo(res);
        return true;
    }

    freeaddrinfo(res);
    return false;
}

uint16_t XcpTl_GetLocalPort(void) {
    uint16_t port = 0;

    if (((struct sockaddr *)&XcpTl_Connection.localAddress)->sa_family == AF_INET) {
        const struct sockaddr_in *sin = (const struct sockaddr_in *)&XcpTl_Connection.localAddress;
        port                          = ntohs(sin->sin_port);
    }
    return port;
}

void XcpTl_GetLocalIpv4(uint8_t out_ip[4]) {
    memset(out_ip, 0, 4);
    if (((struct sockaddr *)&XcpTl_Connection.localAddress)->sa_family == AF_INET) {
        const struct sockaddr_in *sin = (const struct sockaddr_in *)&XcpTl_Connection.localAddress;
#if defined(_WIN32)
        uint32_t ip = ntohl(sin->sin_addr.S_un.S_addr);
#else
        uint32_t ip = ntohl(sin->sin_addr.s_addr);
#endif
        if (ip != 0u) {
            out_ip[0] = (uint8_t)((ip >> 24u) & 0xFFu);
            out_ip[1] = (uint8_t)((ip >> 16u) & 0xFFu);
            out_ip[2] = (uint8_t)((ip >> 8u) & 0xFFu);
            out_ip[3] = (uint8_t)(ip & 0xFFu);
            return;
        }
    }
    /* When bound to INADDR_ANY, determine a primary local IPv4 for announcements. */
    (void)XcpTl_FindPrimaryIpv4(out_ip);
}

uint8_t XcpTl_BuildStatus(bool extended) {
    uint8_t proto  = (XcpTl_Connection.socketType == SOCK_DGRAM) ? (uint8_t)1 : (uint8_t)0; /* 0=TCP, 1=UDP */
    uint8_t status = (uint8_t)(proto & (uint8_t)0x03);                                      /* bits[1:0] */

    /* IP_VERSION (bit2) = 0 (IPv4) */
    /* SLV_AVAILABILITY (bit3) */
    status |= (uint8_t)((Xcp_GetConnectionState() == XCP_CONNECTED ? 1u : 0u) << 3u);

    if (extended == false) {
        /* GET_SLAVE_ID: bit4 indicates support for extended */
        status |= (uint8_t)0x10;
    }

    return status;
}

void XcpTl_SendUdpResponse(const char *mcast_ip, uint16_t port, uint8_t const *payload, size_t len) {
    const size_t frame_len = XCP_ETH_HEADER_SIZE + len;
    uint8_t     *frame     = (uint8_t *)malloc(frame_len);

    if (frame == NULL) {
        return;
    }

    /* LEN (little-endian) + COUNTER (always 0 for UDP multicast) */
    frame[0] = (uint8_t)(len & 0xFFu);
    frame[1] = (uint8_t)((len >> 8u) & 0xFFu);
    frame[2] = 0u;
    frame[3] = 0u;
    memcpy(frame + XCP_ETH_HEADER_SIZE, payload, len);
#if defined(_WIN32)
    SOCKET             s;
    struct sockaddr_in addr;
    int                reuse = 1;

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) {
        XcpHw_ErrorMsg("XcpTl_SendUdpResponse:socket()", WSAGetLastError());
        free(frame);
        return;
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));

    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (!XcpTl_ParseIpv4(mcast_ip, &addr.sin_addr)) {
        free(frame);
        closesocket(s);
        return;
    }

    if (sendto(s, (const char *)frame, (int)frame_len, 0, (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        XcpHw_ErrorMsg("XcpTl_SendUdpResponse:sendto()", WSAGetLastError());
    }
    closesocket(s);
#else
    int                s;
    struct sockaddr_in addr;
    int                reuse = 1;

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        XcpHw_ErrorMsg("XcpTl_SendUdpResponse:socket()", errno);
        free(frame);
        return;
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (!XcpTl_ParseIpv4(mcast_ip, &addr.sin_addr)) {
        free(frame);
        close(s);
        return;
    }

    if (sendto(s, frame, frame_len, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        XcpHw_ErrorMsg("XcpTl_SendUdpResponse:sendto()", errno);
    }
    close(s);
#endif
    free(frame);
}

bool XcpTl_HandleTransportMulticastPacket(const uint8_t *buf, size_t n) {
    uint8_t sub;
    uint8_t payload[256] = { 0 };

    if (n < 9) {
        return false;
    }
    if (buf[4] != (uint8_t)0xF2) {
        return false;
    }

    sub = buf[5];

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    if (sub == (uint8_t)0xFA) {
        /* GET_DAQ_CLOCK_MULTICAST */
        uint16_t cluster_id = (uint16_t)((uint16_t)buf[6] | ((uint16_t)buf[7] << 8u));
        uint8_t  counter    = buf[8];
        XcpTimecorr_HandleMulticast(cluster_id, counter);
        return true;
    }
#endif
#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON)
    if (sub == (uint8_t)0xFF) {
        /* GET_SLAVE_ID */
        uint16_t resp_port = (uint16_t)((uint16_t)buf[6] | ((uint16_t)buf[7] << 8u));
        char     resp_ip[32];
        uint8_t  resource = Xcp_GetResourceMask();
        uint8_t  offset   = (uint8_t)XCP_MIN(Xcp_GetId0.len, MAX_IDENTIFIER_LEN);
        uint8_t  ip[4];

        (void)snprintf(
            resp_ip, sizeof(resp_ip), "%u.%u.%u.%u", (unsigned)buf[8], (unsigned)buf[9], (unsigned)buf[10], (unsigned)buf[11]
        );

        DBG_TRACE("GET_SLAVE_ID [response-address: %s:%d]\n", resp_ip, resp_port);
        XcpTl_GetLocalIpv4(ip);
        payload[0]  = ip[0];
        payload[1]  = ip[1];
        payload[2]  = ip[2];
        payload[3]  = ip[3];
        payload[16] = (uint8_t)(XcpTl_GetLocalPort() & 0xFFu);
        payload[17] = (uint8_t)((XcpTl_GetLocalPort() >> 8u) & 0xFFu);
        payload[18] = XcpTl_BuildStatus(false);
        payload[19] = resource; /* RESOURCE */
        /* Length DWORD */
        payload[20] = offset;
        payload[21] = payload[22] = payload[23] = (uint8_t)0;

        for (uint8_t idx = 0; idx < offset; ++idx) {
            payload[24 + idx] = Xcp_GetId0.name[idx];
        }

        XcpTl_SendUdpResponse(resp_ip, resp_port, payload, 24u + offset);
        return true;
    } else if (sub == (uint8_t)0xFD) {
        /* GET_SLAVE_ID_EXTENDED */
        uint16_t resp_port = (uint16_t)((uint16_t)buf[6] | ((uint16_t)buf[7] << 8u));
        char     resp_ip[256];
        uint8_t  resource = Xcp_GetResourceMask();
        uint8_t  ip[4];
        uint8_t  offset = (uint8_t)XCP_MIN(Xcp_GetId0.len, MAX_IDENTIFIER_LEN);

        (void)snprintf(
            resp_ip, sizeof(resp_ip), "%u.%u.%u.%u", (unsigned)buf[8], (unsigned)buf[9], (unsigned)buf[10], (unsigned)buf[11]
        );

        DBG_TRACE("GET_SLAVE_ID_EXTENDED [response-address: %s:%d]\n", resp_ip, resp_port);
        XcpTl_GetLocalIpv4(ip);
        payload[0]  = (uint8_t)0xFF;
        payload[1]  = (uint8_t)0xFD;
        payload[2]  = ip[0];
        payload[3]  = ip[1];
        payload[4]  = ip[2];
        payload[5]  = ip[3];
        payload[18] = (uint8_t)(XcpTl_GetLocalPort() & 0xFFu);
        payload[19] = (uint8_t)((XcpTl_GetLocalPort() >> 8u) & 0xFFu);
        payload[20] = XcpTl_BuildStatus(true);
        payload[21] = resource; /* RESOURCE */
        /* Length DWORD*/
        payload[22] = offset;
        payload[23] = payload[24] = payload[25] = (uint8_t)0;

        for (uint8_t idx = 0; idx < offset; ++idx) {
            payload[26 + idx] = Xcp_GetId0.name[idx];
        }

        payload[26 + offset] = (uint8_t)XCP_ETH_DISCOVERY_MAC0;
        payload[27 + offset] = (uint8_t)XCP_ETH_DISCOVERY_MAC1;
        payload[28 + offset] = (uint8_t)XCP_ETH_DISCOVERY_MAC2;
        payload[29 + offset] = (uint8_t)XCP_ETH_DISCOVERY_MAC3;
        payload[30 + offset] = (uint8_t)XCP_ETH_DISCOVERY_MAC4;
        payload[31 + offset] = (uint8_t)XCP_ETH_DISCOVERY_MAC5;

        XcpTl_SendUdpResponse(resp_ip, resp_port, payload, 32u + offset);
        return true;
    } else if (sub == (uint8_t)0xFC) {
        /* SET_SLAVE_IP_ADDRESS – acknowledge without applying */
        uint16_t resp_port = (uint16_t)((uint16_t)buf[6] | ((uint16_t)buf[7] << 8u));
        char     resp_ip[32];

        (void)snprintf(
            resp_ip, sizeof(resp_ip), "%u.%u.%u.%u", (unsigned)buf[8], (unsigned)buf[9], (unsigned)buf[10], (unsigned)buf[11]
        );

        DBG_TRACE("SET_SLAVE_IP_ADDRESS [response-address: %s:%d]\n", resp_ip, resp_port);
        payload[0] = (uint8_t)0xFF;
        payload[1] = (uint8_t)0xFC;
        payload[2] = (uint8_t)XCP_ETH_DISCOVERY_SET_IP_STATUS;
        payload[3] = (uint8_t)XCP_ETH_DISCOVERY_MAC0;
        payload[4] = (uint8_t)XCP_ETH_DISCOVERY_MAC1;
        payload[5] = (uint8_t)XCP_ETH_DISCOVERY_MAC2;
        payload[6] = (uint8_t)XCP_ETH_DISCOVERY_MAC3;
        payload[7] = (uint8_t)XCP_ETH_DISCOVERY_MAC4;
        payload[8] = (uint8_t)XCP_ETH_DISCOVERY_MAC5;

        XcpTl_SendUdpResponse(resp_ip, resp_port, payload, 9u);
        return true;
    }
#endif
    (void)buf;
    return false;
}

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
    uint8_t  ip[4];
    char     ip_str[32];
    uint16_t port = XcpTl_GetLocalPort();

    if (port == 0u) {
        port = (uint16_t)XCP_ETH_DEFAULT_PORT;
    }

    XcpTl_GetLocalIpv4(ip);
    (void)snprintf(ip_str, sizeof(ip_str), "%u.%u.%u.%u", (unsigned)ip[0], (unsigned)ip[1], (unsigned)ip[2], (unsigned)ip[3]);

    printf(
        "XCPonEth -- Listening on %s:%u / %s [%s]\n\r", ip_str, (unsigned)port, Xcp_Options.tcp ? "TCP" : "UDP",
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
