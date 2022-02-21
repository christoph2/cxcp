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
#include <Mstcpip.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#elif defined(__unix__)
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
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
    SOCKET boundSocket;
    SOCKET connectedSocket;
    bool connected;
    int socketType;
} XcpTl_ConnectionType;
#elif defined(__unix__)
typedef struct tagXcpTl_ConnectionType {
    struct sockaddr_storage connectionAddress;
    struct sockaddr_storage currentAddress;
    struct sockaddr_storage localAddress;
    int boundSocket;
    int connectedSocket;
    bool connected;
    int socketType;
} XcpTl_ConnectionType;
#endif

#if defined(__unix__)
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (-1)
#define ZeroMemory(b, l) memset((b), 0, (l))
#endif

#endif /* __XCP_ETH_H */
