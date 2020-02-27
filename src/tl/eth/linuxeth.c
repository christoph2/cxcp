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

#include "xcp.h"
#include "xcp_hw.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

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

#define BUFFER_SIZE 1024

#define XCP_COMM_PORT    (5555)

#define DEFAULT_FAMILY     PF_UNSPEC // Accept either IPv4 or IPv6
#define DEFAULT_SOCKTYPE   SOCK_STREAM
#define DEFAULT_PORT       "5555"

#define EVENT_TABLE_SIZE    (8)
#define MAX_EVENT_NUMBER    (1024)


typedef struct tagXcpTl_ConnectionType {
    struct sockaddr_storage connectionAddress;
    struct sockaddr_storage currentAddress;

    int boundSocket;
    int connectedSocket;
    bool connected;
    int socketType;
 } XcpTl_ConnectionType;


extern pthread_t XcpHw_ThreadID[4];

unsigned char buf[XCP_COMM_BUFLEN];
int addrSize = sizeof(struct sockaddr_storage);


static XcpTl_ConnectionType XcpTl_Connection;
static XcpHw_OptionsType Xcp_Options;

static uint8_t Xcp_PduOutBuffer[XCP_MAX_CTO] = {0};
static struct epoll_event socket_events[MAX_EVENT_NUMBER] = {0};
static int epoll_fd = 0;


void Xcp_DispatchCommand(Xcp_PDUType const * const pdu);


extern Xcp_PDUType Xcp_PduIn;
extern Xcp_PDUType Xcp_PduOut;

static bool Xcp_EnableSocketOption(int sock, int option);
static bool Xcp_DisableSocketOption(int sock, int option);

static int Xcp_SetNonblocking(int fd);
static void Xcp_AddFd(int epoll_fd, int fd);
static void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd);
static void * XcpTl_WorkerThread(void * param);

static  bool Xcp_EnableSocketOption(int sock, int option)
{
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const char enable = 1;

    if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
        return false;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){1}, sizeof(int)) < 0) {
        return false;
    }
#endif
    return true;
}


static bool Xcp_DisableSocketOption(int sock, int option)
{
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const char enable = 0;

    if (setsockopt(sock, SOL_SOCKET, option, &enable, sizeof(int)) < 0) {
        return false;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){0}, sizeof(int)) < 0) {
        return false;
    }
#endif
    return true;
}

static int Xcp_SetNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

static void Xcp_AddFd(int epoll_fd, int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    Xcp_SetNonblocking(fd);
}

void XcpTl_Init(void)
{
    struct addrinfo hints, *addr_info;
    char * address = NULL;
    char * port = DEFAULT_PORT;
    int sock;
    int ret;

    XcpUtl_ZeroMem(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
    Xcp_PduOut.data = &Xcp_PduOutBuffer[0];
    memset(&hints, 0, sizeof(hints));

//    Xcp_Options.tcp = 1;

    XcpTl_Connection.socketType = Xcp_Options.tcp ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_family = Xcp_Options.ipv6 ? PF_INET6: PF_INET;
    hints.ai_socktype = XcpTl_Connection.socketType;
    hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    ret = getaddrinfo(address, port, &hints, &addr_info);

    if (ret != 0) {
        XcpHw_ErrorMsg("XcpTl_Init::getaddrinfo()", errno);
        return;
    }

    sock = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    printf("socket()\n");
    if (sock == -1){
        XcpHw_ErrorMsg("XcpTl_Init::socket()", errno);
        return;
    }
    if (bind(sock, addr_info->ai_addr, addr_info->ai_addrlen) == -1) {
        XcpHw_ErrorMsg("XcpTl_Init::bind()", errno);
        return;
    }
    if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (listen(sock, 1) == -1) {
            XcpHw_ErrorMsg("XcpTl_Init::listen()", errno);
            return;
        }
    }

    XcpTl_Connection.boundSocket = sock;
    freeaddrinfo(addr_info);
    if (!Xcp_EnableSocketOption(XcpTl_Connection.boundSocket, SO_REUSEADDR)) {
        XcpHw_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEADDR)", errno);
    }

    epoll_fd = epoll_create(EVENT_TABLE_SIZE);
    if (epoll_fd == -1) {
        XcpHw_ErrorMsg("XcpTl_Init::epoll_create()", errno);
        exit(1);
    }

    Xcp_AddFd(epoll_fd, sock);

    ret = pthread_create(&XcpHw_ThreadID[0], NULL, &XcpTl_WorkerThread, NULL);
    if (ret != 0) {
        err_abort(ret, "Create worker thread");
    }

}

static void * XcpTl_WorkerThread(void * param)
{
    int status;

    while(1) {
        status = epoll_wait(epoll_fd, socket_events, MAX_EVENT_NUMBER, -1);
        if (status < 0) {
            printf("epoll failure!\n");
            break;
        }
        lt_process(socket_events, status, epoll_fd, XcpTl_Connection.boundSocket);
    }
    return NULL;
}


void XcpTl_DeInit(void)
{
    close(epoll_fd);
    close(XcpTl_Connection.boundSocket);
}


void XcpTl_MainFunction(void)
{
}

void * get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void XcpTl_RxHandler(void)
{
    int recv_len;
    uint16_t dlc;
    int FromLen;
    struct sockaddr_storage From;
    char hostname[NI_MAXHOST];

    XcpUtl_ZeroMem(buf, XCP_COMM_BUFLEN);

    if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (!XcpTl_Connection.connected) {
            FromLen = sizeof(From);
            XcpTl_Connection.connectedSocket = accept(XcpTl_Connection.boundSocket, (struct sockaddr *)&XcpTl_Connection.currentAddress, &FromLen);
            if (XcpTl_Connection.connectedSocket == -1) {
                XcpHw_ErrorMsg("XcpTl_RxHandler::accept()", errno);
                //WSACleanup();
                exit(1);
                return;
            }
            inet_ntop(XcpTl_Connection.currentAddress.ss_family, get_in_addr((struct sockaddr *)&XcpTl_Connection.currentAddress), hostname, sizeof(hostname));
            printf("server: got connection from %s\n", hostname);
        }
        recv_len = recv(XcpTl_Connection.connectedSocket, (char*)buf, XCP_COMM_BUFLEN, 0);
        if (recv_len == -1) {
            if (errno != EAGAIN) {
                XcpHw_ErrorMsg("XcpTl_RxHandler::recv()", errno);
                close(XcpTl_Connection.connectedSocket);
                exit(1);
                return;
            }
        }
        if (recv_len == 0) {
            DBG_PRINT1("Client closed connection\n");
            close(XcpTl_Connection.connectedSocket);
            Xcp_Disconnect();
            return;
        }
    } else {
        recv_len = recvfrom(XcpTl_Connection.boundSocket, (char*)buf, XCP_COMM_BUFLEN, 0,
            (struct sockaddr *)&XcpTl_Connection.currentAddress, &addrSize
        );
        if (recv_len == -1)
        {
            XcpHw_ErrorMsg("XcpTl_RxHandler:recvfrom()", errno);
            fflush(stdout);
            exit(1);
        }
        //printf("Received %d bytes from client: ", recv_len);
        //Xcp_Hexdump(buf, recv_len);
    }
    if (recv_len > 0) {
#if XCP_TRANSPORT_LAYER_LENGTH_SIZE == 1
        dlc = (uint16_t)buf[0];
#elif XCP_TRANSPORT_LAYER_LENGTH_SIZE == 2
        dlc = XCP_MAKEWORD(buf[0], buf[1]);
        //dlc = (uint16_t)*(buf + 0);
#endif // XCP_TRANSPORT_LAYER_LENGTH_SIZE
        if (!XcpTl_Connection.connected || (XcpTl_VerifyConnection())) {
            Xcp_PduIn.len = dlc;
            Xcp_PduIn.data = buf + XCP_TRANSPORT_LAYER_BUFFER_OFFSET;
            Xcp_DispatchCommand(&Xcp_PduIn);
        }
        if (recv_len < 5) {
            DBG_PRINT2("Error: frame to short: %d\n", recv_len);
        } else {

        }
        fflush(stdout);
    }
}

static void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buf[BUFFER_SIZE];
    int i;
    int recv_len;
    uint16_t dlc;
    int from_len = sizeof(struct sockaddr_storage);
    char hostname[NI_MAXHOST];

    for(i = 0; i < number; i++) //number: number of events ready
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listen_fd) { //If it is a file descriptor for listen, it indicates that a new customer is connected to
            if (!XcpTl_Connection.connected) {
                printf("Try connect\n");
                XcpTl_Connection.connectedSocket = accept(XcpTl_Connection.boundSocket, (struct sockaddr *)&XcpTl_Connection.currentAddress, &from_len);
                if (XcpTl_Connection.connectedSocket == -1) {
                    XcpHw_ErrorMsg("XcpTl_RxHandler::accept()", errno);
                    exit(1);
                    return;
                }
                Xcp_AddFd(epoll_fd, XcpTl_Connection.connectedSocket);
                inet_ntop(XcpTl_Connection.currentAddress.ss_family, get_in_addr((struct sockaddr *)&XcpTl_Connection.currentAddress), hostname, sizeof(hostname));
                printf("server: got connection from %s\n", hostname);
            }


        } else if(events[i].events & EPOLLIN) { //Readable with client data
            // This code is triggered as long as the data in the buffer has not been read.This is what LT mode is all about: repeating notifications until processing is complete
            //printf("Recv\n");
            //printf("lt mode: event trigger once!\n");
            memset(buf, 0, BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if(ret <= 0)  //After reading the data, remember to turn off fd
            {
                printf("disco()\n");
                close(sockfd);
                continue;
            }
//            printf("get %d bytes of content: \n", ret);
            recv_len = ret;
//            hexdump(buf, ret);
            if (recv_len > 0) {
        #if XCP_TRANSPORT_LAYER_LENGTH_SIZE == 1
                dlc = (uint16_t)buf[0];
        #elif XCP_TRANSPORT_LAYER_LENGTH_SIZE == 2
                dlc = XCP_MAKEWORD(buf[0], buf[1]);
                //dlc = (uint16_t)*(buf + 0);
        #endif // XCP_TRANSPORT_LAYER_LENGTH_SIZE
                if (!XcpTl_Connection.connected || (XcpTl_VerifyConnection())) {
                    Xcp_PduIn.len = dlc;
                    Xcp_PduIn.data = buf + XCP_TRANSPORT_LAYER_BUFFER_OFFSET;
                    Xcp_DispatchCommand(&Xcp_PduIn);
                }
                if (recv_len < 5) {
                    DBG_PRINT2("Error: frame to short: %d\n", recv_len);
                } else {

                }
                fflush(stdout);
            }
        }
        else
        {
            printf("something unexpected happened!\n");
        }
    }
}

void XcpTl_TxHandler(void)
{

}

void XcpTl_Send(uint8_t const * buf, uint16_t len)
{

    XcpUtl_Hexdump(buf,  len);

    if (XcpTl_Connection.socketType == SOCK_DGRAM) {
        if (sendto(XcpTl_Connection.boundSocket, (char const *)buf, len, 0,
            (struct sockaddr const *)&XcpTl_Connection.connectionAddress, addrSize) == -1) {
            XcpHw_ErrorMsg("XcpTl_Send:sendto()", errno);
        }
    } else if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (send(XcpTl_Connection.connectedSocket, (char const *)buf, len, 0) == -1) {
            XcpHw_ErrorMsg("XcpTl_Send:send()", errno);
            close(XcpTl_Connection.connectedSocket);
        }
    }
}

void XcpTl_SaveConnection(void)
{
    XcpUtl_MemCopy(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_storage));
    XcpTl_Connection.connected = true;
}


void XcpTl_ReleaseConnection(void)
{
    XcpTl_Connection.connected = false;
}


bool XcpTl_VerifyConnection(void)
{
    return memcmp(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(struct sockaddr_storage)) == 0;
}

void XcpTl_SetOptions(XcpHw_OptionsType const * options)
{
    Xcp_Options = *options;
}

void XcpTl_PrintConnectionInformation(void)
{
    printf("\nXCPonEth -- Listening on port %s / %s [%s]\n",
        DEFAULT_PORT,
        Xcp_Options.tcp ? "TCP" : "UDP",
        Xcp_Options.ipv6 ? "IPv6" : "IPv4"
    );
}
