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
        endwin(); \
        fprintf (stderr, "%s at \"%s\":%d: %s\n", \
                        text, __FILE__, __LINE__, strerror (code)); \
        abort (); \
        } while (0)

#define errno_abort(text) do { \
        endwin(); \
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

static XcpTl_ConnectionType XcpTl_Connection;

extern void endwin(void);

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
    struct sockaddr_can addr;
    struct can_filter rfilter[2];


    memset(&XcpTl_Connection, '\x00', sizeof(XcpTl_ConnectionType));

    XcpTl_Connection.can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (XcpTl_Connection.can_socket== -1){
        errno_abort("XcpTl_Init::socket()");
    }
    if (Xcp_Options.fd) {
    	if (setsockopt(XcpTl_Connection.can_socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_sockopt, sizeof(enable_sockopt)) == -1) {
            errno_abort("Your kernel doesn't supports CAN-FD.\n");
        }
    }
	if (setsockopt(XcpTl_Connection.can_socket, SOL_SOCKET, SO_TIMESTAMP, &enable_sockopt, sizeof(enable_sockopt)) < 0) {
        // Enable precision timestamps.
		errno_abort("setsockopt(SO_TIMESTAMP)");
	}

    /* Select that CAN interface, and bind the socket to it. */
    addr.can_family = AF_CAN;
    addr.can_ifindex = locate_interface(XcpTl_Connection.can_socket, Xcp_Options.interface);
    bind(XcpTl_Connection.can_socket, (struct sockaddr*)&addr, sizeof(addr));
    if (XcpTl_Connection.can_socket== -1){
        errno_abort("XcpTl_Init::bind()");
    }

    rfilter[0].can_id   = XCP_ON_CAN_INBOUND_IDENTIFIER;
    rfilter[0].can_mask = XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER) ? CAN_EFF_FLAG :  CAN_SFF_MASK;
    rfilter[1].can_id   = XCP_ON_CAN_BROADCAST_IDENTIFIER;
    rfilter[1].can_mask = XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_BROADCAST_IDENTIFIER) ? CAN_EFF_FLAG :  CAN_SFF_MASK;;
    setsockopt(XcpTl_Connection.can_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
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
    struct canfd_frame frame;
    int nbytes = 0;

    nbytes = read(XcpTl_Connection.can_socket, &frame, CANFD_MTU);

    if (nbytes == CANFD_MTU) {
        //printf("got CAN FD frame with length %d\n", frame.len);
        /* FD frame.flags could be used here */
    } else if (nbytes == CAN_MTU) {
        //printf("got legacy CAN frame with length %d\n", frame.len);
    } else {
        errno_abort("can raw socket read");
    }
    //printf("#%d bytes received from '%04x' DLC: %d .\n", nbytes, frame.can_id, frame.can_dlc);
    if (frame.len > 0) {
        Xcp_CtoIn.len = frame.len;
        Xcp_CtoIn.data = (__u8*)&frame.data + XCP_TRANSPORT_LAYER_BUFFER_OFFSET;
        Xcp_DispatchCommand(&Xcp_CtoIn);
    }
}


void XcpTl_TxHandler(void)
{

}


int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec)
{
    return 1;
}


void XcpTl_Send(uint8_t const * buf, uint16_t len)
{
    struct can_frame frame = {0};
    struct canfd_frame frame_fd = {0};

    XCP_TL_ENTER_CRITICAL();
    if (Xcp_Options.fd) {
        frame_fd.len = len;
        frame_fd.can_id = XCP_ON_CAN_OUTBOUND_IDENTIFIER;
        memcpy(frame_fd.data, buf, len);
        (void)write(XcpTl_Connection.can_socket, &frame_fd, sizeof(struct canfd_frame));
    } else {
        frame.can_dlc = len;
        frame.can_id = XCP_ON_CAN_OUTBOUND_IDENTIFIER;
        memcpy(frame.data, buf, len);
        (void)write(XcpTl_Connection.can_socket, &frame, sizeof(struct can_frame));
    }
    XCP_TL_LEAVE_CRITICAL();
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
    return XCP_TRUE;
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


