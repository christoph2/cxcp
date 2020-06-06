/*
 * BlueParrot XCP
 *
 * (C) 2007-2020 by Christoph Schueler <github.com/Christoph2,
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

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>


#include "xcp.h"
#include "xcp_hw.h"


#define err_abort(code,text) do { \
        fprintf (stderr, "%s at \"%s\":%d: %s\n", \
                        text, __FILE__, __LINE__, strerror (code)); \
        abort (); \
        } while (0)

#define errno_abort(text) do { \
        fprintf (stderr, "%s at \"%s\":%d: %s\n", \
                        text, __FILE__, __LINE__, strerror (errno)); \
        abort (); \
        } while (0)


typedef struct tagXcpTl_ConnectionType {
    uint32_t connectionAddress;
    uint32_t currentAddress;
    int can_socket;
    bool connected;
 } XcpTl_ConnectionType;


unsigned char buf[XCP_COMM_BUFLEN];
int addrSize = sizeof(SOCKADDR_STORAGE);


static XcpTl_ConnectionType XcpTl_Connection;
static XcpHw_OptionsType Xcp_Options;

static uint8_t Xcp_PduOutBuffer[XCP_MAX_CTO] = {0};


void Xcp_DispatchCommand(Xcp_PDUType const * const pdu);


extern Xcp_PDUType Xcp_PduIn;
extern Xcp_PDUType Xcp_PduOut;

static boolean Xcp_EnableSocketOption(SOCKET sock, int option);
static boolean Xcp_DisableSocketOption(SOCKET sock, int option);


static  boolean Xcp_EnableSocketOption(SOCKET sock, int option)
{
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


int locate_interface(int socket, char const * name)
{
    struct ifreq ifr;

    strcpy(ifr.ifr_name, name);
    if (ioctl(socket, SIOCGIFINDEX, &ifr) == -1) {
        errno_abort("locate_interface::ioctl()");
    }

    return ifr.ifr_ifindex;
}

void XcpTl_Init(void)
{
    int enable_sockopt = 1;
    struct ifreq ifr;
    struct sockaddr_can addr;
    struct can_frame frame;

    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
    Xcp_PduOut.data = &Xcp_PduOutBuffer[0];

    XcpTl_Connection.can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (XcpTl_Connection.can_socket== -1){
        errno_abort("XcpTl_Init::socket()");
    }
#if 0
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_sockopt, sizeof(enable_sockopt));
#endif

	if (setsockopt(XcpTl_Connection.can_socket, SOL_SOCKET, SO_TIMESTAMP, &enable_sockopt, sizeof(enable_sockopt)) < 0) {
        // Enable precision timestamps.
		errno_abort("setsockopt(SO_TIMESTAMP)");
		return 1;
	}


    /* Select that CAN interface, and bind the socket to it. */
    addr.can_family = AF_CAN;
    addr.can_ifindex = locate_interface(XcpTl_Connection.can_socket, "vcan0");
    bind(XcpTl_Connection.can_socket, (struct sockaddr*)&addr, sizeof(addr));
    if (XcpTl_Connection.can_socket== -1){
        errno_abort("XcpTl_Init::bind()");
    }
}


void XcpTl_DeInit(void)
{
    close(XcpTl_Connection.can_socket);
}


void XcpTl_MainFunction(void)
{
    if (XcpTl_FrameAvailable(0, 1000) > 0) {
        XcpTl_RxHandler();
    }
}


void XcpTl_RxHandler(void)
{
}


void XcpTl_TxHandler(void)
{

}


int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec)
{
    struct timeval timeout;
    fd_set fds;
    int16_t res;

    timeout.tv_sec = sec;
    timeout.tv_usec = usec;

    FD_ZERO(&fds);
    FD_SET(XcpTl_Connection.can_socket, &fds);

    res = select(0, &fds, 0, 0, &timeout);
    return res;
#if 0
//    if (((XcpTl_Connection.socketType == SOCK_STREAM) && (!XcpTl_Connection.connected)) || (XcpTl_Connection.socketType == SOCK_DGRAM)) {
        res = select(0, &fds, 0, 0, &timeout);
        if (res == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_FrameAvailable:select()", WSAGetLastError());
            exit(2);
        }
        return res;
    } else {
        return 1;
    }
#endif
}


void XcpTl_Send(uint8_t const * buf, uint16_t len)
{

}


void XcpTl_SaveConnection(void)
{
//    CopyMemory(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(SOCKADDR_STORAGE));
    XcpTl_Connection.connected = XCP_TRUE;
}


void XcpTl_ReleaseConnection(void)
{
    XcpTl_Connection.connected = XCP_FALSE;
}


bool XcpTl_VerifyConnection(void)
{
//    return memcmp(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(SOCKADDR_STORAGE)) == 0;
}

void XcpTl_SetOptions(XcpHw_OptionsType const * options)
{
    Xcp_Options = *options;
}

void XcpTl_PrintConnectionInformation(void)
{
#if 0
    printf("\nXCPonCan -- Listening on port %s / %s [%s]\n",
        DEFAULT_PORT,
        Xcp_Options.tcp ? "TCP" : "UDP",
        Xcp_Options.ipv6 ? "IPv6" : "IPv4"
    );
#endif
}


