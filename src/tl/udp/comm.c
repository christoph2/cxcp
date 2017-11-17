#include <winsock2.h>
//#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "comm.h"

#pragma comment(lib,"ws2_32.lib") // MSVC


#if !defined(MAX)
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#endif

#define XCP_COMM_BUFLEN  (MAX(XCP_MAX_CTO, XCP_MAX_DTO))
#define XCP_COMM_PORT    (5555)

typedef struct tagXcpComm_MailboxType {
    uint16_t dlc;
    uint16_t ctr;
    uint8_t buffer[XCP_COMM_BUFLEN];
} XcpComm_MailboxType;


static SOCKET sock = INVALID_SOCKET;
struct sockaddr_in server = {0};
struct sockaddr_in remote = {0};
unsigned char buf[XCP_COMM_BUFLEN];
int addrSize = sizeof(struct sockaddr_in);


/// TODO: HEADER!
void XcpComm_Task(void);
void XcpComm_RxHandler(void);
void XcpComm_TxHandler(void);
///


static boolean Xcp_EnableSocketOption(SOCKET sock, int option);


void Win_ErrorMsg(char * const fun, DWORD errorCode)
{
    //LPWSTR buffer = NULL;
    char * buffer = NULL;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &buffer, 0, NULL);
    if (buffer != NULL) {
        fprintf(stderr, "[%s] failed with: %s", fun, buffer);
        LocalFree((HLOCAL)buffer);
    } else {
        //printf("FormatMessage failed!\n");
    }
}

static  boolean Xcp_EnableSocketOption(SOCKET sock, int option)
{
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    const int enable = 1;

    if (setsockopt(sock, SOL_SOCKET, options, &enable, sizeof(int)) < 0) {
        return FALSE;
    }
#else
    if (setsockopt(sock, SOL_SOCKET, option, &(const char){1}, sizeof(int)) < 0) {
        return FALSE;
    }
#endif
    return TRUE;
}

void XcpComm_Init(void)
{
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        Win_ErrorMsg("XcpComm_Init:WSAStartup()", WSAGetLastError());
        exit(EXIT_FAILURE);
    } else {
        printf("Winsock started!\n");
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        Win_ErrorMsg("XcpComm_Init:socket()", WSAGetLastError());
        exit(EXIT_FAILURE);
    } else {
        printf("UDP Socket created!\n");
    }

    if (!Xcp_EnableSocketOption(sock, SO_REUSEADDR)) {
        Win_ErrorMsg("XcpComm_Init:setsockopt(SO_REUSEADDR)", WSAGetLastError());
    }

#ifdef SO_REUSEPORT
    if (!Xcp_EnableSocketOption(sock, SO_REUSEPORT)) {
        Win_ErrorMsg("XcpComm_Init:setsockopt(SO_REUSEPORT)", WSAGetLastError());
    }
#endif

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(XCP_COMM_PORT);

    if(bind(sock ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR) {
        Win_ErrorMsg("XcpComm_Init:bind()", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    puts("Bind done");

    getsockname(sock, (SOCKADDR *)&server, (int *)sizeof(server));
    printf("Server: Receiving IP(s) used: %s\n", inet_ntoa(server.sin_addr));
    printf("Server: Receiving port used: %d\n", htons(server.sin_port));
    printf("Server: I\'m ready to receive a datagram...\n");

#if 0
    //keep listening for data
    while(1)
    {
        //now reply the client with the same data
    }
#endif
}

void XcpComm_DeInit(void)
{
    closesocket(sock);
    WSACleanup();
}

void XcpComm_Task(void)
{

}

void hexdump(unsigned char const * buf, int sz)
{
    for (int idx = 0; idx < sz; ++idx) {
        printf("%02X ", buf[idx]);
    }
    printf("\n");
}

void XcpComm_RxHandler(void)
{
    int recv_len;

    uint16_t dlc;
    uint16_t ctr;

    memset(buf,'\0', XCP_COMM_BUFLEN);

    recv_len = recvfrom(sock, buf, XCP_COMM_BUFLEN, 0, (struct sockaddr *)&remote, &addrSize);
    if (recv_len == SOCKET_ERROR)
    {
        Win_ErrorMsg("XcpComm_RxHandler:recvfrom()", WSAGetLastError());
    } else if (recv_len > 0) {
        dlc = (uint16_t)*(buf + 0);
        ctr = (uint16_t)*(buf + 2);

        printf("Received packet from %s:%d [%d] FL: %d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), recv_len, dlc);
        hexdump(buf, recv_len);
    }
}


void XcpComm_TxHandler(void)
{

}

int XcpComm_FrameAvailable(long sec, long usec)
{
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = usec;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    // Return value:
    // -1: error occurred
    // 0: timed out
    // > 0: data ready to be read
    return select(0, &fds, 0, 0, &timeout);
}

int XcpComm_Send(uint8_t const * buf, uint16_t len)
{
    if (sendto(sock, (char const *)buf, len, 0, (struct sockaddr*)&remote, addrSize) == SOCKET_ERROR) {
        Win_ErrorMsg("XcpComm_Send:sendto()", WSAGetLastError());
        return 0;
    }
    return 1;
}

#if 0

HANDLE timerHandle;

void CTimersDlg::OnButtonBegin()
{
    .
    .
    .
    // create the timer

    BOOL success = CreateTimerQueueTimer(
        &timerHandle,
        NULL,
        TimerProc,
        NULL,   // Parameter
        0,      // DueTime
        elTime, // Period
        WT_EXECUTEINTIMERTHREAD);
}

void CTimersDlg::OnButtonStop()
{
    // destroy the timer
    DeleteTimerQueueTimer(NULL, timerHandle, NULL);
    CloseHandle (timerHandle);
}

void CTimersDlg::QueueTimerHandler() // called every elTime milliseconds
{
// do what you want to do, but quickly
    .
    .
    .
}

void CALLBACK TimerProc(void* lpParametar, BOOLEAN TimerOrWaitFired)
    {
    // This is used only to call QueueTimerHandler
    // Typically, this function is static member of CTimersDlg
    /CTimersDlg* obj = (CTimersDlg*) lpParametar;
    //obj->QueueTimerHandler();
    }
#endif

