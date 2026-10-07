/*
 * BlueParrot XCP
 *
 * (C) 2007-2026 by Christoph Schueler <github.com/Christoph2,
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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "esp_mac.h"
#include "esp_log.h"

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_eth.h"
#include "xcp_hw.h"
/*!!! END-INCLUDE-SECTION !!!*/

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
#include "xcp_timecorr.h"
#endif

static const char *TAG = "xcp-eth";
static socklen_t XcpTl_AddressLength = sizeof(struct sockaddr_storage);
static bool XcpTl_FirstTxLogged = false;
static bool XcpTl_FirstDiscoveryRxLogged = false;
static bool XcpTl_FirstDiscoveryCmdLogged = false;
static XcpTl_MacAddress XcpTl_LocalMacAddress;
static struct ip_mreq XcpTl_TimecorrMembership;
static bool XcpTl_TimecorrMembershipActive = false;
static struct ip_mreq XcpTl_DiscoveryMembership;
static bool XcpTl_DiscoveryMembershipActive = false;

#define XCP_ESP32_MCAST_THREAD_STACK_SIZE (8192U)

static bool XcpTl_ParseIpv4String(const char *ip, struct in_addr *out) {
    return inet_pton(AF_INET, ip, out) == 1;
}

static void XcpTl_LoadFallbackMac(XcpTl_MacAddress *mac_address) {
    mac_address->mac[0] = (uint8_t) XCP_ETH_DISCOVERY_MAC0;
    mac_address->mac[1] = (uint8_t) XCP_ETH_DISCOVERY_MAC1;
    mac_address->mac[2] = (uint8_t) XCP_ETH_DISCOVERY_MAC2;
    mac_address->mac[3] = (uint8_t) XCP_ETH_DISCOVERY_MAC3;
    mac_address->mac[4] = (uint8_t) XCP_ETH_DISCOVERY_MAC4;
    mac_address->mac[5] = (uint8_t) XCP_ETH_DISCOVERY_MAC5;
}

static void XcpTl_ReadLocalMacAddress(XcpTl_MacAddress *mac_address) {
	uint8_t idx=0;
    uint8_t raw_mac[6];
	const uint8_t MAC_TYPES[] = {
        ESP_MAC_WIFI_STA, ESP_MAC_BASE, ESP_MAC_WIFI_SOFTAP, ESP_MAC_EFUSE_FACTORY, ESP_MAC_EFUSE_CUSTOM,
    };

    XcpUtl_ZeroMem(raw_mac, (uint32_t)sizeof(raw_mac));
    XcpUtl_ZeroMem(mac_address, (uint32_t)sizeof(*mac_address));

	for (idx = 0; idx < XCP_ARRAY_SIZE(MAC_TYPES); ++idx) {
		if (esp_read_mac(raw_mac, MAC_TYPES[idx]) == ESP_OK) {
			
			// printf("ESP32 MAC Address: [%u]", idx);			
			// printf("%02X:%02X:%02X:%02X:%02X:%02X\n", raw_mac[0], raw_mac[1], raw_mac[2], raw_mac[3], raw_mac[4], raw_mac[5]);
			
			XcpUtl_MemCopy(mac_address->mac, raw_mac, sizeof(raw_mac));
			return;
		}
	}
	// printf("MAC NOT found - using fallback...\n");
    XcpTl_LoadFallbackMac(mac_address);
}

static void XcpTl_LeaveMembershipIfNeeded(int sock, struct ip_mreq *membership, bool *active) {
    if ((sock == INVALID_SOCKET) || (*active == false)) {
        return;
    }
    (void) setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, membership, sizeof(*membership));
    *active = false;
}

#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON) || (defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON))
static void *XcpTl_MulticastThread(void *param);
#endif

int gethostname(char *name, size_t len) {
    static const char fallback_name[] = "esp32-xcp";
    size_t n;

    if ((name == NULL) || (len == 0U)) {
        errno = EINVAL;
        return -1;
    }
    n = strlen(fallback_name);
    if (n >= len) {
        n = len - 1U;
    }
    memcpy(name, fallback_name, n);
    name[n] = '\0';
    return 0;
}

extern XcpTl_ConnectionType XcpTl_Connection;

void XcpTl_Init(void) {
    int sock;
    int dsock;
    int msock;
    int reuse = 1;
    struct timeval timeout;
    struct sockaddr_storage bind_addr;

    XcpUtl_ZeroMem(&XcpTl_Connection, (uint32_t)sizeof(XcpTl_Connection));
    XcpTl_Connection.boundSocket = INVALID_SOCKET;
    XcpTl_Connection.connectedSocket = INVALID_SOCKET;
    XcpTl_Connection.discoverySocket = INVALID_SOCKET;
    XcpTl_Connection.multicastSocket = INVALID_SOCKET;
    XcpTl_Connection.socketType = SOCK_DGRAM;
    XcpTl_Connection.connected = XCP_FALSE;
    XcpTl_FirstTxLogged = false;
    XcpTl_FirstDiscoveryRxLogged = false;
    XcpTl_FirstDiscoveryCmdLogged = false;

    XcpUtl_ZeroMem(&XcpTl_LocalMacAddress, (uint32_t)sizeof(XcpTl_LocalMacAddress));
    XcpUtl_ZeroMem(&XcpTl_TimecorrMembership, (uint32_t)sizeof(XcpTl_TimecorrMembership));
    XcpUtl_ZeroMem(&XcpTl_DiscoveryMembership, (uint32_t)sizeof(XcpTl_DiscoveryMembership));
    XcpTl_TimecorrMembershipActive = false;
    XcpTl_DiscoveryMembershipActive = false;

    XcpTl_ReadLocalMacAddress(&XcpTl_LocalMacAddress);

    XcpUtl_ZeroMem(&bind_addr, (uint32_t)sizeof(bind_addr));
#if defined(CONFIG_EXAMPLE_IPV6)
    {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) &bind_addr;

        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons((uint16_t) XCP_ON_ETHERNET_PORT);
        addr6->sin6_addr = in6addr_any;
        XcpTl_AddressLength = (socklen_t) sizeof(struct sockaddr_in6);
    }
    sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
#else
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *) &bind_addr;

        addr4->sin_family = AF_INET;
        addr4->sin_port = htons((uint16_t) XCP_ON_ETHERNET_PORT);
        addr4->sin_addr.s_addr = htonl(INADDR_ANY);
        XcpTl_AddressLength = (socklen_t) sizeof(struct sockaddr_in);
    }
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif

    if (sock == INVALID_SOCKET) {
        XcpHw_ErrorMsg("XcpTl_Init:socket()", errno);
        return;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        XcpHw_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEADDR)", errno);
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        XcpHw_ErrorMsg("XcpTl_Init:setsockopt(SO_RCVTIMEO)", errno);
    }

    if (bind(sock, (struct sockaddr *) &bind_addr, XcpTl_AddressLength) < 0) {
        XcpHw_ErrorMsg("XcpTl_Init:bind()", errno);
        close(sock);
        return;
    }

    XcpTl_Connection.boundSocket = sock;
    XcpUtl_MemCopy(&XcpTl_Connection.localAddress, &bind_addr, sizeof(struct sockaddr_storage));

    XcpTl_Connection.discoverySocket = INVALID_SOCKET;
#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON)
    {
        struct sockaddr_in daddr;
        struct ip_mreq dmreq;
        char d_ip[32];

        (void) snprintf(
            d_ip, sizeof(d_ip), "%u.%u.%u.%u", (unsigned) XCP_ETH_DISCOVERY_MCAST_IP0,
            (unsigned) XCP_ETH_DISCOVERY_MCAST_IP1,
            (unsigned) XCP_ETH_DISCOVERY_MCAST_IP2, (unsigned) XCP_ETH_DISCOVERY_MCAST_IP3
        );

        dsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (dsock == INVALID_SOCKET) {
            XcpHw_ErrorMsg("XcpTl_Init:discovery socket()", errno);
        } else {
            (void) setsockopt(dsock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
            XcpUtl_ZeroMem(&daddr, (uint32_t)sizeof(daddr));
            daddr.sin_family = AF_INET;
            daddr.sin_port = htons((uint16_t) XCP_ETH_DISCOVERY_MCAST_PORT);
            daddr.sin_addr.s_addr = htonl(INADDR_ANY);

            if (bind(dsock, (struct sockaddr *) &daddr, sizeof(daddr)) < 0) {
                XcpHw_ErrorMsg("XcpTl_Init:discovery bind()", errno);
                close(dsock);
            } else if (!XcpTl_ParseIpv4String(d_ip, &dmreq.imr_multiaddr)) {
                XcpHw_ErrorMsg("XcpTl_Init:invalid discovery mcast IP", EINVAL);
                close(dsock);
            } else {
                dmreq.imr_interface.s_addr = htonl(INADDR_ANY);
                if (setsockopt(dsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &dmreq, sizeof(dmreq)) < 0) {
                    XcpHw_ErrorMsg("XcpTl_Init:IP_ADD_MEMBERSHIP(discovery)", errno);
                    close(dsock);
                } else {
                    XcpUtl_MemCopy(&XcpTl_DiscoveryMembership, &dmreq, sizeof(dmreq));
                    XcpTl_DiscoveryMembershipActive = true;
                    XcpTl_Connection.discoverySocket = dsock;
                    ESP_LOGI(TAG, "Discovery multicast listener: %s:%u", d_ip, (unsigned) XCP_ETH_DISCOVERY_MCAST_PORT);
                }
            }
        }
    }
#endif

    XcpTl_Connection.multicastSocket = INVALID_SOCKET;
#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    {
        struct sockaddr_in maddr;
        struct ip_mreq mreq;
        uint16_t cluster_id = XcpTimecorr_GetClusterAffiliation();
        unsigned char mcast_upper = (unsigned char) ((cluster_id >> 8u) & 0xFFu);
        unsigned char mcast_lower = (unsigned char) (cluster_id & 0xFFu);
        char mcast_ip[32];

        (void) snprintf(mcast_ip, sizeof(mcast_ip), "239.255.%u.%u", (unsigned) mcast_upper, (unsigned) mcast_lower);

        msock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (msock == INVALID_SOCKET) {
            XcpHw_ErrorMsg("XcpTl_Init:multicast socket()", errno);
        } else {
            (void) setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
            XcpUtl_ZeroMem(&maddr, (uint32_t)sizeof(maddr));
            maddr.sin_family = AF_INET;
            maddr.sin_port = htons((uint16_t) XCP_TIMECORR_MULTICAST_PORT);
            maddr.sin_addr.s_addr = htonl(INADDR_ANY);

            if (bind(msock, (struct sockaddr *) &maddr, sizeof(maddr)) < 0) {
                XcpHw_ErrorMsg("XcpTl_Init:multicast bind()", errno);
                close(msock);
            } else if (!XcpTl_ParseIpv4String(mcast_ip, &mreq.imr_multiaddr)) {
                XcpHw_ErrorMsg("XcpTl_Init:invalid timecorr mcast IP", EINVAL);
                close(msock);
            } else {
                mreq.imr_interface.s_addr = htonl(INADDR_ANY);
                if (setsockopt(msock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                    XcpHw_ErrorMsg("XcpTl_Init:IP_ADD_MEMBERSHIP(timecorr)", errno);
                    close(msock);
                } else {
                    XcpUtl_MemCopy(&XcpTl_TimecorrMembership, &mreq, sizeof(mreq));
                    XcpTl_TimecorrMembershipActive = true;
                    XcpTl_Connection.multicastSocket = msock;
                    ESP_LOGI(TAG, "Timecorr multicast listener: %s:%u", mcast_ip,
                             (unsigned) XCP_TIMECORR_MULTICAST_PORT);
                }
            }
        }
    }
#endif

#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON) || (defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON))
    if ((XcpTl_Connection.discoverySocket != INVALID_SOCKET) || (XcpTl_Connection.multicastSocket != INVALID_SOCKET)) {
        pthread_t mcast_thread;
        pthread_attr_t attr;
        int attr_inited = 0;
        int attr_ok = 0;

        if (pthread_attr_init(&attr) == 0) {
            attr_inited = 1;
            if (pthread_attr_setstacksize(&attr, (size_t) XCP_ESP32_MCAST_THREAD_STACK_SIZE) == 0) {
                attr_ok = 1;
            }
        }

        if (pthread_create(&mcast_thread, attr_ok ? &attr : NULL, XcpTl_MulticastThread, NULL) != 0) {
            XcpHw_ErrorMsg("XcpTl_Init:pthread_create(multicast)", errno);
        } else {
            (void) pthread_detach(mcast_thread);
        }

        if (attr_inited) {
            (void) pthread_attr_destroy(&attr);
        }
    }
#endif

    ESP_LOGI(TAG, "XCP UDP socket bound on port %u", (unsigned) XCP_ON_ETHERNET_PORT);
    XcpTl_PrintConnectionInformation();
}

void XcpTl_DeInit(void) {
    if (XcpTl_Connection.multicastSocket != INVALID_SOCKET) {
        XcpTl_LeaveMembershipIfNeeded(
            XcpTl_Connection.multicastSocket, &XcpTl_TimecorrMembership, &XcpTl_TimecorrMembershipActive
        );
        close(XcpTl_Connection.multicastSocket);
        XcpTl_Connection.multicastSocket = INVALID_SOCKET;
    }
    if (XcpTl_Connection.discoverySocket != INVALID_SOCKET) {
        XcpTl_LeaveMembershipIfNeeded(
            XcpTl_Connection.discoverySocket, &XcpTl_DiscoveryMembership, &XcpTl_DiscoveryMembershipActive
        );
        close(XcpTl_Connection.discoverySocket);
        XcpTl_Connection.discoverySocket = INVALID_SOCKET;
    }
    if (XcpTl_Connection.connectedSocket != INVALID_SOCKET) {
        close(XcpTl_Connection.connectedSocket);
        XcpTl_Connection.connectedSocket = INVALID_SOCKET;
    }
    if (XcpTl_Connection.boundSocket != INVALID_SOCKET) {
        close(XcpTl_Connection.boundSocket);
        XcpTl_Connection.boundSocket = INVALID_SOCKET;
    }
    XcpTl_Connection.connected = XCP_FALSE;
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    socklen_t addr_len;

    if ((XcpTl_Connection.boundSocket == INVALID_SOCKET) || (XcpTl_Connection.connected == XCP_FALSE)) {
        return;
    }

    addr_len = XcpTl_AddressLength;
    if (((struct sockaddr *) &XcpTl_Connection.connectionAddress)->sa_family == AF_INET6) {
        addr_len = (socklen_t) sizeof(struct sockaddr_in6);
    } else if (((struct sockaddr *) &XcpTl_Connection.connectionAddress)->sa_family == AF_INET) {
        addr_len = (socklen_t) sizeof(struct sockaddr_in);
    }

    XCP_TL_ENTER_CRITICAL();
    if (sendto(
            XcpTl_Connection.boundSocket, (char const *) buf, len, 0,
            (struct sockaddr const *) &XcpTl_Connection.connectionAddress, addr_len
        ) < 0) {
        XcpHw_ErrorMsg("XcpTl_Send:sendto()", errno);
    } else if (!XcpTl_FirstTxLogged) {
        XcpTl_FirstTxLogged = true;
        ESP_LOGI(TAG, "First XCP UDP response sent (%u bytes)", (unsigned) len);
    }
    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const *const pdu) {
#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    uint8_t sub_cmd = Xcp_GetByte(pdu, (uint8_t) 1);

    if (sub_cmd == (uint8_t) 0xFA) {
        uint16_t cluster_id = Xcp_GetWord(pdu, (uint8_t) 2);
        uint8_t counter = Xcp_GetByte(pdu, (uint8_t) 4);

        XcpTimecorr_HandleMulticast(cluster_id, counter);
    }
#else
    (void) pdu;
#endif
}

void XcpTl_UpdateMulticastGroup(uint16_t cluster_id) {
#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    struct ip_mreq mreq;
    char mcast_ip[32];
    unsigned char mcast_upper = (unsigned char) ((cluster_id >> 8u) & 0xFFu);
    unsigned char mcast_lower = (unsigned char) (cluster_id & 0xFFu);

    if (XcpTl_Connection.multicastSocket == INVALID_SOCKET) {
        return;
    }

    (void) snprintf(mcast_ip, sizeof(mcast_ip), "239.255.%u.%u", (unsigned) mcast_upper, (unsigned) mcast_lower);
    if (!XcpTl_ParseIpv4String(mcast_ip, &mreq.imr_multiaddr)) {
        XcpHw_ErrorMsg("XcpTl_UpdateMulticastGroup:invalid IP", EINVAL);
        return;
    }
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    XcpTl_LeaveMembershipIfNeeded(
        XcpTl_Connection.multicastSocket, &XcpTl_TimecorrMembership, &XcpTl_TimecorrMembershipActive
    );

    if (setsockopt(XcpTl_Connection.multicastSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        XcpHw_ErrorMsg("XcpTl_UpdateMulticastGroup:IP_ADD_MEMBERSHIP", errno);
    } else {
        XcpUtl_MemCopy(&XcpTl_TimecorrMembership, &mreq, sizeof(mreq));
        XcpTl_TimecorrMembershipActive = true;
    }
#else
    (void) cluster_id;
#endif
}

#if (XCP_ENABLE_ETH_DISCOVERY == XCP_ON) || (defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON))
static void *XcpTl_MulticastThread(void *param) {
    char buffer[1024];
    XcpTl_MacAddress mac_address;

    (void) param;

    XcpUtl_ZeroMem(&mac_address, (uint32_t)sizeof(mac_address));
    XcpUtl_ZeroMem(buffer, (uint32_t)sizeof(buffer));

    for (;;) {
        fd_set rfds;
        int max_fd = -1;
        int ready;

        FD_ZERO(&rfds);
        if (XcpTl_Connection.discoverySocket != INVALID_SOCKET) {
            FD_SET(XcpTl_Connection.discoverySocket, &rfds);
            if (XcpTl_Connection.discoverySocket > max_fd) {
                max_fd = XcpTl_Connection.discoverySocket;
            }
        }
        if (XcpTl_Connection.multicastSocket != INVALID_SOCKET) {
            FD_SET(XcpTl_Connection.multicastSocket, &rfds);
            if (XcpTl_Connection.multicastSocket > max_fd) {
                max_fd = XcpTl_Connection.multicastSocket;
            }
        }
        if ((XcpTl_Connection.discoverySocket == INVALID_SOCKET) && (
                XcpTl_Connection.multicastSocket == INVALID_SOCKET)) {
            break;
        }

        ready = select(max_fd + 1, &rfds, NULL, NULL, NULL);
        if (ready <= 0) {
            continue;
        }

        if ((XcpTl_Connection.discoverySocket != INVALID_SOCKET) && FD_ISSET(XcpTl_Connection.discoverySocket, &rfds)) {
            uint32_t bytes_received = 0U;
            struct sockaddr_storage peer_addr;
            socklen_t peer_len = (socklen_t) sizeof(peer_addr);
            ssize_t n;

            n = recvfrom(
                XcpTl_Connection.discoverySocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &peer_addr, &peer_len
            );
            if (n > 0) {
                if (!XcpTl_FirstDiscoveryRxLogged) {
                    XcpTl_FirstDiscoveryRxLogged = true;
                    if (((struct sockaddr *) &peer_addr)->sa_family == AF_INET) {
                        const struct sockaddr_in *peer4 = (const struct sockaddr_in *) &peer_addr;
                        ESP_LOGI(
                            TAG, "First discovery packet received from %s:%u", inet_ntoa(peer4->sin_addr),
                            (unsigned) ntohs(peer4->sin_port)
                        );
                    } else {
                        ESP_LOGI(TAG, "First discovery packet received (%d bytes)", (int) n);
                    }
                }

                bytes_received = (uint32_t) n;

                if (!XcpTl_FirstDiscoveryCmdLogged && (n > 5) && (((uint8_t) buffer[4]) == (uint8_t) 0xF2)) {
                    XcpTl_FirstDiscoveryCmdLogged = true;
                    ESP_LOGI(TAG, "Discovery self-test: TL cmd 0x%02X received", (unsigned) ((uint8_t) buffer[5]));
                }

                XcpUtl_MemCopy(&mac_address, &XcpTl_LocalMacAddress, sizeof(mac_address));
                
                (void) XcpTl_HandleTransportMulticastPacket((const uint8_t *) buffer, bytes_received, &mac_address);
            }
        }
        if ((XcpTl_Connection.multicastSocket != INVALID_SOCKET) && FD_ISSET(XcpTl_Connection.multicastSocket, &rfds)) {
            uint32_t bytes_received = 0U;
            struct sockaddr_storage peer_addr;
            socklen_t peer_len = (socklen_t) sizeof(peer_addr);
            ssize_t n;

            n = recvfrom(
                XcpTl_Connection.multicastSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &peer_addr, &peer_len
            );
            if (n > 0) {
                bytes_received = (uint32_t) n;
                XcpUtl_MemCopy(&mac_address, &XcpTl_LocalMacAddress, sizeof(mac_address));
                (void) XcpTl_HandleTransportMulticastPacket((const uint8_t *) buffer, bytes_received, &mac_address);
            }
        }
    }
    return NULL;
}
#endif
