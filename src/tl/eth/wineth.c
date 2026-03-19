/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
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

#include <memory.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_eth.h"
#include "xcp_hw.h"
/*!!! END-INCLUDE-SECTION !!!*/

#include "xcp_config.h"

extern XcpTl_ConnectionType XcpTl_Connection;

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    #include "xcp_timecorr.h"
#endif
/* Forward declaration for multicast thread */
static unsigned __stdcall XcpTl_MulticastThread(void *param);

/* Protocol/Address-Family selection: allow runtime when available, otherwise use compile-time defaults */
#ifndef XCP_ETH_USE_TCP
    #if defined(XCP_TRANSPORT_LAYER) && (XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET)
        #define XCP_ETH_USE_TCP (Xcp_Options.tcp)
    #else
        #define XCP_ETH_USE_TCP 1
    #endif
#endif
#ifndef XCP_ETH_USE_IPV6
    #if defined(XCP_TRANSPORT_LAYER) && (XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET)
        #define XCP_ETH_USE_IPV6 (Xcp_Options.ipv6)
    #else
        #define XCP_ETH_USE_IPV6 0
    #endif
#endif

/* Port selection: prefer runtime option when available, otherwise default */
#ifndef XCP_ETH_PORT
    #if defined(XCP_TRANSPORT_LAYER) && (XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET)
        #define XCP_ETH_PORT (Xcp_Options.port)
    #else
        #define XCP_ETH_PORT (XCP_ETH_DEFAULT_PORT)
    #endif
#endif

#define DEFAULT_FAMILY   PF_UNSPEC    // Accept either IPv4 or IPv6
#define DEFAULT_SOCKTYPE SOCK_STREAM  //

#define ADDR_LEN ((int)sizeof(SOCKADDR_STORAGE))

extern XcpTl_ConnectionType XcpTl_Connection;

void Xcp_DispatchCommand(Xcp_PduType const * const pdu);

extern Xcp_PduType Xcp_CtoIn;
extern Xcp_PduType Xcp_CtoOut;

static boolean Xcp_EnableSocketOption(SOCKET sock, int option);

#if 0
static boolean Xcp_DisableSocketOption(SOCKET sock, int option);
#endif

static bool XcpTl_ParseIpv4String(const char *ip, struct in_addr *out) {
    return InetPtonA(AF_INET, ip, out) == 1;
}

static boolean Xcp_EnableSocketOption(SOCKET sock, int option) {
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const char enable = 1;

    if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
        return XCP_FALSE;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){ 1 }, sizeof(int)) < 0) {
        return XCP_FALSE;
    }
#endif
    return XCP_TRUE;
}

void XcpTl_Init(void) {
    WSADATA   wsa;
    ADDRINFO  Hints;
    ADDRINFO *AddrInfo;
    ADDRINFO *AI;
    char     *Address = NULL;
    char      PortStr[16];
    SOCKET    serverSockets[FD_SETSIZE];
    int       boundSocketNum = -1;
    int       ret;
    int       idx;
    DWORD     dwTimeAdjustment    = 0UL;
    DWORD     dwTimeIncrement     = 0UL;
    BOOL      fAdjustmentDisabled = XCP_TRUE;
    /* unsigned long ul = 1; */

    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
    XcpTl_Connection.connectedSocket = INVALID_SOCKET;
    memset(&Hints, 0, sizeof(Hints));
    GetSystemTimeAdjustment(&dwTimeAdjustment, &dwTimeIncrement, &fAdjustmentDisabled);
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        XcpHw_ErrorMsg("XcpTl_Init:WSAStartup()", WSAGetLastError());
        exit(EXIT_FAILURE);
    } else {
    }
    XcpTl_Connection.socketType = XCP_ETH_USE_TCP ? SOCK_STREAM : SOCK_DGRAM;
    Hints.ai_family             = XCP_ETH_USE_IPV6 ? PF_INET6 : PF_INET;
    Hints.ai_socktype           = XcpTl_Connection.socketType;
    Hints.ai_flags              = AI_PASSIVE;
    /* Build port string from runtime option */
    memset(PortStr, 0, sizeof(PortStr));
    _snprintf(PortStr, sizeof(PortStr) - 1, "%u", (unsigned)XCP_ETH_PORT);
    ret = getaddrinfo(Address, PortStr, &Hints, &AddrInfo);
    if (ret != 0) {
        XcpHw_ErrorMsg("XcpTl_Init::getaddrinfo()", WSAGetLastError());
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
        if (serverSockets[idx] == INVALID_SOCKET) {
            XcpHw_ErrorMsg("XcpTl_Init::socket()", WSAGetLastError());
            continue;
        }
        if (bind(serverSockets[idx], AI->ai_addr, (int)AI->ai_addrlen) != 0) {
            XcpHw_ErrorMsg("XcpTl_Init::bind()", WSAGetLastError());
            continue;
        }

        memcpy(&XcpTl_Connection.localAddress, AI->ai_addr, sizeof(SOCKADDR_STORAGE));
        // XcpTl_Connection.localAddress = *AI->ai_addr;

        if (XcpTl_Connection.socketType == SOCK_STREAM) {
            if (listen(serverSockets[idx], 1) == SOCKET_ERROR) {
                XcpHw_ErrorMsg("XcpTl_Init::listen()", WSAGetLastError());
                continue;
            }
        }
        boundSocketNum               = idx;
        XcpTl_Connection.boundSocket = serverSockets[boundSocketNum];
        /* ioctlsocket(XcpTl_Connection.boundSocket, FIONBIO, &ul); */
        break; /* Grab first address. */
    }
    freeaddrinfo(AddrInfo);

    /* Print effective connection information */
    XcpTl_PrintConnectionInformation();
    if (boundSocketNum == -1) {
        fprintf(
            stderr,
            "Fatal error: unable to serve on any address.\nPerhaps"
            " a server is already running on port %u / %s [%s]?\n",
            XCP_ETH_DEFAULT_PORT, (XCP_ETH_USE_TCP ? "TCP" : "UDP"), (XCP_ETH_USE_IPV6 ? "IPv6" : "IPv4")
        );
        WSACleanup();
        exit(2);
    }
    if (!Xcp_EnableSocketOption(XcpTl_Connection.boundSocket, SO_REUSEADDR)) {
        XcpHw_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEADDR)", WSAGetLastError());
    }
#if 0
    mode = 1;
    ioctlsocket(XcpTl_Connection.boundSocket, FIONBIO, &ul);
#endif
    /* Create discovery multicast socket (GET_SLAVE_ID/EXT/SET_IP) */
    XcpTl_Connection.discoverySocket = INVALID_SOCKET;
#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON)
    {
        SOCKET             dsock;
        struct sockaddr_in daddr;
        struct ip_mreq     dmreq;
        char               d_ip[32];

        _snprintf(
            d_ip, sizeof(d_ip), "%u.%u.%u.%u", (unsigned)XCP_ETH_DISCOVERY_MCAST_IP0, (unsigned)XCP_ETH_DISCOVERY_MCAST_IP1,
            (unsigned)XCP_ETH_DISCOVERY_MCAST_IP2, (unsigned)XCP_ETH_DISCOVERY_MCAST_IP3
        );

        dsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (dsock != INVALID_SOCKET) {
            int reuse = 1;
            setsockopt(dsock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));

            ZeroMemory(&daddr, sizeof(daddr));
            daddr.sin_family      = AF_INET;
            daddr.sin_port        = htons((uint16_t)XCP_ETH_DISCOVERY_MCAST_PORT);
            daddr.sin_addr.s_addr = INADDR_ANY;

            if (bind(dsock, (struct sockaddr *)&daddr, sizeof(daddr)) == 0) {
                if (!XcpTl_ParseIpv4String(d_ip, &dmreq.imr_multiaddr)) {
                    XcpHw_ErrorMsg("XcpTl_Init:invalid discovery mcast IP", WSAEINVAL);
                    closesocket(dsock);
                    dsock = INVALID_SOCKET;
                }
                dmreq.imr_interface.s_addr = INADDR_ANY;
                if ((dsock != INVALID_SOCKET) &&
                    setsockopt(dsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&dmreq, sizeof(dmreq)) == 0) {
                    XcpTl_Connection.discoverySocket = dsock;
                    printf("XCPonEth -- Discovery multicast listener: %s:%u\n", d_ip, (unsigned)XCP_ETH_DISCOVERY_MCAST_PORT);
                } else {
                    XcpHw_ErrorMsg("XcpTl_Init:IP_ADD_MEMBERSHIP(discovery)", WSAGetLastError());
                    closesocket(dsock);
                }
            } else {
                XcpHw_ErrorMsg("XcpTl_Init:discovery bind()", WSAGetLastError());
                closesocket(dsock);
            }
        } else {
            XcpHw_ErrorMsg("XcpTl_Init:discovery socket()", WSAGetLastError());
        }
    }
#endif /* XCP_ENABLE_ETH_DISCOVERY */

    /* Create multicast socket for GET_DAQ_CLOCK_MULTICAST */
    XcpTl_Connection.multicastSocket = INVALID_SOCKET;
#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    {
        SOCKET             msock;
        struct sockaddr_in maddr;
        struct ip_mreq     mreq;
        uint16_t           cluster_id  = XcpTimecorr_GetClusterAffiliation();
        unsigned char      mcast_upper = (unsigned char)((cluster_id >> 8u) & 0xFFu);
        unsigned char      mcast_lower = (unsigned char)(cluster_id & 0xFFu);
        char               mcast_ip[32];

        _snprintf(mcast_ip, sizeof(mcast_ip), "239.255.%u.%u", (unsigned)mcast_upper, (unsigned)mcast_lower);

        msock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (msock != INVALID_SOCKET) {
            int reuse = 1;
            setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));

            ZeroMemory(&maddr, sizeof(maddr));
            maddr.sin_family      = AF_INET;
            maddr.sin_port        = htons(XCP_TIMECORR_MULTICAST_PORT);
            maddr.sin_addr.s_addr = INADDR_ANY;

            if (bind(msock, (struct sockaddr *)&maddr, sizeof(maddr)) == 0) {
                if (!XcpTl_ParseIpv4String(mcast_ip, &mreq.imr_multiaddr)) {
                    XcpHw_ErrorMsg("XcpTl_Init:invalid timecorr mcast IP", WSAEINVAL);
                    closesocket(msock);
                    msock = INVALID_SOCKET;
                }
                mreq.imr_interface.s_addr = INADDR_ANY;
                if ((msock != INVALID_SOCKET) &&
                    setsockopt(msock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)) == 0) {
                    XcpTl_Connection.multicastSocket = msock;
                    printf("XCPonEth -- Timecorr multicast listener: %s:%u\n", mcast_ip, XCP_TIMECORR_MULTICAST_PORT);
                } else {
                    XcpHw_ErrorMsg("XcpTl_Init:IP_ADD_MEMBERSHIP(timecorr)", WSAGetLastError());
                    closesocket(msock);
                }
            } else {
                XcpHw_ErrorMsg("XcpTl_Init:multicast bind()", WSAGetLastError());
                closesocket(msock);
            }
        } else {
            XcpHw_ErrorMsg("XcpTl_Init:multicast socket()", WSAGetLastError());
        }
    }
#endif /* XCP_ENABLE_TIME_CORRELATION */

    if ((XcpTl_Connection.discoverySocket != INVALID_SOCKET) || (XcpTl_Connection.multicastSocket != INVALID_SOCKET)) {
        _beginthreadex(NULL, 0, XcpTl_MulticastThread, NULL, 0, NULL);
    }
}

void XcpTl_PrintConnectionInformation(void);

void XcpTl_DeInit(void) {
    if (XcpTl_Connection.multicastSocket != INVALID_SOCKET) {
        closesocket(XcpTl_Connection.multicastSocket);
        XcpTl_Connection.multicastSocket = INVALID_SOCKET;
    }
    if (XcpTl_Connection.discoverySocket != INVALID_SOCKET) {
        closesocket(XcpTl_Connection.discoverySocket);
        XcpTl_Connection.discoverySocket = INVALID_SOCKET;
    }
    closesocket(XcpTl_Connection.boundSocket);
    WSACleanup();
}

#if 0
void XcpTl_MainFunction(void) {
    int n;
    int addr_len = ADDR_LEN;

    if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (XcpTl_Connection.connectedSocket == INVALID_SOCKET) {
            XcpTl_Connection.connectedSocket = accept(
                XcpTl_Connection.boundSocket, (SOCKADDR *)&XcpTl_Connection.connectionAddress, &addr_len
            );
            if (XcpTl_Connection.connectedSocket != INVALID_SOCKET) {
                printf("XCPonEth -- Client connected.\n");
            }
            return; /* Defer recv to next cycle */
        }
    }

    if (XcpTl_Connection.socketType == SOCK_DGRAM) {
        n = recvfrom(
            XcpTl_Connection.boundSocket, (char *)Xcp_CtoIn.data, XCP_MAX_CTO, 0,
            (SOCKADDR *)&XcpTl_Connection.connectionAddress, &addr_len
        );
    } else {
        /* SOCK_STREAM */
        if (XcpTl_Connection.connectedSocket == INVALID_SOCKET) {
            return;
        }
        n = recv(XcpTl_Connection.connectedSocket, (char *)Xcp_CtoIn.data, XCP_MAX_CTO, 0);
    }

    if (n > 0) {
        Xcp_CtoIn.len = (uint8_t)n;
        Xcp_DispatchCommand(&Xcp_CtoIn);
    } else if (n == 0 || (n < 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
        if (XcpTl_Connection.socketType == SOCK_STREAM && XcpTl_Connection.connectedSocket != INVALID_SOCKET) {
            printf("XCPonEth -- Client disconnected.\n");
            closesocket(XcpTl_Connection.connectedSocket);
            XcpTl_Connection.connectedSocket = INVALID_SOCKET;
        }
    }
}
#endif

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    XCP_TL_ENTER_CRITICAL();
    if (XcpTl_Connection.socketType == SOCK_DGRAM) {
        if (sendto(
                XcpTl_Connection.boundSocket, (char const *)buf, len, 0,
                (SOCKADDR const *)(SOCKADDR_STORAGE const *)&XcpTl_Connection.connectionAddress, ADDR_LEN
            ) == SOCKET_ERROR) {
            XcpHw_ErrorMsg("XcpTl_Send:sendto()", WSAGetLastError());
        }
    } else if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (XcpTl_Connection.connectedSocket != INVALID_SOCKET) {
            if (send(XcpTl_Connection.connectedSocket, (char const *)buf, len, 0) == SOCKET_ERROR) {
                XcpHw_ErrorMsg("XcpTl_Send:send()", WSAGetLastError());
                closesocket(XcpTl_Connection.connectedSocket);
                XcpTl_Connection.connectedSocket = INVALID_SOCKET;
            }
        }
    }
    XCP_TL_LEAVE_CRITICAL();
}

#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON) || (defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON))

unsigned __stdcall XcpTl_MulticastThread(void *param) {
    uint8_t buf[128];

    (void)param;

    for (;;) {
        fd_set rfds;
        SOCKET max_sock = 0;
        int    ready;

        FD_ZERO(&rfds);
        if (XcpTl_Connection.discoverySocket != INVALID_SOCKET) {
            FD_SET(XcpTl_Connection.discoverySocket, &rfds);
            if (XcpTl_Connection.discoverySocket > max_sock) {
                max_sock = XcpTl_Connection.discoverySocket;
            }
        }
        if (XcpTl_Connection.multicastSocket != INVALID_SOCKET) {
            FD_SET(XcpTl_Connection.multicastSocket, &rfds);
            if (XcpTl_Connection.multicastSocket > max_sock) {
                max_sock = XcpTl_Connection.multicastSocket;
            }
        }
        if ((XcpTl_Connection.discoverySocket == INVALID_SOCKET) && (XcpTl_Connection.multicastSocket == INVALID_SOCKET)) {
            break;
        }

        ready = select((int)max_sock + 1, &rfds, NULL, NULL, NULL);
        if (ready <= 0) {
            continue;
        }

        if ((XcpTl_Connection.discoverySocket != INVALID_SOCKET) && FD_ISSET(XcpTl_Connection.discoverySocket, &rfds)) {
            int n = recv(XcpTl_Connection.discoverySocket, (char *)buf, (int)sizeof(buf), 0);
            if (n > 0) {
                XcpTl_HandleTransportMulticastPacket(buf, n);
            }
        }
        if ((XcpTl_Connection.multicastSocket != INVALID_SOCKET) && FD_ISSET(XcpTl_Connection.multicastSocket, &rfds)) {
            int n = recv(XcpTl_Connection.multicastSocket, (char *)buf, (int)sizeof(buf), 0);
            if (n > 0) {
                XcpTl_HandleTransportMulticastPacket(buf, n);
            }
        }
    }
    return 0;
}

#endif /* discovery/timecorr multicast listener */

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
/*
 * Handle GET_DAQ_CLOCK_MULTICAST arriving via the unicast channel
 * (transport layer command 0xF2, sub-command 0xFA).
 */
void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
    uint8_t sub_cmd = Xcp_GetByte(pdu, UINT8(1));

    if (sub_cmd == UINT8(0xFA)) { /* GET_DAQ_CLOCK_MULTICAST */
        uint16_t cluster_id = Xcp_GetWord(pdu, UINT8(2));
        uint8_t  counter    = Xcp_GetByte(pdu, UINT8(4));
        XcpTimecorr_HandleMulticast(cluster_id, counter);
    }
}

/*
 * Rejoin a new multicast group when cluster affiliation changes.
 */
void XcpTl_UpdateMulticastGroup(uint16_t cluster_id) {
    struct ip_mreq mreq;
    char           mcast_ip[32];
    unsigned char  mcast_upper = (unsigned char)((cluster_id >> 8u) & 0xFFu);
    unsigned char  mcast_lower = (unsigned char)(cluster_id & 0xFFu);

    if (XcpTl_Connection.multicastSocket == INVALID_SOCKET) {
        return;
    }
    _snprintf(mcast_ip, sizeof(mcast_ip), "239.255.%u.%u", (unsigned)mcast_upper, (unsigned)mcast_lower);
    if (!XcpTl_ParseIpv4String(mcast_ip, &mreq.imr_multiaddr)) {
        XcpHw_ErrorMsg("XcpTl_UpdateMulticastGroup:invalid IP", WSAEINVAL);
        return;
    }
    mreq.imr_interface.s_addr = INADDR_ANY;
    setsockopt(XcpTl_Connection.multicastSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq));
}

#else /* !XCP_ENABLE_TIME_CORRELATION */

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
    (void)pdu;
}

void XcpTl_UpdateMulticastGroup(uint16_t cluster_id) {
    (void)cluster_id;
}

#endif /* XCP_ENABLE_TIME_CORRELATION */
