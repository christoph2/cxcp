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

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_eth.h"
#include "xcp_hw.h"
/*!!! END-INCLUDE-SECTION !!!*/

#define DEFAULT_FAMILY PF_UNSPEC     // Accept either IPv4 or IPv6
#define DEFAULT_SOCKTYPE SOCK_STREAM //

#define ADDR_LEN sizeof(SOCKADDR_STORAGE)

extern XcpTl_ConnectionType XcpTl_Connection;

void Xcp_DispatchCommand(Xcp_PduType const *const pdu);

extern Xcp_PduType Xcp_CtoIn;
extern Xcp_PduType Xcp_CtoOut;

static boolean Xcp_EnableSocketOption(SOCKET sock, int option);

#if 0
static boolean Xcp_DisableSocketOption(SOCKET sock, int option);
#endif

static boolean Xcp_EnableSocketOption(SOCKET sock, int option) {
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
  const char enable = 1;

  if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
    return XCP_FALSE;
  }
#else
  if (setsockopt(sock, SOL_SOCKET, option, &(const char){1}, sizeof(int)) < 0) {
    return XCP_FALSE;
  }
#endif
  return XCP_TRUE;
}

#if 0
static boolean Xcp_DisableSocketOption(SOCKET sock, int option)
{
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const char enable = 0;

    if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
        return XCP_FALSE;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){0}, sizeof(int)) < 0) {
        return XCP_FALSE;
    }
#endif
    return XCP_TRUE;
}
#endif

void XcpTl_Init(void) {
  WSADATA wsa;
  ADDRINFO Hints;
  ADDRINFO *AddrInfo;
  ADDRINFO *AI;
  char *Address = NULL;
  char *Port = "5555";
  SOCKET serverSockets[FD_SETSIZE];
  int boundSocketNum = -1;
  int ret;
  int idx;
  DWORD dwTimeAdjustment = 0UL;
  DWORD dwTimeIncrement = 0UL;
  BOOL fAdjustmentDisabled = XCP_TRUE;
  /* unsigned long ul = 1; */

  ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
  memset(&Hints, 0, sizeof(Hints));
  GetSystemTimeAdjustment(&dwTimeAdjustment, &dwTimeIncrement,
                          &fAdjustmentDisabled);
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    XcpHw_ErrorMsg("XcpTl_Init:WSAStartup()", WSAGetLastError());
    exit(EXIT_FAILURE);
  } else {
  }
  XcpTl_Connection.socketType = Xcp_Options.tcp ? SOCK_STREAM : SOCK_DGRAM;
  Hints.ai_family = Xcp_Options.ipv6 ? PF_INET6 : PF_INET;
  Hints.ai_socktype = XcpTl_Connection.socketType;
  Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
  ret = getaddrinfo(Address, Port, &Hints, &AddrInfo);
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
    serverSockets[idx] =
        socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);
    if (serverSockets[idx] == INVALID_SOCKET) {
      XcpHw_ErrorMsg("XcpTl_Init::socket()", WSAGetLastError());
      continue;
    }
    if (bind(serverSockets[idx], AI->ai_addr, AI->ai_addrlen) != 0) {
      XcpHw_ErrorMsg("XcpTl_Init::bind()", WSAGetLastError());
      continue;
    }

    memcpy(&XcpTl_Connection.localAddress, AI->ai_addr,
           sizeof(SOCKADDR_STORAGE));
    // XcpTl_Connection.localAddress = *AI->ai_addr;

    if (XcpTl_Connection.socketType == SOCK_STREAM) {
      if (listen(serverSockets[idx], 1) == SOCKET_ERROR) {
        XcpHw_ErrorMsg("XcpTl_Init::listen()", WSAGetLastError());
        continue;
      }
    }
    boundSocketNum = idx;
    XcpTl_Connection.boundSocket = serverSockets[boundSocketNum];
    /* ioctlsocket(XcpTl_Connection.boundSocket, FIONBIO, &ul); */
    break; /* Grab first address. */
  }
  freeaddrinfo(AddrInfo);
  if (boundSocketNum == -1) {
    fprintf(stderr,
            "Fatal error: unable to serve on any address.\nPerhaps"
            " a server is already running on port %u / %s [%s]?\n",
            XCP_ETH_DEFAULT_PORT, Xcp_Options.tcp ? "TCP" : "UDP",
            Xcp_Options.ipv6 ? "IPv6" : "IPv4");
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
}

void XcpTl_DeInit(void) {
  closesocket(XcpTl_Connection.boundSocket);
  WSACleanup();
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
  XCP_TL_ENTER_CRITICAL();
  if (XcpTl_Connection.socketType == SOCK_DGRAM) {
    if (sendto(XcpTl_Connection.boundSocket, (char const *)buf, len, 0,
               (SOCKADDR const *)(SOCKADDR_STORAGE const *)&XcpTl_Connection
                   .connectionAddress,
               ADDR_LEN) == SOCKET_ERROR) {
      XcpHw_ErrorMsg("XcpTl_Send:sendto()", WSAGetLastError());
    }
  } else if (XcpTl_Connection.socketType == SOCK_STREAM) {
    if (send(XcpTl_Connection.connectedSocket, (char const *)buf, len, 0) ==
        SOCKET_ERROR) {
      XcpHw_ErrorMsg("XcpTl_Send:send()", WSAGetLastError());
      closesocket(XcpTl_Connection.connectedSocket);
    }
  }
  XCP_TL_LEAVE_CRITICAL();
}
