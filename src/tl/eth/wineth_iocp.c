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

typedef enum tagHandleType {
    HANDLE_SOCKET,
    HANDLE_FILE,
    HANDLE_NAMED_PIPE,
    HANDLE_USER,
} HandleType;

typedef enum tagIoOpcode {
    IoAccept,
    IoRead,
    IoWrite
} IoOpcode;

typedef struct tagPerHandleData {
    HandleType handleType;
    HANDLE handle;
    DWORD seqNoSend;
    DWORD seqNoRecv;
} PerHandleData;

typedef struct tagPerIoData {
    OVERLAPPED overlapped;
    IoOpcode opcode;
    SOCKET socket;
    char buf[XCP_COMM_BUFLEN];
    WSABUF wsabuf;
} PerIoData;

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
static void XcpTl_TriggerRecv(DWORD numBytes);
static void XcpTl_Feed(DWORD numBytesReceived);

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

#define TL_WORKER_THREAD    (0)
#define TL_ACCEPTOR_THREAD  (1)
#define NUM_TL_THREADS      (2)

static HANDLE XcpTl_Threads[NUM_TL_THREADS];
static PerIoData recvOlap = {0};
static PerIoData sendOlap = {0};

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

    ZeroMemory(&XcpTl_Connection, sizeof(XcpTl_ConnectionType));
    XcpTl_Connection.iocp = XcpTl_CreateIOCP();

    XcpTl_Threads[TL_WORKER_THREAD] = CreateThread(NULL, 0, WorkerThread, (LPVOID)XcpTl_Connection.iocp, 0, NULL);
    SetThreadPriority(XcpTl_Threads[TL_WORKER_THREAD], THREAD_PRIORITY_ABOVE_NORMAL);
    SetProcessAffinityMask(XcpTl_Threads[TL_WORKER_THREAD], 1UL);

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
    recvOlap.opcode = IoRead;
    recvOlap.wsabuf.buf = recvOlap.buf;
    recvOlap.wsabuf.len = XCP_COMM_BUFLEN;

    sendOlap.opcode = IoWrite;
    sendOlap.wsabuf.buf = sendOlap.buf;
    sendOlap.wsabuf.len = XCP_COMM_BUFLEN;

    if (XcpTl_Connection.socketType == SOCK_STREAM) {
        XcpTl_Threads[TL_ACCEPTOR_THREAD] = CreateThread(NULL, 0, AcceptorThread, NULL, 0, NULL);
        SetProcessAffinityMask(XcpTl_Threads[TL_ACCEPTOR_THREAD], 1UL);
    } else {
        recvOlap.opcode = IoAccept;
        XcpTl_TriggerRecv(XCP_COMM_BUFLEN);
    }
}

void XcpTl_DeInit(void)
{
    size_t idx;

    XcpTl_PostQuitMessage();
    closesocket(XcpTl_Connection.connectedSocket);
    closesocket(XcpTl_Connection.boundSocket);
    CloseHandle(XcpTl_Connection.iocp);
    WaitForMultipleObjects(NUM_TL_THREADS, XcpTl_Threads, TRUE, INFINITE);
    for (idx = 0; idx < NUM_TL_THREADS; ++idx) {
        CloseHandle(XcpTl_Threads[idx]);
    }

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
    int fromLen;
    SOCKADDR_STORAGE From;
    DWORD error;
    DWORD flags = (DWORD)0;

    XCP_FOREVER {
        if (!XcpTl_Connection.xcpConnected) {
            fromLen = sizeof(From);
            XcpTl_Connection.connectedSocket = accept(XcpTl_Connection.boundSocket,
                (LPSOCKADDR)&XcpTl_Connection.currentAddress, &fromLen
            );
            if (XcpTl_Connection.connectedSocket == INVALID_SOCKET) {
                error = WSAGetLastError();
                if (error == WSAEINTR) {
                    break;
                } else {
                    Win_ErrorMsg("AcceptorThread::accept()", error);
                    WSACleanup();
                    exit(1);
                }
            }
            XcpTl_Connection.socketConnected = XCP_TRUE;
            XcpTl_RegisterIOCPHandle(XcpTl_Connection.iocp,
                (HANDLE)XcpTl_Connection.connectedSocket,
                (ULONG_PTR)XcpTl_Connection.connectedSocket
            );
            XcpTl_TriggerRecv(XCP_COMM_BUFLEN);
        }
    }
    ExitThread(0);
}

void XcpTl_Send(uint8_t const * buf, uint16_t len)
{
    DWORD bytesWritten;
    int addrLen;

    sendOlap.wsabuf.buf = buf;
    sendOlap.wsabuf.len = len;
    sendOlap.opcode = IoWrite;

    SecureZeroMemory(&sendOlap.overlapped, sizeof(OVERLAPPED));
    if (XcpTl_Connection.socketType == SOCK_DGRAM) {
        addrLen = sizeof(SOCKADDR_STORAGE);
        WSASendTo(XcpTl_Connection.boundSocket,
            &sendOlap.wsabuf,
            1,
            &bytesWritten,
            0,
            (LPSOCKADDR)&XcpTl_Connection.currentAddress,
            addrLen,
            (LPWSAOVERLAPPED)&sendOlap.overlapped,
            NULL
        );
        if (sendto(XcpTl_Connection.boundSocket, (char const *)buf, len, 0,
            (SOCKADDR * )(SOCKADDR_STORAGE const *)&XcpTl_Connection.connectionAddress, addrSize) == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_Send:WSASendTo()", WSAGetLastError());
        }
    } else if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (WSASend(
            XcpTl_Connection.connectedSocket,
            &sendOlap.wsabuf,
            1,
            &bytesWritten,
            0,
            (LPWSAOVERLAPPED)&sendOlap.overlapped,
            NULL) == SOCKET_ERROR) {
            Win_ErrorMsg("XcpTl_Send:WSASend()", WSAGetLastError());
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
    bool res =  memcmp(&XcpTl_Connection.connectionAddress, &XcpTl_Connection.currentAddress, sizeof(SOCKADDR_STORAGE)) == 0;
    return res;
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
** TODO: use SetFileCompletionNotificationModes() and SetFileIoOverlappedRange() on Vista and later.
*/

static DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
    HANDLE hCompletionPort = (HANDLE)lpParameter;
    DWORD numBytesReceived = 0;
    ULONG_PTR CompletionKey;
    PerIoData * iod = NULL;
    OVERLAPPED * olap = NULL;
    bool exitLoop = FALSE;
    DWORD flags = (DWORD)0;
    DWORD error;


    while (!exitLoop) {
        if (GetQueuedCompletionStatus(hCompletionPort, &numBytesReceived, &CompletionKey, (LPOVERLAPPED*)&olap, INFINITE)) {
            if ((numBytesReceived == 0) &&  (CompletionKey == 0)) {
                exitLoop = XCP_TRUE;
            } else {
                iod = (PerIoData*)olap;
                switch (iod->opcode) {
                    case IoAccept:
                        //printf("ACCEPT() %d %ld\n", numBytesReceived, CompletionKey);
                        printf("\t");
                        XcpUtl_Hexdump(recvOlap.wsabuf.buf, numBytesReceived);
                        printf("\n");
//                        XcpTl_Connection.socketConnected = XCP_TRUE;
                        recvOlap.opcode = IoRead;
                        XcpTl_Feed(numBytesReceived);
                        XcpTl_TriggerRecv(XCP_COMM_BUFLEN);
                        break;
                    case IoRead:
                        //printf("READ() %d %ld [%d]\n", numBytesReceived, CompletionKey, recvOlap.opcode);
                        if (numBytesReceived == (DWORD)0) {
                            DBG_PRINT1("Client closed connection\n");
                            XcpTl_Connection.socketConnected = XCP_FALSE;
                            closesocket(XcpTl_Connection.connectedSocket);
                            Xcp_Disconnect();
                        }
                        XcpTl_Feed(numBytesReceived);
                        XcpTl_TriggerRecv(XCP_COMM_BUFLEN);
                        break;
                    case IoWrite:
                        //printf("WRITE() %d %ld\n", numBytesReceived, CompletionKey);
#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
                        Xcp_UploadSingleFrame();
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */
                        break;
                }
            }
        } else {
            error = GetLastError();
            if (olap == NULL) {

            } else {
                // Failed I/O operation.
                // The function stores information in the variables pointed to by lpNumberOfBytes, lpCompletionKey.
            }
            Win_ErrorMsg("WorkerThread::GetQueuedCompletionStatus()", error);
        }
    }
    ExitThread(0);
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


static void XcpTl_TriggerRecv(DWORD numBytes)
{
    DWORD numReceived = (DWORD)0;
    DWORD flags = (DWORD)0;
    DWORD err = 0;
    int addrLen;

    SecureZeroMemory(&recvOlap.overlapped, sizeof(OVERLAPPED));

    if (XcpTl_Connection.socketType == SOCK_STREAM) {
        if (XcpTl_Connection.socketConnected == XCP_FALSE) {
            return;
        }
        if (WSARecv(XcpTl_Connection.connectedSocket,
                    &recvOlap.wsabuf,
                    1,
                    &numReceived,
                    &flags,
                    (LPWSAOVERLAPPED)&recvOlap,
                    (LPWSAOVERLAPPED_COMPLETION_ROUTINE)NULL)  == SOCKET_ERROR) {
            err = WSAGetLastError();
            if (err != WSA_IO_PENDING) {
                Win_ErrorMsg("XcpTl_TriggerRecv::WSARecv()", err);
            }
        }
    } else if (XcpTl_Connection.socketType == SOCK_DGRAM) {
        addrLen = sizeof(SOCKADDR_STORAGE);
        if (WSARecvFrom(XcpTl_Connection.boundSocket,
                    &recvOlap.wsabuf,
                    1,
                    &numReceived,
                    &flags,
                    (LPSOCKADDR)&XcpTl_Connection.currentAddress,
                    &addrLen,
                    (LPWSAOVERLAPPED)&recvOlap,
                    (LPWSAOVERLAPPED_COMPLETION_ROUTINE)NULL)) {
            err = WSAGetLastError();
            if (err != WSA_IO_PENDING) {
                Win_ErrorMsg("XcpTl_TriggerRecv:WSARecvFrom()", WSAGetLastError());
            }
        }
    }
}


static void XcpTl_Feed(DWORD numBytesReceived)
{
    uint16_t dlc;

    if (numBytesReceived > 0) {
#if XCP_TRANSPORT_LAYER_LENGTH_SIZE == 1
        dlc = (uint16_t)recvOlap.wsabuf.buf[0];
#elif XCP_TRANSPORT_LAYER_LENGTH_SIZE == 2
        dlc = MAKEWORD(recvOlap.wsabuf.buf[0], recvOlap.wsabuf.buf[1]);
#endif // XCP_TRANSPORT_LAYER_LENGTH_SIZE
        if (!XcpTl_Connection.xcpConnected || (XcpTl_VerifyConnection())) {
            Xcp_PduIn.len = dlc;
            Xcp_PduIn.data = recvOlap.wsabuf.buf + XCP_TRANSPORT_LAYER_BUFFER_OFFSET;
            Xcp_DispatchCommand(&Xcp_PduIn);
        }
        if (numBytesReceived < 5) {
            DBG_PRINT2("Error: frame to short: %d\n", numBytesReceived);
        } else {

        }
    }
}
