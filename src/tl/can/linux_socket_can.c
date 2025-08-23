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

#include <errno.h>
#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>
#include <memory.h>
#include <net/if.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_hw.h"
/*!!! END-INCLUDE-SECTION !!!*/

#define err_abort(code, text)                                                                                                      \
    do {                                                                                                                           \
        fprintf(stderr, "%s at \"%s\":%d: %s\n\r", text, __FILE__, __LINE__, strerror(code));                                      \
        abort();                                                                                                                   \
    } while (0)

#define errno_abort(text)                                                                                                          \
    do {                                                                                                                           \
        fprintf(stderr, "%s at \"%s\":%d: %s\n\r", text, __FILE__, __LINE__, strerror(errno));                                     \
        abort();                                                                                                                   \
    } while (0)

typedef struct tagXcpTl_ConnectionType {
    int  can_socket;
    SOCKET           boundSocket;
    bool connected;
} XcpTl_ConnectionType;

unsigned char buf[XCP_COMM_BUFLEN];

static XcpTl_ConnectionType XcpTl_Connection;

int locate_interface(int socket, char const * name) {
    struct ifreq ifr;

    strcpy(ifr.ifr_name, name);
    if (ioctl(socket, SIOCGIFINDEX, &ifr) == -1) {
        errno_abort("locate_interface::ioctl()");
    }

    return ifr.ifr_ifindex;
}

void XcpTl_Init(void) {
    int ret = 0;
    int sock = -1;
    struct sockaddr_can addr;
    struct ifreq ifr;

    memset(&XcpTl_Connection, 0, sizeof(XcpTl_ConnectionType));
    XcpTl_Connection.boundSocket = -1;

    /*---[ socket(PF_CAN, SOCK_RAW, CAN_RAW) ]---*/
    /* open socket */
    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) {
        XcpHw_ErrorMsg("XcpTl_Init::socket(PF_CAN)", errno);
        return;
    }

    /*---[ setsockopt(..., CAN_RAW_FILTER, ...) ]---*/
    /* setup filter */
    struct can_filter rfilter[2];
    rfilter[0].can_id = XCP_ON_CAN_INBOUND_IDENTIFIER;
    rfilter[0].can_mask = CAN_SFF_MASK;
    rfilter[1].can_id = XCP_ON_CAN_BROADCAST_IDENTIFIER;
    rfilter[1].can_mask = CAN_SFF_MASK;
    ret = setsockopt(sock, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
    if (ret < 0) {
        XcpHw_ErrorMsg("XcpTl_Init::setsockopt(CAN_RAW_FILTER)", errno);
        close(sock);
        return;
    }

    /*---[ setsockopt(..., CAN_RAW_ERR_FILTER, ...) ]---*/
    can_err_mask_t err_mask = (CAN_ERR_TX_TIMEOUT | CAN_ERR_LOSTARB | CAN_ERR_CRTL | CAN_ERR_PROT |
                                   CAN_ERR_TRX | CAN_ERR_ACK | CAN_ERR_BUSOFF | CAN_ERR_BUSERROR |
                                   CAN_ERR_RESTARTED | CAN_ERR_CNT);

    ret = setsockopt(sock, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));
    if (ret < 0) {
        XcpHw_ErrorMsg("XcpTl_Init::setsockopt(CAN_RAW_ERR_FILTER)", errno);
        close(sock);
        return;
    }

    /*---[ setsockopt(..., CAN_RAW_LOOPBACK, ...) ]---*/
    /* loopback */
    int loopback = 0; /* 0 = disabled, 1 = enabled */
    ret = setsockopt(sock, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));
    if (ret < 0) {
        XcpHw_ErrorMsg("XcpTl_Init::setsockopt(CAN_RAW_LOOPBACK)", errno);
        close(sock);
        return;
    }

    /*---[ setsockopt(..., CAN_RAW_RECV_OWN_MSGS, ...) ]---*/
    /* recv own messages */
    int recv_own_msgs = 0; /* 0 = disabled, 1 = enabled */
    ret = setsockopt(sock, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs, sizeof(recv_own_msgs));
    if (ret < 0) {
        XcpHw_ErrorMsg("XcpTl_Init::setsockopt(CAN_RAW_RECV_OWN_MSGS)", errno);
        close(sock);
        return;
    }

    /*---[ setsockopt(..., SO_BINDTODEVICE, ...) ]---*/
    /* interface */
    strncpy(ifr.ifr_name, Xcp_Options.can_device, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    ioctl(sock, SIOCGIFINDEX, &ifr);
    ret = setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &ifr.ifr_name, sizeof(ifr.ifr_name));
    if (ret < 0) {
        XcpHw_ErrorMsg("XcpTl_Init::setsockopt(SO_BINDTODEVICE)", errno);
        close(sock);
        return;
    }

    /*---[ bind(..., sockaddr_can, ...) ]---*/
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    memset(&addr.can_addr, 0, sizeof(addr.can_addr));
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        XcpHw_ErrorMsg("XcpTl_Init::bind(PF_CAN)", errno);
        close(sock);
        return;
    }

    XcpTl_Connection.boundSocket = sock;

    XcpTl_PrintConnectionInformation();
}

void XcpTl_DeInit(void) {
    if (XcpTl_Connection.boundSocket >= 0) {
        close(XcpTl_Connection.boundSocket);
        XcpTl_Connection.boundSocket = -1;
    }
}

void* XcpTl_Thread(void* param) {
    XCP_FOREVER {
        XcpTl_MainFunction();
    }
    return NULL;
}

void XcpTl_MainFunction(void) {
    if (XcpTl_FrameAvailable(0, 1000) > 0) {
        XcpTl_RxHandler();
    }
}

void XcpTl_RxHandler(void) {
    struct can_frame frame;
    for (;;) {
        ssize_t n = read(XcpTl_Connection.boundSocket, &frame, sizeof(frame));
        if (n < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                return;
            }
            XcpHw_ErrorMsg("XcpTl_RxHandler::read(CAN)", errno);
            return;
        }
        if ((size_t)n < sizeof(struct can_frame)) {
            XcpHw_ErrorMsg("XcpTl_RxHandler::read(CAN) short frame", EIO);
            return;
        }
        if (frame.can_dlc > 8U) {
            XcpHw_ErrorMsg("XcpTl_RxHandler: DLC > 8 not supported", EPROTO);
            continue;
        }
        if (frame.can_dlc > XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE) {
            XcpHw_ErrorMsg("XcpTl_RxHandler: DLC exceeds TL buffer", EOVERFLOW);
            continue;
        }

        Xcp_CtoIn.len = frame.can_dlc;
        XcpUtl_MemCopy(Xcp_CtoIn.data, frame.data, frame.can_dlc);
        Xcp_DispatchCommand(&Xcp_CtoIn);
    }
}

void XcpTl_TxHandler(void) {
}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec) {
#if 0
    struct timeval timeout;
    fd_set fds;
    int16_t res;

    timeout.tv_sec = sec;
    timeout.tv_usec = usec;

    FD_ZERO(&fds);
    FD_SET(XcpTl_Connection.can_socket, &fds);

    // Return value:
    // -1: error occurred
    // 0: timed out
    // > 0: data ready to be read
    res = select(0, &fds, 0, 0, &timeout);
    if (res != 0) {
        printf("sel: %d\r\n", res);
    }
    if (res == -1) {
        XcpHw_ErrorMsg("XcpTl_FrameAvailable:select()", errno);
        exit(2);
    }
    return res;
#endif
    return 1;
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    /* SocketCAN unterstützt max. 8 Bytes im klassischen CAN-Frame. */
    if (buf == NULL || len == 0) {
        return;
    }
    if (len > 8U) {
        XcpHw_ErrorMsg("XcpTl_Send: DLC > 8 not supported (classical CAN)", EINVAL);
        return;
    }

    struct can_frame frame;
    XCP_TL_ENTER_CRITICAL();
    memset(&frame, 0, sizeof(frame));
    /* Erwartet: erste 4/8 Bytes im PDU-Puffer sind die Nutzlast für CAN.
       An dieser Stelle wird nur sicher kopiert und gesendet. */
    frame.can_id = Xcp_Options.can_id; /* Annahme: ID kommt aus Optionen/Konfiguration. */
    frame.can_dlc = (uint8_t)len;
    memcpy(frame.data, buf, len);

    ssize_t n = write(XcpTl_Connection.boundSocket, &frame, sizeof(frame));
    if (n < 0) {
        XcpHw_ErrorMsg("XcpTl_Send::write(CAN)", errno);
    } else if ((size_t)n != sizeof(frame)) {
        XcpHw_ErrorMsg("XcpTl_Send::write(CAN) short write", EIO);
    }
    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_SaveConnection(void) {
    XcpTl_Connection.connected = XCP_TRUE;
}

void XcpTl_ReleaseConnection(void) {
    XcpTl_Connection.connected = XCP_FALSE;
}

bool XcpTl_VerifyConnection(void) {
    return XCP_TRUE;
}

void XcpTl_PrintConnectionInformation(void) {
    printf("\n\rXCPonCan\n\r");
#if 0
    printf("\nXCPonCan -- Listening on port %s / %s [%s]\n\r",
        DEFAULT_PORT,
        Xcp_Options.tcp ? "TCP" : "UDP",
        Xcp_Options.ipv6 ? "IPv6" : "IPv4"
    );
#endif
}
