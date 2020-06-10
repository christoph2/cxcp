
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "xcp.h"
#include "xcp_hw.h"
#include "xcp_tui.h"
#include "app_config.h"

void usage(void);

void XcpOnCan_Init(void);


void XcpTl_SetOptions(XcpHw_OptionsType const * options);
void * XcpHw_MainFunction(void);
void XcpTl_MainFunction(void);
void * TlTask(void * param);

void * AppTask(void * param);

extern pthread_t XcpHw_ThreadID[4];

#define XCP_THREAD  (0)
#define UI_THREAD   (1)
#define APP_THREAD  (2)

#define NUM_THREADS (3)


#if defined(ETHER)
static const char OPTION_STR[] = "htu46";
#elif defined(SOCKET_CAN)
static const char OPTION_STR[] = "hif";
#endif

int main(int argc, char **argv)
{
    int flags, opt;
    XcpHw_OptionsType options;

    srand(23);

    while ((opt = getopt(argc, argv, OPTION_STR)) != -1) {
        printf("opt: %c\n", opt);
    }


    XcpHw_ParseCommandLineOptions(argc, argv, &options);
    XcpTl_SetOptions(&options);

    Xcp_Init();
    Xcp_DisplayInfo();

    pthread_create(&XcpHw_ThreadID[1], NULL, &AppTask, NULL);
    pthread_create(&XcpHw_ThreadID[2], NULL, &XcpTui_MainFunction, NULL);
    pthread_create(&XcpHw_ThreadID[3], NULL, &TlTask, NULL);

    pthread_join(XcpHw_ThreadID[2], NULL);
#if 0
    while (state != 0x01) {
        state = XcpHw_WaitApplicationState(0x01);
//        printf("signaled/main: %u\n", state);
        XcpHw_ResetApplicationState(0x01);
    }
#endif

    //pthread_join(XcpHw_ThreadID[2], NULL);

#if 0
    while (XCP_TRUE) {
        XcpTl_MainFunction();
    }
#endif

    XcpHw_Deinit();
    XcpTl_DeInit();

    return 0;
}


void usage(void)
{

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

