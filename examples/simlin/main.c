
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xcp.h"
#include "xcp_hw.h"
#include "xcp_tui.h"
#include "app_config.h"

void usage(void);

void XcpOnCan_Init(void);

void * XcpHw_MainFunction(void);
void XcpTl_MainFunction(void);
void * TlTask(void * param);

void * AppTask(void * param);

extern pthread_t XcpHw_ThreadID[4];

#define XCP_THREAD  (0)
#define UI_THREAD   (1)
#define APP_THREAD  (2)

#define NUM_THREADS (3)

#define DEFAULT_CAN_IF  "vcan0"
#define XCP_ETH_DEFAULT_PORT    (5555)

#if defined(ETHER)
static const char OPTION_STR[] = "htu46p:";
#elif defined(SOCKET_CAN)
static const char OPTION_STR[] = "hi:f";
#endif

Xcp_OptionsType Xcp_Options = {0};

int main(int argc, char **argv)
{
    int opt;
    int res;

#if defined(ETHER)
    int p_assigned = 0;
    int v_assigned = 0;

    Xcp_Options.tcp = XCP_TRUE;
    Xcp_Options.ipv6 = XCP_FALSE;
    Xcp_Options.port = XCP_ETH_DEFAULT_PORT;
#elif defined(SOCKET_CAN)
    int if_assigned = 0;

    Xcp_Options.fd = XCP_FALSE;
#endif

    srand(23);

    while ((opt = getopt(argc, argv, OPTION_STR)) != -1) {
        switch (opt) {
            case 'h':
            case '?':
                usage();
                break; /* never reached. */
#if defined(ETHER)
            case 't':
                if (p_assigned) {
                    printf("-t and -u options are mutual exclusive.\n");
                    exit(1);
                }
                p_assigned = XCP_TRUE;
                Xcp_Options.tcp = XCP_TRUE;
                break;
            case 'u':
                if (p_assigned) {
                    printf("-t and -u options are mutual exclusive.\n");
                    exit(1);
                }
                p_assigned = XCP_TRUE;
                Xcp_Options.tcp = XCP_FALSE;
                break;
            case '4':
                if (v_assigned) {
                    printf("-4 and -6 options are mutual exclusive.\n");
                    exit(1);
                }
                v_assigned = XCP_TRUE;
                Xcp_Options.ipv6 = XCP_FALSE;
                break;
            case '6':
                if (v_assigned) {
                    printf("-4 and -6 options are mutual exclusive.\n");
                    exit(1);
                }
                v_assigned = XCP_TRUE;
                Xcp_Options.ipv6 = XCP_TRUE;
                break;
            case 'p':
                Xcp_Options.port = atoi(optarg);
#elif defined(SOCKET_CAN)
            case 'f':
                Xcp_Options.fd = XCP_TRUE;
                break;
            case 'i':
                if_assigned = 1;
                strcpy(Xcp_Options.interface, optarg);
                break;
#endif
        }
    }

#if defined(SOCKET_CAN)
    if (!if_assigned) {
        strcpy(Xcp_Options.interface, DEFAULT_CAN_IF);
    }
#endif

    FlsEmu_Init(&FlsEmu_Config);
    Xcp_Init();

    res = pthread_create(&XcpHw_ThreadID[1], NULL, &AppTask, NULL);
    res = pthread_create(&XcpHw_ThreadID[2], NULL, &XcpTui_MainFunction, NULL);
    res = pthread_create(&XcpHw_ThreadID[3], NULL, &TlTask, NULL);

    pthread_join(XcpHw_ThreadID[2], NULL);

    XcpHw_Deinit();
    XcpTl_DeInit();

    FlsEmu_DeInit();

    return 0;
}


void usage(void)
{
    printf("\nparameter summary: \n");
#if defined(ETHER)
    printf("-h\t  this message.\n");
    printf("-t\t  TCP\t\t  default: TRUE\n");
    printf("-u\t  UDP\t\t  default: FALSE\n");
    printf("-4\t  IPv4\t\t  default: TRUE\n");
    printf("-6\t  IPv6\t\t  default: FALSE\n");
    printf("-p <port> port to listen  default: 5555\n");
#elif defined(SOCKET_CAN)
    printf("-h\tthis message.\n");
    printf("-f\t\tuse CAN-FD\t\tdefault: FALSE\n");
    printf("-i <if-name>\tinterface to use\tdefault: vcan0\n");
#endif
    exit(0);
}

#if 0
DWORD Xcp_MainTask(LPVOID param)
{
    HANDLE * quit_event = (HANDLE *)param;
    XCP_FOREVER {

        Xcp_MainFunction();
        XcpTl_MainFunction();

        if (WaitForSingleObject(*quit_event, INFINITE) == WAIT_OBJECT_0) {
            break;
        }
    }
    ExitThread(0);
}
#endif

void * TlTask(void * param)
{
    XCP_FOREVER {
        XcpTl_MainFunction();
    }
    return NULL;
}

void * AppTask(void * param)
{
    static uint32_t currentTS = 0UL;
    static uint32_t previousTS = 0UL;
    static uint16_t ticker = 0;
    uint32_t state;
#if 0
    HANDLE * quit_event = (HANDLE *)param;
    LARGE_INTEGER liDueTime;
    LONG period;
    HANDLE handles[2] = {*quit_event, userTimer};
    DWORD res;

    //liDueTime.QuadPart=-10 0 000 000;
    liDueTime.QuadPart=-50000;
    period = 5000;
    SetWaitableTimer(userTimer, &liDueTime, period, NULL, NULL, 0);
#endif

    XCP_FOREVER {

        state = XcpHw_WaitApplicationState(0x03);
//        printf("signaled/AppTask: %u\n", state);
        XcpHw_ResetApplicationState(state);
        if (state == 1) {
            break;
        }

        currentTS = XcpHw_GetTimerCounter() / 1000;
        if (currentTS >= (previousTS + 10)) {
//            printf("T [%u::%lu]\n", currentTS, XcpHw_GetTimerCounter() / 1000);

            if ((ticker % 3) == 0) {
                if (triangle.down) {
                    triangle.value--;
                    if (triangle.value == 0) {
                        triangle.down = XCP_FALSE;
                    }
                } else {
                    triangle.value++;
                    if (triangle.value >= 75) {
                        triangle.down = XCP_TRUE;
                    }
                }
                randomValue = (uint16_t)rand();

                //printf("\t\t\tTRI [%u]\n", triangle.value);
                XcpDaq_TriggerEvent(1);
            }
            ticker++;
            previousTS =  XcpHw_GetTimerCounter() / 1000;
        }
    }
    return NULL;
}

