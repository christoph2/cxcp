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

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#define BUFFER_SIZE 1024

#define XCP_COMM_PORT    (5555)

#define DEFAULT_FAMILY     PF_UNSPEC // Accept either IPv4 or IPv6
#define DEFAULT_SOCKTYPE   SOCK_STREAM //
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

void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buf[BUFFER_SIZE];
    int i;
    for(i = 0; i < number; i++) //number: number of events ready
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listen_fd)  //If it is a file descriptor for listen, it indicates that a new customer is connected to
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength);
            add_fd(epoll_fd, connfd, false);  //Register new customer connection fd to epoll event table, using lt mode
        }
        else if(events[i].events & EPOLLIN) //Readable with client data
        {
            // This code is triggered as long as the data in the buffer has not been read.This is what LT mode is all about: repeating notifications until processing is complete
            printf("lt mode: event trigger once!\n");
            memset(buf, 0, BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if(ret <= 0)  //After reading the data, remember to turn off fd
            {
                close(sockfd);
                continue;
            }
            printf("get %d bytes of content: \n", ret);
//            hexdump(buf, ret);

        }
        else
        {
            printf("something unexpected happened!\n");
        }
    }
}

int SetNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void add_fd(int epoll_fd, int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN; //Registering the fd is readable

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);  //Register the fd with the epoll kernel event table
    SetNonblocking(fd);
}

void XcpTl_Init(void)
{
    struct addrinfo hints, *addr_info;
    char *Address = NULL;
    char *Port = DEFAULT_PORT;
    int serverSockets[FD_SETSIZE];
    int boundSocketNum = -1;
    int ret;
    int idx;

    printf("Tl_Init()\n");

    XcpUtl_ZeroMem(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
    XcpUtl_ZeroMem(serverSockets, FD_SETSIZE);
    Xcp_PduOut.data = &Xcp_PduOutBuffer[0];
    memset(&hints, 0, sizeof(hints));

    Xcp_Options.tcp = 1;

    XcpTl_Connection.socketType = Xcp_Options.tcp ? SOCK_STREAM : SOCK_DGRAM;
    hints.ai_family = Xcp_Options.ipv6 ? PF_INET6: PF_INET;
    hints.ai_socktype = XcpTl_Connection.socketType;
    hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    ret = getaddrinfo(Address, Port, &hints, &addr_info);

//    printf("Port: %s tcp: %u family: %d\n", Port, XcpTl_Connection.socketType, hints.ai_family);

    if (ret != 0) {
        XcpHw_ErrorMsg("XcpTl_Init::getaddrinfo()", errno);
        return;
    }

    serverSockets[0] = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    printf("socket()\n");
    if (serverSockets[0] == -1){
        XcpHw_ErrorMsg("XcpTl_Init::socket()", errno);
        return;
    }
    if (bind(serverSockets[0], addr_info->ai_addr, addr_info->ai_addrlen) == -1) {
        XcpHw_ErrorMsg("XcpTl_Init::bind()", errno);
        return;
    }
    if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (listen(serverSockets[0], 1) == -1) {
            XcpHw_ErrorMsg("XcpTl_Init::listen()", errno);
            return;
        }
    }

    printf("bound...\n");
    boundSocketNum = idx;
    XcpTl_Connection.boundSocket = serverSockets[boundSocketNum];
    freeaddrinfo(addr_info);
    if (boundSocketNum == -1) {
        fprintf(stderr, "Fatal error: unable to serve on any address.\nPerhaps" \
            " a server is already running on port %s / %s [%s]?\n",
            DEFAULT_PORT, Xcp_Options.tcp ? "TCP" : "UDP", Xcp_Options.ipv6 ? "IPv6" : "IPv4"
        );
        exit(2);
    }
    if (!Xcp_EnableSocketOption(XcpTl_Connection.boundSocket, SO_REUSEADDR)) {
        XcpHw_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEADDR)", errno);
    }


    epoll_fd = epoll_create(EVENT_TABLE_SIZE);
    if (epoll_fd == -1) {
        XcpHw_ErrorMsg("XcpTl_Init::epoll_create", errno);
        exit(1);
    }

    add_fd(epoll_fd, serverSockets[0]);

    while(1)
    {
        int ret = epoll_wait(epoll_fd, socket_events, MAX_EVENT_NUMBER, -1);
        if(ret < 0)
        {
            printf("epoll failure!\n");
            break;
        }

        lt_process(socket_events, ret, epoll_fd, serverSockets[0]);

    }
}


void XcpTl_DeInit(void)
{
    close(XcpTl_Connection.boundSocket);
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
//            if (getnameinfo(&From, FromLen, hostname, sizeof(hostname), NULL, 0, NI_NUMERICHOST) != 0) {
//                strcpy(hostname, "<unknown>");
//                //XcpHw_ErrorMsg("XcpTl_RxHandler::getnameinfo()", errno);
//            }
//        DBG_PRINT2("\nAccepted connection from %s\n", hostname);

        }
        recv_len = recv(XcpTl_Connection.connectedSocket, (char*)buf, XCP_COMM_BUFLEN, 0);
        if (recv_len == -1) {
            XcpHw_ErrorMsg("XcpTl_RxHandler::recv()", errno);
            close(XcpTl_Connection.connectedSocket);
            exit(1);
            return;
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
    FD_SET(XcpTl_Connection.boundSocket, &fds);

    // Return value:
    // -1: error occurred
    // 0: timed out
    // > 0: data ready to be read
    if (((XcpTl_Connection.socketType == SOCK_STREAM) && (!XcpTl_Connection.connected)) || (XcpTl_Connection.socketType == SOCK_DGRAM)) {
        res = select(0, &fds, 0, 0, &timeout);
        if (res == -1) {
            XcpHw_ErrorMsg("XcpTl_FrameAvailable:select()", errno);
            exit(2);
        }
        return res;
    } else {
        return 1;
    }
}


void XcpTl_Send(uint8_t const * buf, uint16_t len)
{
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

// Simple
#if 0
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>



#define MAXLINE 5
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5000
#define INFTIM 1000

void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    int i, maxi, listenfd, connfd, sockfd,epfd,nfds, portnumber;
    ssize_t n;
    char line[MAXLINE];
    socklen_t clilen;


    if ( 2 == argc )
    {
        if( (portnumber = atoi(argv[1])) < 0 )
        {
            fprintf(stderr,"Usage:%s portnumber/a/n",argv[0]);
            return 1;
        }
    }
    else
    {
        fprintf(stderr,"Usage:%s portnumber/a/n",argv[0]);
        return 1;
    }



    //Declare variables for the epoll_event structure, ev for registering events, and array for returning events to process

    struct epoll_event ev,events[20];
    //Generate epoll-specific file descriptors for processing accept s

    epfd=epoll_create(256);
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    //Set socket to non-blocking

    //setnonblocking(listenfd);

    //Set the file descriptor associated with the event to be processed

    ev.data.fd=listenfd;
    //Set the type of event to process

    ev.events=EPOLLIN|EPOLLET;
    //ev.events=EPOLLIN;

    //Register epoll events

    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    char *local_addr="127.0.0.1";
    inet_aton(local_addr,&(serveraddr.sin_addr));//htons(portnumber);

    serveraddr.sin_port=htons(portnumber);
    bind(listenfd,(struct sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, LISTENQ);
    maxi = 0;
    for ( ; ; ) {
        //Waiting for the epoll event to occur

        nfds=epoll_wait(epfd,events,20,500);
        //Handle all events that occur

        for(i=0;i<nfds;++i)
        {
            if(events[i].data.fd==listenfd)//If a new SOCKET user is detected to be connected to a bound SOCKET port, establish a new connection.

            {
                connfd = accept(listenfd,(struct sockaddr *)&clientaddr, &clilen);
                if(connfd<0){
                    perror("connfd<0");
                    exit(1);
                }
                //setnonblocking(connfd);

                char *str = inet_ntoa(clientaddr.sin_addr);
                printf("accapt a connection from\n ");
                //Setting file descriptors for read operations

                ev.data.fd=connfd;
                //Set Read Action Events for Annotation

                ev.events=EPOLLIN|EPOLLET;
                //ev.events=EPOLLIN;

                //Register ev

                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
            }
            else if(events[i].events&EPOLLIN)//If the user is already connected and receives data, read in.

            {
                printf("EPOLLIN\n");
                if ( (sockfd = events[i].data.fd) < 0)
                    continue;
                if ( (n = read(sockfd, line, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {
                        close(sockfd);
                        events[i].data.fd = -1;
                    } else
                        printf("readline error\n");
                } else if (n == 0) {
                    close(sockfd);
                    events[i].data.fd = -1;
                }
                if(n<MAXLINE-2)
                    line[n] = '\0';

                //Setting file descriptors for write operations

                ev.data.fd=sockfd;
                //Set Write Action Events for Annotation

                ev.events=EPOLLOUT|EPOLLET;
                //Modify the event to be handled on sockfd to EPOLLOUT

                //epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);

            }
            else if(events[i].events&EPOLLOUT) // If there is data to send

            {
                sockfd = events[i].data.fd;
                write(sockfd, line, n);
                //Setting file descriptors for read operations

                ev.data.fd=sockfd;
                //Set Read Action Events for Annotation

                ev.events=EPOLLIN|EPOLLET;
                //Modify the event to be processed on sockfd to EPOLIN

                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }
        }
    }
    return 0;
}
#endif

//epoll server with ET and LT dual mode
#if 0

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>


#Maximum number of define MAX_EVENT_NUMBER 1024 //event
#define BUFFER_SIZE 10 //Buffer Size
#Define ENABLE_ET 1 //Enable ET mode

/* Set file descriptor to non-congested  */
int SetNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

/* Register EPOLLIN on file descriptor FD into the epoll kernel event table indicated by epoll_fd, and the parameter enable_et specifies whether et mode is enabled for FD */
void AddFd(int epoll_fd, int fd, bool enable_et)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN; //Registering the fd is readable
    if(enable_et)
    {
        event.events |= EPOLLET;
    }

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);  //Register the fd with the epoll kernel event table
    SetNonblocking(fd);
}

/*  LT Work mode features: robust but inefficient */
void lt_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buf[BUFFER_SIZE];
    int i;
    for(i = 0; i < number; i++) //number: number of events ready
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listen_fd)  //If it is a file descriptor for listen, it indicates that a new customer is connected to
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength);
            AddFd(epoll_fd, connfd, false);  //Register new customer connection fd to epoll event table, using lt mode
        }
        else if(events[i].events & EPOLLIN) //Readable with client data
        {
            // This code is triggered as long as the data in the buffer has not been read.This is what LT mode is all about: repeating notifications until processing is complete
            printf("lt mode: event trigger once!\n");
            memset(buf, 0, BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if(ret <= 0)  //After reading the data, remember to turn off fd
            {
                close(sockfd);
                continue;
            }
            printf("get %d bytes of content: %s\n", ret, buf);

        }
        else
        {
            printf("something unexpected happened!\n");
        }
    }
}

/* ET Work mode features: efficient but potentially dangerous */
void et_process(struct epoll_event* events, int number, int epoll_fd, int listen_fd)
{
    char buf[BUFFER_SIZE];
    int i;
    for(i = 0; i < number; i++)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listen_fd)
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof(client_address);
            int connfd = accept(listen_fd, (struct sockaddr*)&client_address, &client_addrlength);
            AddFd(epoll_fd, connfd, true);  //Use et mode
        }
        else if(events[i].events & EPOLLIN)
        {
            /* This code will not be triggered repeatedly, so we cycle through the data to make sure that all the data in the socket read cache is read out.This is how we eliminate the potential dangers of the ET model */

            printf("et mode: event trigger once!\n");
            while(1)
            {
                memset(buf, 0, BUFFER_SIZE);
                int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                if(ret < 0)
                {
                    /* For non-congested IO, the following condition is true to indicate that the data has been read completely, after which epoll can trigger the EPOLLIN event on sockfd again to drive the next read operation */

                    if(errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        printf("read later!\n");
                        break;
                    }

                    close(sockfd);
                    break;
                }
                else if(ret == 0)
                {
                    close(sockfd);
                }
                else //Not finished, continue reading in a loop
                {
                    printf("get %d bytes of content: %s\n", ret, buf);
                }
            }
        }
        else
        {
            printf("something unexpected happened!\n");
        }
    }
}


int main(int argc, char* argv[])
{
    if(argc <= 2)
    {
        printf("usage:  ip_address + port_number\n");
        return -1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int ret = -1;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0)
    {
        printf("fail to create socket!\n");
        return -1;
    }

    ret = bind(listen_fd, (struct sockaddr*)&address, sizeof(address));
    if(ret == -1)
    {
        printf("fail to bind socket!\n");
        return -1;
    }

    ret = listen(listen_fd, 5);
    if(ret == -1)
    {
        printf("fail to listen socket!\n");
        return -1;
    }

    struct epoll_event events[MAX_EVENT_NUMBER];
    int epoll_fd = epoll_create(5);  //Event table size 5
    if(epoll_fd == -1)
    {
        printf("fail to create epoll!\n");
        return -1;
    }

    AddFd(epoll_fd, listen_fd, true); //Add listen file descriptor to event table using ET mode epoll

    while(1)
    {
        int ret = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if(ret < 0)
        {
            printf("epoll failure!\n");
            break;
        }

        if(ENABLE_ET)
        {
            et_process(events, ret, epoll_fd, listen_fd);
        }
        else
        {
            lt_process(events, ret, epoll_fd, listen_fd);
        }

    }

    close(listen_fd);
    return 0;

}
#endif

