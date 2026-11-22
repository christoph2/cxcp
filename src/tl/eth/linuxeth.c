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

#include <net/if.h>
#include <poll.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <sys/ioctl.h>

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    #include "xcp_timecorr.h"
#endif /* XCP_ENABLE_TIME_CORRELATION */

static void *XcpTl_MulticastThread(void *param);

#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON) || (defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON))
static pthread_t     XcpTl_MulticastThreadHandle;
static int           XcpTl_MulticastWakePipe[2] = { -1, -1 };
static bool          XcpTl_MulticastThreadStarted;
static atomic_bool   XcpTl_MulticastStopRequested = ATOMIC_VAR_INIT(false);
#endif
static pthread_mutex_t XcpTl_MulticastSocketMutex = PTHREAD_MUTEX_INITIALIZER;
static bool            XcpTl_Initialized;

socklen_t addrSize = sizeof(struct sockaddr_storage);

extern XcpTl_ConnectionType XcpTl_Connection;

static bool Xcp_EnableSocketOption(int sock, int option);

static bool Xcp_DisableSocketOption(int sock, int option);

static bool XcpTl_ParseIpv4String(const char *ip, struct in_addr *out) {
    return inet_pton(AF_INET, ip, out) == 1;
}

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

    if (XcpTl_Initialized) {
        XcpTl_DeInit();
        if (XcpTl_Initialized) {
            return;
        }
    }
    XcpUtl_ZeroMem(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
    XcpTl_Connection.boundSocket      = -1;
    XcpTl_Connection.connectedSocket  = -1;
    XcpTl_Connection.discoverySocket  = -1;
    XcpTl_Connection.multicastSocket  = -1;
    XcpTl_Initialized                 = true;
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

    /* Discovery multicast listener (GET_SLAVE_ID / GET_SLAVE_ID_EXTENDED / SET_SLAVE_IP_ADDRESS) */
    XcpTl_Connection.discoverySocket = -1;
#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON)
    {
        int                dsock;
        struct sockaddr_in daddr;
        struct ip_mreq     dmreq;
        char               d_ip[32];
        int                reuse  = 1;
        int                optval = 1;

        snprintf(
            d_ip, sizeof(d_ip), "%u.%u.%u.%u", (unsigned)XCP_ETH_DISCOVERY_MCAST_IP0, (unsigned)XCP_ETH_DISCOVERY_MCAST_IP1,
            (unsigned)XCP_ETH_DISCOVERY_MCAST_IP2, (unsigned)XCP_ETH_DISCOVERY_MCAST_IP3
        );

        dsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (dsock >= 0) {
            setsockopt(dsock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
            setsockopt(dsock, IPPROTO_IP, IP_PKTINFO, (const char *)&optval, sizeof(optval));
            memset(&daddr, 0, sizeof(daddr));
            daddr.sin_family      = AF_INET;
            daddr.sin_port        = htons((uint16_t)XCP_ETH_DISCOVERY_MCAST_PORT);
            daddr.sin_addr.s_addr = INADDR_ANY;

            if (bind(dsock, (struct sockaddr *)&daddr, sizeof(daddr)) == 0) {
                if (!XcpTl_ParseIpv4String(d_ip, &dmreq.imr_multiaddr)) {
                    XcpHw_ErrorMsg("XcpTl_Init:invalid discovery mcast IP", EINVAL);
                    close(dsock);
                    dsock = -1;
                }
                dmreq.imr_interface.s_addr = INADDR_ANY;
                if ((dsock != -1) && setsockopt(dsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &dmreq, sizeof(dmreq)) == 0) {
                    setsockopt(dsock, IPPROTO_IP, IP_PKTINFO, &(const int){ 1 }, sizeof(int));
                    XcpTl_Connection.discoverySocket = dsock;
                    printf("XCPonEth -- Discovery multicast listener: %s:%u\n", d_ip, (unsigned)XCP_ETH_DISCOVERY_MCAST_PORT);
                } else {
                    XcpHw_ErrorMsg("XcpTl_Init:IP_ADD_MEMBERSHIP(discovery)", errno);
                    close(dsock);
                }
            } else {
                XcpHw_ErrorMsg("XcpTl_Init:discovery bind()", errno);
                close(dsock);
            }
        } else {
            XcpHw_ErrorMsg("XcpTl_Init:discovery socket()", errno);
        }
    }
#endif /* XCP_ENABLE_ETH_DISCOVERY */

    /* Create multicast socket for GET_DAQ_CLOCK_MULTICAST */
    XcpTl_Connection.multicastSocket = -1;
#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    {
        int                msock;
        struct sockaddr_in maddr;
        struct ip_mreq     mreq;
        uint16_t           cluster_id  = XcpTimecorr_GetClusterAffiliation();
        unsigned char      mcast_upper = (unsigned char)((cluster_id >> 8u) & 0xFFu);
        unsigned char      mcast_lower = (unsigned char)(cluster_id & 0xFFu);
        char               mcast_ip[32];
        int                reuse = 1;

        snprintf(mcast_ip, sizeof(mcast_ip), "239.255.%u.%u", (unsigned)mcast_upper, (unsigned)mcast_lower);

        msock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (msock >= 0) {
            setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

            memset(&maddr, 0, sizeof(maddr));
            maddr.sin_family      = AF_INET;
            maddr.sin_port        = htons(XCP_TIMECORR_MULTICAST_PORT);
            maddr.sin_addr.s_addr = INADDR_ANY;

            if (bind(msock, (struct sockaddr *)&maddr, sizeof(maddr)) == 0) {
                if (!XcpTl_ParseIpv4String(mcast_ip, &mreq.imr_multiaddr)) {
                    XcpHw_ErrorMsg("XcpTl_Init:invalid timecorr mcast IP", EINVAL);
                    close(msock);
                    msock = -1;
                }
                mreq.imr_interface.s_addr = INADDR_ANY;
                if ((msock != -1) && setsockopt(msock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == 0) {
                    setsockopt(msock, IPPROTO_IP, IP_PKTINFO, &(const int){ 1 }, sizeof(int));
                    XcpTl_Connection.multicastSocket = msock;
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

    if ((XcpTl_Connection.discoverySocket >= 0) || (XcpTl_Connection.multicastSocket >= 0)) {
#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON) || (defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON))
        int thread_error = 0;
        if (pipe(XcpTl_MulticastWakePipe) != 0) {
            XcpHw_ErrorMsg("XcpTl_Init:pipe(multicast wakeup)", errno);
            if (XcpTl_Connection.discoverySocket >= 0) {
                close(XcpTl_Connection.discoverySocket);
                XcpTl_Connection.discoverySocket = -1;
            }
            if (XcpTl_Connection.multicastSocket >= 0) {
                close(XcpTl_Connection.multicastSocket);
                XcpTl_Connection.multicastSocket = -1;
            }
        } else {
            atomic_store(&XcpTl_MulticastStopRequested, false);
            thread_error = pthread_create(&XcpTl_MulticastThreadHandle, NULL, XcpTl_MulticastThread, NULL);
            if (thread_error != 0) {
                XcpHw_ErrorMsg("XcpTl_Init:pthread_create(multicast)", thread_error);
                close(XcpTl_MulticastWakePipe[0]);
                close(XcpTl_MulticastWakePipe[1]);
                XcpTl_MulticastWakePipe[0] = -1;
                XcpTl_MulticastWakePipe[1] = -1;
                if (XcpTl_Connection.discoverySocket >= 0) {
                    close(XcpTl_Connection.discoverySocket);
                    XcpTl_Connection.discoverySocket = -1;
                }
                if (XcpTl_Connection.multicastSocket >= 0) {
                    close(XcpTl_Connection.multicastSocket);
                    XcpTl_Connection.multicastSocket = -1;
                }
            } else {
                XcpTl_MulticastThreadStarted = true;
            }
        }
#endif
    }
}

void XcpTl_DeInit(void) {
    if (!XcpTl_Initialized) {
        return;
    }
#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON) || (defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON))
    if (XcpTl_MulticastThreadStarted) {
        const uint8_t wake = 1u;
        int           thread_error;

        atomic_store(&XcpTl_MulticastStopRequested, true);
        if ((XcpTl_MulticastWakePipe[1] >= 0) && (write(XcpTl_MulticastWakePipe[1], &wake, sizeof(wake)) < 0)) {
            XcpHw_ErrorMsg("XcpTl_DeInit:write(multicast wakeup)", errno);
            close(XcpTl_MulticastWakePipe[1]);
            XcpTl_MulticastWakePipe[1] = -1;
        }
        thread_error = pthread_join(XcpTl_MulticastThreadHandle, NULL);
        if (thread_error != 0) {
            XcpHw_ErrorMsg("XcpTl_DeInit:pthread_join(multicast)", thread_error);
            return;
        }
        XcpTl_MulticastThreadStarted = false;
        atomic_store(&XcpTl_MulticastStopRequested, false);
    }
#endif
    pthread_mutex_lock(&XcpTl_MulticastSocketMutex);
    if (XcpTl_Connection.multicastSocket >= 0) {
        close(XcpTl_Connection.multicastSocket);
        XcpTl_Connection.multicastSocket = -1;
    }
    if (XcpTl_Connection.discoverySocket >= 0) {
        close(XcpTl_Connection.discoverySocket);
        XcpTl_Connection.discoverySocket = -1;
    }
    if (XcpTl_Connection.boundSocket >= 0) {
        close(XcpTl_Connection.boundSocket);
        XcpTl_Connection.boundSocket = -1;
    }
#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON) || (defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON))
    if (XcpTl_MulticastWakePipe[0] >= 0) {
        close(XcpTl_MulticastWakePipe[0]);
        XcpTl_MulticastWakePipe[0] = -1;
    }
    if (XcpTl_MulticastWakePipe[1] >= 0) {
        close(XcpTl_MulticastWakePipe[1]);
        XcpTl_MulticastWakePipe[1] = -1;
    }
#endif
    XcpTl_Initialized = false;
    pthread_mutex_unlock(&XcpTl_MulticastSocketMutex);
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

#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON) || (defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON))

static void get_interface_mac_address(int if_index, XcpTl_MacAddress *mac_address) {
    int          sock;
    struct ifreq ifr;
    char         ifname[IF_NAMESIZE];

    memset(mac_address, 0, sizeof(XcpTl_MacAddress));
    memset(&ifr, 0, sizeof(ifr));

    if (if_indextoname(if_index, ifname) == NULL) {
        return;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        return;
    }

    strncpy(ifr.ifr_name, ifname, IF_NAMESIZE - 1);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
        memcpy(mac_address->mac, ifr.ifr_hwaddr.sa_data, 6);
    }

    close(sock);
}

bool receive_message(
    int socket, char *buffer, size_t buffer_size, uint32_t *bytes_received, XcpTl_MacAddress *mac_address,
    uint8_t source_ipv4[4]
) {
    struct msghdr      msg;
    struct iovec       iov[1];
    struct sockaddr_in peer_addr;
    bool               result = false;
    char               cmbuf[0x100];
    ssize_t            n;

    *bytes_received = 0UL;
    memset(mac_address, 0, sizeof(*mac_address));
    memset(source_ipv4, 0, 4u);

    memset(&msg, 0, sizeof(msg));
    iov[0].iov_base = buffer;
    iov[0].iov_len  = buffer_size;
    msg.msg_iov     = iov;
    msg.msg_iovlen  = 1;

    msg.msg_name    = &peer_addr;
    msg.msg_namelen = sizeof(peer_addr);

    msg.msg_control    = cmbuf;
    msg.msg_controllen = sizeof(cmbuf);

    n = recvmsg(socket, &msg, 0);
    if (n > 0) {
        *bytes_received = (uint32_t)n;
        result          = true;
        if (peer_addr.sin_family == AF_INET) {
            memcpy(source_ipv4, &peer_addr.sin_addr.s_addr, 4u);
        }
        for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
                struct in_pktinfo *pi = (struct in_pktinfo *)CMSG_DATA(cmsg);
                get_interface_mac_address(pi->ipi_ifindex, mac_address);
                break;
            }
        }
    }
    return result;
}

/*
 * Thread that receives transport-layer multicast commands.
 * Packet layout (Intel byte order):
 *   [0..1]  len (WORD)
 *   [2..3]  transport counter (WORD)
 *   [4]     0xF2  (TRANSPORT_LAYER_CMD)
 *   [5]     sub-command (0xFA/0xFF/0xFD/0xFC)
 *   [6..]   payload per sub-command
 */
static void *XcpTl_MulticastThread(void *param) {
    char             buffer[1024];
    XcpTl_MacAddress mac_address;
    uint8_t          source_ipv4[4];
    uint32_t         bytes_received = 0UL;
    struct pollfd    fds[3];
    nfds_t           fds_count       = 1u;
    nfds_t           discovery_index = 0u;
    nfds_t           multicast_index = 0u;
    bool             has_discovery   = false;
    bool             has_multicast   = false;

    fds[0].fd      = XcpTl_MulticastWakePipe[0];
    fds[0].events  = POLLIN;
    fds[0].revents = 0;
    if (XcpTl_Connection.discoverySocket >= 0) {
        discovery_index = fds_count;
        fds[fds_count].fd      = XcpTl_Connection.discoverySocket;
        fds[fds_count].events  = POLLIN;
        fds[fds_count].revents = 0;
        ++fds_count;
        has_discovery = true;
    }
    if (XcpTl_Connection.multicastSocket >= 0) {
        multicast_index = fds_count;
        fds[fds_count].fd      = XcpTl_Connection.multicastSocket;
        fds[fds_count].events  = POLLIN;
        fds[fds_count].revents = 0;
        ++fds_count;
        has_multicast = true;
    }

    while (!atomic_load(&XcpTl_MulticastStopRequested)) {
        int ready;
        nfds_t idx;

        for (idx = 0u; idx < fds_count; ++idx) {
            fds[idx].revents = 0;
        }
        ready = poll(fds, fds_count, -1);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            XcpHw_ErrorMsg("XcpTl_MulticastThread:poll()", errno);
            break;
        }
        if ((fds[0].revents & POLLIN) != 0) {
            uint8_t wake_buffer[16];
            ssize_t wake_bytes = read(fds[0].fd, wake_buffer, sizeof(wake_buffer));
            if ((wake_bytes < 0) && (errno != EINTR)) {
                XcpHw_ErrorMsg("XcpTl_MulticastThread:read(wakeup)", errno);
                break;
            }
            if (atomic_load(&XcpTl_MulticastStopRequested)) {
                break;
            }
        }
        if ((fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
            break;
        }
        if (ready == 0) {
            continue;
        }
        if ((has_discovery && ((fds[discovery_index].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0)) ||
            (has_multicast && ((fds[multicast_index].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0))) {
            XcpHw_ErrorMsg("XcpTl_MulticastThread:poll socket error", EIO);
            break;
        }

        if (has_discovery && ((fds[discovery_index].revents & POLLIN) != 0)) {
            if (receive_message(
                    XcpTl_Connection.discoverySocket, buffer, sizeof(buffer), &bytes_received, &mac_address, source_ipv4
                )) {
                XcpTl_HandleTransportMulticastPacket((const uint8_t *)buffer, bytes_received, &mac_address, source_ipv4);
            }
        }
        if (has_multicast && ((fds[multicast_index].revents & POLLIN) != 0)) {
            if (receive_message(
                    XcpTl_Connection.multicastSocket, buffer, sizeof(buffer), &bytes_received, &mac_address, source_ipv4
                )) {
                XcpTl_HandleTransportMulticastPacket((const uint8_t *)buffer, bytes_received, &mac_address, source_ipv4);
            }
        }
    }
    return NULL;
}

#endif /* discovery/timecorr multicast listener */

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
    uint8_t sub_cmd = Xcp_GetByte(pdu, (uint8_t)1);

    if (sub_cmd == (uint8_t)0xFA) {
        /* GET_DAQ_CLOCK_MULTICAST */
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

    pthread_mutex_lock(&XcpTl_MulticastSocketMutex);
    if (XcpTl_Connection.multicastSocket < 0) {
        pthread_mutex_unlock(&XcpTl_MulticastSocketMutex);
        return;
    }
    snprintf(mcast_ip, sizeof(mcast_ip), "239.255.%u.%u", (unsigned)mcast_upper, (unsigned)mcast_lower);
    if (!XcpTl_ParseIpv4String(mcast_ip, &mreq.imr_multiaddr)) {
        XcpHw_ErrorMsg("XcpTl_UpdateMulticastGroup:invalid IP", EINVAL);
        pthread_mutex_unlock(&XcpTl_MulticastSocketMutex);
        return;
    }
    mreq.imr_interface.s_addr = INADDR_ANY;
    if (setsockopt(XcpTl_Connection.multicastSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        XcpHw_ErrorMsg("XcpTl_UpdateMulticastGroup:IP_ADD_MEMBERSHIP", errno);
    }
    pthread_mutex_unlock(&XcpTl_MulticastSocketMutex);
}

#else /* !XCP_ENABLE_TIME_CORRELATION */

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
    (void)pdu;
}

void XcpTl_UpdateMulticastGroup(uint16_t cluster_id) {
    (void)cluster_id;
}

#endif /* XCP_ENABLE_TIME_CORRELATION */
