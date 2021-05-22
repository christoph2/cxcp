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
    #define SOCKET_ERROR    (-1)
#endif


XcpTl_ConnectionType XcpTl_Connection;


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
    if (XcpTl_FrameAvailable(0, 1000) > 0) {
        XcpTl_RxHandler();
    }
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec)
{
    struct timeval timeout;
    fd_set fds;
    int16_t res;

    timeout.tv_sec = sec;
    timeout.tv_usec = usec;

    FD_ZERO(&fds);
    FD_SET(XcpTl_Connection.boundSocket, &fds);

    // Return value:
    // -1: error occurred
    // 0: timed out
    // > 0: data ready to be read
    if (((XcpTl_Connection.socketType == SOCK_STREAM) && (!XcpTl_Connection.connected)) || (XcpTl_Connection.socketType == SOCK_DGRAM)) {
        res = select(0, &fds, 0, 0, &timeout);
        if (res == SOCKET_ERROR) {
            XcpHw_ErrorMsg("XcpTl_FrameAvailable:select()", WSAGetLastError());
            exit(2);
        }
        return res;
    } else {
        return 1;
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
    printf("XCPonEth -- Listening on port %s / %s [%s]\n\r",
        XCP_ETH_DEFAULT_PORT,
        Xcp_Options.tcp ? "TCP" : "UDP",
        Xcp_Options.ipv6 ? "IPv6" : "IPv4"
    );
}

