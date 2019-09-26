/*
 * BlueParrot XCP
 *
 * (C) 2007-2019 by Christoph Schueler <github.com/Christoph2,
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

#define _WIN32_WINNT    0x601

#include <WinSock2.h>
#include "mswsock.h"
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>




#include "xcp.h"
#include "xcp_hw.h"

#if !defined(__GNUC__)
#pragma comment(lib,"ws2_32.lib") // MSVC only.
#endif


#define XCP_COMM_PORT    (5555)

#define DEFAULT_FAMILY     PF_UNSPEC // Accept either IPv4 or IPv6
#define DEFAULT_SOCKTYPE   SOCK_STREAM //
#define DEFAULT_PORT       "5555"

void Win_ErrorMsg(char * const function, unsigned errorCode);


typedef struct tagXcpTl_ConnectionType {
    HANDLE iocp;
    SOCKADDR_STORAGE connectionAddress;
    SOCKADDR_STORAGE currentAddress;
    SOCKET boundSocket;
    SOCKET connectedSocket;
    bool xcpConnected;
    bool socketConnected;
    int socketType;
} XcpTl_ConnectionType;

typedef enum tagIoOpcode {
    IoAccept,
    IoRead,
    IoWrite
} IoOpcode;

typedef struct tagPerIoData {
    OVERLAPPED overlapped;
    IoOpcode opcode;
    SOCKET socket;
} PerIoData;

unsigned char buf[XCP_COMM_BUFLEN];
int addrSize = sizeof(SOCKADDR_STORAGE);


static XcpTl_ConnectionType XcpTl_Connection;
static XcpHw_OptionsType Xcp_Options;

static uint8_t Xcp_PduOutBuffer[XCP_MAX_CTO] = {0};


void Xcp_DispatchCommand(Xcp_PDUType const * const pdu);


extern Xcp_PDUType Xcp_PduIn;
extern Xcp_PDUType Xcp_PduOut;

static HANDLE XcpTl_CreateIOCP(void);
static bool XcpTl_RegisterIOCPHandle(HANDLE port, HANDLE object, ULONG_PTR key);
static DWORD WINAPI WorkerThread(LPVOID lpParameter);
static DWORD WINAPI AcceptorThread(LPVOID lpParameter);
void XcpTl_PostQuitMessage(void);
static void XcpTl_Receive(DWORD numBytes);

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

HANDLE hThread;
HANDLE hTimer;

void XcpTl_Init(void)
{
    WSADATA wsa;
    ADDRINFO Hints, *AddrInfo, *AI;
    char *Address = NULL;
    char *Port = DEFAULT_PORT;
    SOCKET serverSockets[FD_SETSIZE];
    int boundSocketNum = -1;
    int ret;
    int idx;
    DWORD dwTimeAdjustment = 0UL, dwTimeIncrement = 0UL;
    BOOL fAdjustmentDisabled = XCP_TRUE;

    LARGE_INTEGER liDueTime;

    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
    XcpTl_Connection.iocp = XcpTl_CreateIOCP();
    hThread = CreateThread(NULL, 0, WorkerThread, (LPVOID)XcpTl_Connection.iocp, 0, NULL);

    SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);

    Xcp_PduOut.data = &Xcp_PduOutBuffer[0];
    ZeroMemory(&Hints, sizeof(Hints));
    GetSystemTimeAdjustment(&dwTimeAdjustment, &dwTimeIncrement, &fAdjustmentDisabled);
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        Win_ErrorMsg("XcpTl_Init:WSAStartup()", WSAGetLastError());
        exit(EXIT_FAILURE);
    } else {

    }
    XcpTl_Connection.socketType = Xcp_Options.tcp ? SOCK_STREAM : SOCK_DGRAM;
    Hints.ai_family = Xcp_Options.ipv6 ? PF_INET6: PF_INET;
    Hints.ai_socktype = XcpTl_Connection.socketType;
    Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    ret = getaddrinfo(Address, Port, &Hints, &AddrInfo);
    if (ret != 0) {
        Win_ErrorMsg("XcpTl_Init::getaddrinfo()", WSAGetLastError());
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
        serverSockets[idx] = WSASocket(AI->ai_family, AI->ai_socktype, AI->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (serverSockets[idx] == INVALID_SOCKET){
            Win_ErrorMsg("XcpTl_Init::socket()", WSAGetLastError());
            continue;
        }

        XcpTl_RegisterIOCPHandle(XcpTl_Connection.iocp, (HANDLE)serverSockets[idx], (ULONG_PTR)serverSockets[idx]);

        if (bind(serverSockets[idx], AI->ai_addr, AI->ai_addrlen) == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_Init::bind()", WSAGetLastError());
            continue;
        }
        if (XcpTl_Connection.socketType == SOCK_STREAM) {
            if (listen(serverSockets[idx], 1) == SOCKET_ERROR) {
                Win_ErrorMsg("XcpTl_Init::listen()", WSAGetLastError());
                continue;
            }
        }
        boundSocketNum = idx;
        XcpTl_Connection.boundSocket = serverSockets[boundSocketNum];
        break;  /* Grab first address. */
    }
    freeaddrinfo(AddrInfo);
    if (boundSocketNum == -1) {
        fprintf(stderr, "Fatal error: unable to serve on any address.\nPerhaps" \
            " a server is already running on port %s / %s [%s]?\n",
            DEFAULT_PORT, Xcp_Options.tcp ? "TCP" : "UDP", Xcp_Options.ipv6 ? "IPv6" : "IPv4"
        );
        WSACleanup();
        exit(2);
    }
    if (!Xcp_EnableSocketOption(XcpTl_Connection.boundSocket, SO_REUSEADDR)) {
        Win_ErrorMsg("XcpTl_Init:setsockopt(SO_REUSEADDR)", WSAGetLastError());
    }
#if 0
    hTimer = CreateWaitableTimer(NULL, FALSE, "Timeout");
    if (hTimer == NULL) {
        Win_ErrorMsg("XcpTl_Init::CreateWaitableTimer()", GetLastError());
    }

    liDueTime.QuadPart=-300000000;
    SetWaitableTimer(hTimer, &liDueTime, 2000, NULL, NULL, 0);
    XcpTl_RegisterIOCPHandle(XcpTl_Connection.iocp, (HANDLE)hTimer, (ULONG_PTR)hTimer);
#endif
    hThread = CreateThread(NULL, 0, AcceptorThread, NULL, 0, NULL);
}


void XcpTl_DeInit(void)
{
    CloseHandle(hTimer);
    XcpTl_PostQuitMessage();
    closesocket(XcpTl_Connection.connectedSocket);
    closesocket(XcpTl_Connection.boundSocket);
    CloseHandle(XcpTl_Connection.iocp);
    WSACleanup();
}


void XcpTl_MainFunction(void)
{

}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


static DWORD WINAPI AcceptorThread(LPVOID lpParameter)
{
    int FromLen;
    SOCKADDR_STORAGE From;
    DWORD error;

    ZeroMemory(buf,XCP_COMM_BUFLEN);

    XCP_FOREVER {
        if (XcpTl_Connection.socketType == SOCK_STREAM) {
            if (!XcpTl_Connection.xcpConnected) {

                XcpTl_Connection.connectedSocket = accept(XcpTl_Connection.boundSocket, (LPSOCKADDR)&XcpTl_Connection.currentAddress, &FromLen);
                if (XcpTl_Connection.connectedSocket == INVALID_SOCKET) {
                    error = WSAGetLastError();
                    if (error == WSAEINTR) {
                        break;
                    } else {
                        Win_ErrorMsg("XcpTl_RxHandler::accept()", error);
                        WSACleanup();
                        exit(1);
                    }
                }

                XcpTl_Connection.socketConnected = XCP_TRUE;
                printf("Connected to Socket: %d\n", XcpTl_Connection.connectedSocket);
                XcpTl_RegisterIOCPHandle(XcpTl_Connection.iocp,
                    (HANDLE)XcpTl_Connection.connectedSocket,
                    (ULONG_PTR)XcpTl_Connection.connectedSocket
                );
                XcpTl_Receive(0);
            }
        }
    }
//    printf("Exiting Acceptor...\n");
    ExitThread(0);
}

void XcpTl_RxHandler(void)
{

}



void XcpTl_TxHandler(void)
{

}


int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec)
{
}


void XcpTl_Send(uint8_t const * buf, uint16_t len)
{
    static WSABUF sendBuffer;
    DWORD bytesWritten;
    static PerIoData ov = {0};

    sendBuffer.buf = buf;
    sendBuffer.len = len;
    ov.opcode = IoWrite;

    if (XcpTl_Connection.socketType == SOCK_DGRAM) {
        if (sendto(XcpTl_Connection.boundSocket, (char const *)buf, len, 0,
            (SOCKADDR * )(SOCKADDR_STORAGE const *)&XcpTl_Connection.connectionAddress, addrSize) == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_Send:sendto()", WSAGetLastError());
        }
    } else if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (WSASend(XcpTl_Connection.connectedSocket, &sendBuffer, 1, &bytesWritten, 0, (LPWSAOVERLAPPED)&ov, NULL) == SOCKET_ERROR) {
        //if (send(XcpTl_Connection.connectedSocket, (char const *)buf, len, 0) == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_Send:send()", WSAGetLastError());
            closesocket(XcpTl_Connection.connectedSocket);
        }
    }
}


void XcpTl_SaveConnection(void)
{
    CopyMemory(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(SOCKADDR_STORAGE));
    XcpTl_Connection.xcpConnected = XCP_TRUE;
}


void XcpTl_ReleaseConnection(void)
{
    XcpTl_Connection.xcpConnected = XCP_FALSE;
}


bool XcpTl_VerifyConnection(void)
{
    return memcmp(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(SOCKADDR_STORAGE)) == 0;
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


///////////////////
///////////////////
///////////////////
/*
** TODO: use SetFileCompletionNotificationModes() and SetFileIoOverlappedRange() on Vista an later.
*/

static DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
    HANDLE hCompletionPort = (HANDLE)lpParameter;
    DWORD NumBytesRecv = 0;
    ULONG_PTR CompletionKey;
    PerIoData * iod = NULL;
    OVERLAPPED * ov = NULL;
    bool exitLoop = FALSE;
    char receiveBuffer[XCP_COMM_BUFLEN];
    static WSABUF wsaBuffer;
    DWORD flags = (DWORD)0;
    DWORD numReceived;
    uint16_t dlc;
    DWORD error;

    wsaBuffer.buf = receiveBuffer;
    wsaBuffer.len = XCP_COMM_BUFLEN;

    while (!exitLoop) {
        if (GetQueuedCompletionStatus(hCompletionPort, &NumBytesRecv, &CompletionKey, (LPOVERLAPPED*)&ov, INFINITE)) {
//            printf("\tGot Event: %ld %ld\n", NumBytesRecv, CompletionKey);
            if ((NumBytesRecv == 0) &&  (CompletionKey == 0)) {
                exitLoop = XCP_TRUE;
            } else {
                iod = (PerIoData*)ov;
                switch (iod->opcode) {
                    case IoRead:
                        WSARecv(XcpTl_Connection.connectedSocket,
                                &wsaBuffer,
                                1,
                                &numReceived,
                                &flags,
                                (LPWSAOVERLAPPED)NULL,
                                (LPWSAOVERLAPPED_COMPLETION_ROUTINE)NULL
                        );
                        if (numReceived == (DWORD)0) {
                            DBG_PRINT1("Client closed connection\n");
                            XcpTl_Connection.socketConnected = XCP_FALSE;
                            closesocket(XcpTl_Connection.connectedSocket);
                            Xcp_Disconnect();
                        }
                        if (numReceived > 0) {
//                            printf("\t\tReceived %d bytes.\n", numReceived);
#if XCP_TRANSPORT_LAYER_LENGTH_SIZE == 1
                            dlc = (uint16_t)buf[0];
#elif XCP_TRANSPORT_LAYER_LENGTH_SIZE == 2
                            dlc = MAKEWORD(buf[0], buf[1]);
                            //dlc = (uint16_t)*(buf + 0);
#endif // XCP_TRANSPORT_LAYER_LENGTH_SIZE
                            if (!XcpTl_Connection.xcpConnected || (XcpTl_VerifyConnection())) {
                                Xcp_PduIn.len = dlc;
                                Xcp_PduIn.data = wsaBuffer.buf + XCP_TRANSPORT_LAYER_BUFFER_OFFSET;
                                Xcp_DispatchCommand(&Xcp_PduIn);
                            }
                            if (numReceived < 5) {
                                DBG_PRINT2("Error: frame to short: %d\n", numReceived);
                            } else {

                            }
                            fflush(stdout);
                        }
                        XcpTl_Receive(0);
                        break;
                    case IoAccept:
                        break;
                    case IoWrite:
                        break;
                }
            }
        } else {
            error = GetLastError();
            if (ov == NULL) {

            } else {
                // Failed I/O operation.
                // The function stores information in the variables pointed to by lpNumberOfBytes, lpCompletionKey.
            }
            Win_ErrorMsg("IOWorkerThread::GetQueuedCompletionStatus()", error);
        }
    }
    ExitThread(0);
    return 0;
}

static HANDLE XcpTl_CreateIOCP(void)
{
    HANDLE handle;

    handle  = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)0, 1);
    if (handle == NULL) {
        Win_ErrorMsg("XcpTl_CreateIOCP::CreateIoCompletionPort()", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    return handle;
}


static bool XcpTl_RegisterIOCPHandle(HANDLE port, HANDLE object, ULONG_PTR key)
{
    HANDLE handle;

    handle = CreateIoCompletionPort(object, port, key, 0);
    if (handle == NULL) {
        Win_ErrorMsg("XcpTl_RegisterIOCPHandle::CreateIoCompletionPort()", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    return (bool)(handle == port);
}

void XcpTl_PostQuitMessage(void)
{
    PostQueuedCompletionStatus(XcpTl_Connection.iocp, 0, (ULONG_PTR)NULL, NULL);
}


static void XcpTl_Receive(DWORD numBytes)
{
    static WSABUF receiveBuffer;
    static PerIoData ov = {0};
    DWORD numReceived = (DWORD)0;
    DWORD flags = (DWORD)0;
    DWORD err = 0;

    if (XcpTl_Connection.socketConnected == XCP_FALSE) {
        return;
    }

    ov.opcode = IoRead;
    if (WSARecv(XcpTl_Connection.connectedSocket,
                &receiveBuffer,
                1, &numReceived,
                &flags,
                (LPWSAOVERLAPPED)&ov,
                (LPWSAOVERLAPPED_COMPLETION_ROUTINE)NULL)  == SOCKET_ERROR) {
        err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            Win_ErrorMsg("XcpTl_Receive::WSARecv()", err);
        }
    }
//    printf("Rec'd: %d on [%d]\n", numReceived, XcpTl_Connection.connectedSocket);
}
