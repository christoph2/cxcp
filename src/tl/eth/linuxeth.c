/*
 * BlueParrot XCP
 *
 * (C) 2007-2021 by Christoph Schueler <github.com/Christoph2,
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

#include <stdio.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_eth.h"
#include "xcp_hw.h"
/*!!! END-INCLUDE-SECTION !!!*/

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    #include <pthread.h>

    #include "xcp_timecorr.h"
static void *XcpTl_MulticastThread(void *param);
#endif /* XCP_ENABLE_TIME_CORRELATION */

socklen_t addrSize = sizeof(struct sockaddr_storage);

extern XcpTl_ConnectionType XcpTl_Connection;

static bool Xcp_EnableSocketOption(int sock, int option);
static bool Xcp_DisableSocketOption(int sock, int option);

static bool Xcp_EnableSocketOption(int sock, int option) {
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const char enable = 1;

    if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
        return false;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){ 1 }, sizeof(int)) < 0) {
        return false;
    }
#endif
    return true;
}

static bool Xcp_DisableSocketOption(int sock, int option) {
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const char enable = 0;

    if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
        return false;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){ 0 }, sizeof(int)) < 0) {
        return false;
    }
#endif
    return true;
}

void XcpTl_Init(void) {
    struct addrinfo  hints;
    struct addrinfo *addr_info = NULL;
    char             port[16];
    int              sock = 0;
    int              ret  = 0;

    XcpUtl_ZeroMem(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
    memset(&hints, 0, sizeof(hints));
    XcpTl_Connection.socketType = Xcp_Options.tcp ? SOCK_STREAM : SOCK_DGRAM;
    (void)snprintf(port, sizeof(port), "%d", Xcp_Options.port);
    hints.ai_family   = Xcp_Options.ipv6 ? PF_INET6 : PF_INET;
    hints.ai_socktype = XcpTl_Connection.socketType;
    hints.ai_flags    = AI_NUMERICHOST | AI_PASSIVE;
    ret               = getaddrinfo(NULL, port, &hints, &addr_info);

    if (ret != 0) {
        XcpHw_ErrorMsg("XcpTl_Init::getaddrinfo()", ret);
        return;
    }

    sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (sock == -1) {
        XcpHw_ErrorMsg("XcpTl_Init::socket()", errno);
        freeaddrinfo(addr_info);
        return;
    }
    if (bind(sock, addr_info->ai_addr, addr_info->ai_addrlen) == -1) {
        XcpHw_ErrorMsg("XcpTl_Init::bind()", errno);
        close(sock);
        freeaddrinfo(addr_info);
        return;
    }
    if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (listen(sock, 1) == -1) {
            XcpHw_ErrorMsg("XcpTl_Init::listen()", errno);
            close(sock);
            freeaddrinfo(addr_info);
            return;
        }
    }

    memcpy(&XcpTl_Connection.localAddress, addr_info->ai_addr, addr_info->ai_addrlen);

    XcpTl_Connection.boundSocket = sock;
    freeaddrinfo(addr_info);
    if (!Xcp_EnableSocketOption(XcpTl_Connection.boundSocket, SO_REUSEADDR)) {
        XcpHw_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEADDR)", errno);
    }
#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    /* Create multicast socket for GET_DAQ_CLOCK_MULTICAST */
    XcpTl_Connection.multicastSocket = -1;
    {
        int                msock;
        struct sockaddr_in maddr;
        struct ip_mreq     mreq;
        uint16_t           cluster_id  = XcpTimecorr_GetClusterAffiliation();
        unsigned char      mcast_upper = (unsigned char)((cluster_id >> 8u) & 0xFFu);
        unsigned char      mcast_lower = (unsigned char)(cluster_id & 0xFFu);
        char               mcast_ip[32];
        int                reuse = 1;
        pthread_t          mcast_thread;

        snprintf(mcast_ip, sizeof(mcast_ip), "239.255.%u.%u", (unsigned)mcast_upper, (unsigned)mcast_lower);

        msock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (msock >= 0) {
            setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

            memset(&maddr, 0, sizeof(maddr));
            maddr.sin_family      = AF_INET;
            maddr.sin_port        = htons(XCP_TIMECORR_MULTICAST_PORT);
            maddr.sin_addr.s_addr = INADDR_ANY;

            if (bind(msock, (struct sockaddr *)&maddr, sizeof(maddr)) == 0) {
                mreq.imr_multiaddr.s_addr = inet_addr(mcast_ip);
                mreq.imr_interface.s_addr = INADDR_ANY;
                if (setsockopt(msock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == 0) {
                    XcpTl_Connection.multicastSocket = msock;
                    pthread_create(&mcast_thread, NULL, XcpTl_MulticastThread, NULL);
                    pthread_detach(mcast_thread);
                    printf("XCPonEth -- Multicast listener: %s:%u\n", mcast_ip, XCP_TIMECORR_MULTICAST_PORT);
                } else {
                    XcpHw_ErrorMsg("XcpTl_Init:IP_ADD_MEMBERSHIP", errno);
                    close(msock);
                }
            } else {
                XcpHw_ErrorMsg("XcpTl_Init:multicast bind()", errno);
                close(msock);
            }
        } else {
            XcpHw_ErrorMsg("XcpTl_Init:multicast socket()", errno);
        }
    }
#endif /* XCP_ENABLE_TIME_CORRELATION */
}

void XcpTl_DeInit(void) {
#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    if (XcpTl_Connection.multicastSocket >= 0) {
        close(XcpTl_Connection.multicastSocket);
        XcpTl_Connection.multicastSocket = -1;
    }
#endif /* XCP_ENABLE_TIME_CORRELATION */
    if (XcpTl_Connection.boundSocket >= 0) {
        close(XcpTl_Connection.boundSocket);
        XcpTl_Connection.boundSocket = -1;
    }
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    // XcpUtl_Hexdump(buf,  len);
    XCP_TL_ENTER_CRITICAL();
    if (XcpTl_Connection.socketType == SOCK_DGRAM) {
        if (sendto(
                XcpTl_Connection.boundSocket, (char const *)buf, len, 0,
                (struct sockaddr const *)&XcpTl_Connection.connectionAddress, addrSize
            ) == -1) {
            XcpHw_ErrorMsg("XcpTl_Send:sendto()", errno);
        }
    } else if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (send(XcpTl_Connection.connectedSocket, (char const *)buf, len, 0) == -1) {
            XcpHw_ErrorMsg("XcpTl_Send:send()", errno);
            close(XcpTl_Connection.connectedSocket);
        }
    }
    XCP_TL_LEAVE_CRITICAL();
}

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)

/*
 * Thread that receives GET_DAQ_CLOCK_MULTICAST packets.
 * XCP ETH multicast packet layout (Intel byte order):
 *   [0..1]  len (WORD)
 *   [2..3]  transport counter (WORD)
 *   [4]     0xF2  (TRANSPORT_LAYER_CMD)
 *   [5]     0xFA  (GET_DAQ_CLOCK_MULTICAST sub-command)
 *   [6..7]  cluster_id (WORD)
 *   [8]     application counter byte
 */
static void *XcpTl_MulticastThread(void *param) {
    uint8_t  buf[32];
    ssize_t  n;
    uint16_t cluster_id;
    uint8_t  counter;

    (void)param;

    while (XcpTl_Connection.multicastSocket >= 0) {
        n = recv(XcpTl_Connection.multicastSocket, buf, sizeof(buf), 0);
        if (n < 9) {
            continue;
        }
        if (buf[4] != (uint8_t)0xF2 || buf[5] != (uint8_t)0xFA) {
            continue;
        }
        cluster_id = (uint16_t)((uint16_t)buf[6] | ((uint16_t)buf[7] << 8u));
        counter    = buf[8];
        XcpTimecorr_HandleMulticast(cluster_id, counter);
    }
    return NULL;
}

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
    uint8_t sub_cmd = Xcp_GetByte(pdu, (uint8_t)1);

    if (sub_cmd == (uint8_t)0xFA) { /* GET_DAQ_CLOCK_MULTICAST */
        uint16_t cluster_id = Xcp_GetWord(pdu, (uint8_t)2);
        uint8_t  counter    = Xcp_GetByte(pdu, (uint8_t)4);
        XcpTimecorr_HandleMulticast(cluster_id, counter);
    }
}

void XcpTl_UpdateMulticastGroup(uint16_t cluster_id) {
    struct ip_mreq mreq;
    char           mcast_ip[32];
    unsigned char  mcast_upper = (unsigned char)((cluster_id >> 8u) & 0xFFu);
    unsigned char  mcast_lower = (unsigned char)(cluster_id & 0xFFu);

    if (XcpTl_Connection.multicastSocket < 0) {
        return;
    }
    snprintf(mcast_ip, sizeof(mcast_ip), "239.255.%u.%u", (unsigned)mcast_upper, (unsigned)mcast_lower);
    mreq.imr_multiaddr.s_addr = inet_addr(mcast_ip);
    mreq.imr_interface.s_addr = INADDR_ANY;
    setsockopt(XcpTl_Connection.multicastSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
}

#else /* !XCP_ENABLE_TIME_CORRELATION */

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
    (void)pdu;
}

void XcpTl_UpdateMulticastGroup(uint16_t cluster_id) {
    (void)cluster_id;
}

#endif /* XCP_ENABLE_TIME_CORRELATION */
