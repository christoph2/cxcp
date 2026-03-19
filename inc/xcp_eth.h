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

#if !defined(__XCP_ETH_H)
    #define __XCP_ETH_H

    #if defined(_WIN32)
        #include <WinSock2.h>
        #include <Ws2tcpip.h>
    #elif defined(__unix__) || defined(__APPLE__)

        #include <arpa/inet.h>
        #include <errno.h>
        #include <fcntl.h>
        #include <netdb.h>
        #include <netinet/in.h>
        #include <signal.h>
        #include <stdbool.h>
        #include <string.h>
        #include <sys/socket.h>
        #include <sys/types.h>
        #include <sys/wait.h>
        #include <unistd.h>
    #endif

    #if defined(_WIN32)
typedef struct tagXcpTl_ConnectionType {
    SOCKADDR_STORAGE connectionAddress;
    SOCKADDR_STORAGE currentAddress;
    SOCKADDR_STORAGE localAddress;
    SOCKET           boundSocket;
    SOCKET           connectedSocket;
    SOCKET           multicastSocket;
    SOCKET           discoverySocket;
    bool             connected;
    int              socketType;
} XcpTl_ConnectionType;
    #elif defined(__unix__) || defined(__APPLE__)
typedef struct tagXcpTl_ConnectionType {
    struct sockaddr_storage connectionAddress;
    struct sockaddr_storage currentAddress;
    struct sockaddr_storage localAddress;
    int                     boundSocket;
    int                     connectedSocket;
    int                     multicastSocket;
    int                     discoverySocket;
    bool                    connected;
    int                     socketType;
} XcpTl_ConnectionType;
    #endif

    #if defined(__unix__) || defined(__APPLE__)
        #define SOCKET_ERROR     (-1)
        #define INVALID_SOCKET   (-1)
        #define ZeroMemory(b, l) memset((b), 0, (l))
    #endif

    #ifndef XCP_ENABLE_ETH_DISCOVERY
        #define XCP_ENABLE_ETH_DISCOVERY XCP_ON
    #endif
    #ifndef XCP_ETH_DISCOVERY_MCAST_IP0
        #define XCP_ETH_DISCOVERY_MCAST_IP0 (239u)
        #define XCP_ETH_DISCOVERY_MCAST_IP1 (255u)
        #define XCP_ETH_DISCOVERY_MCAST_IP2 (0u)
        #define XCP_ETH_DISCOVERY_MCAST_IP3 (0u)
    #endif
    #ifndef XCP_ETH_DISCOVERY_MCAST_PORT
        #define XCP_ETH_DISCOVERY_MCAST_PORT (5556u)
    #endif
    #ifndef XCP_ETH_DISCOVERY_MAC0
        #define XCP_ETH_DISCOVERY_MAC0 (0x00u)
        #define XCP_ETH_DISCOVERY_MAC1 (0x00u)
        #define XCP_ETH_DISCOVERY_MAC2 (0x00u)
        #define XCP_ETH_DISCOVERY_MAC3 (0x00u)
        #define XCP_ETH_DISCOVERY_MAC4 (0x00u)
        #define XCP_ETH_DISCOVERY_MAC5 (0x00u)
    #endif
    #ifndef XCP_ETH_DISCOVERY_SET_IP_STATUS
        #define XCP_ETH_DISCOVERY_SET_IP_STATUS (2u) /* 0=valid, 1=will activate, 2=manual action required */
    #endif

uint16_t XcpTl_GetLocalPort(void);
void     XcpTl_GetLocalIpv4(uint8_t out_ip[4]);
uint8_t  XcpTl_BuildStatus(bool extended);
void     XcpTl_SendUdpResponse(const char *mcast_ip, uint16_t port, uint8_t const *payload, size_t len);
bool     XcpTl_HandleTransportMulticastPacket(const uint8_t *buf, size_t len);

#endif /* __XCP_ETH_H */
