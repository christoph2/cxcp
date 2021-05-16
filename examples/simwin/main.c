
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define _WIN32_WINNT    0x601
#include <Windows.h>

#include "xcp.h"
#include "xcp_hw.h"

#include "app_config.h"

void XcpOnCan_Init(void);

#define SIZE    (4096)
uint8_t puffer[SIZE];

DWORD XcpHw_UIThread(LPVOID param);
void XcpTl_SetOptions(Xcp_OptionsType const * options);

DWORD AppTask(LPVOID param);
DWORD Xcp_MainTask(LPVOID param);

#if 0
void memimnfo(void * ptr)
{
    /*
    ** mainly a debugging/bug reporting aid.
    */

    MEMORY_BASIC_INFORMATION info;
    SYSTEM_INFO si;
    size_t res;

    GetSystemInfo(&si);
    res = VirtualQuery(ptr, &info, sizeof(MEMORY_BASIC_INFORMATION));
    if (!res) {
        XcpHw_ErrorMsg("memimnfo::VirtualQuery()", GetLastError());
    } else {
        printf("%p %p %d %08x %08x %08x\n", info.BaseAddress, info.AllocationBase, info.RegionSize / 1024,
               info.AllocationProtect, info.Protect, info.Type);
    }
}
#endif

#define XCP_THREAD  (0)
#define UI_THREAD   (1)
#define APP_THREAD  (2)

#define NUM_THREADS (3)

HANDLE threads[NUM_THREADS];
extern HANDLE quit_event;
HANDLE userTimer;
Xcp_OptionsType Xcp_Options;

float sine_wave = 1.0;


int main(int argc, char **argv)
{
    size_t idx;
    Xcp_OptionsType options;

    srand(23);

    XcpHw_ParseCommandLineOptions(argc, argv, &options);
    XcpTl_SetOptions(&options);

    FlsEmu_Init(&FlsEmu_Config);

    Xcp_Init();
    Xcp_DisplayInfo();

    quit_event = CreateEvent(NULL, TRUE, FALSE, NULL);

    userTimer = CreateWaitableTimer(NULL, FALSE, NULL);

    threads[XCP_THREAD] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Xcp_MainTask, &quit_event, 0, NULL);
    threads[UI_THREAD] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)XcpHw_UIThread, &quit_event, 0, NULL);
    threads[APP_THREAD] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AppTask, &quit_event, 0, NULL);

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);
    for (idx = 0; idx < NUM_THREADS; ++idx) {
        CloseHandle(threads[idx]);
    }

    CloseHandle(userTimer);

    FlsEmu_DeInit();
    XcpHw_Deinit();
    XcpTl_DeInit();

    return 0;
}


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

DWORD AppTask(LPVOID param)
{
    static uint32_t currentTS = 0UL;
    static uint32_t previousTS = 0UL;
    static uint16_t ticker = 0;
    HANDLE * quit_event = (HANDLE *)param;

    LARGE_INTEGER liDueTime;
    LONG period;
    HANDLE handles[2] = {*quit_event, userTimer};
    DWORD res;

    //liDueTime.QuadPart=-10 0 000 000;
    liDueTime.QuadPart=-500;
    period = 150;
    SetWaitableTimer(userTimer, &liDueTime, period, NULL, NULL, 0);

    XCP_FOREVER {
        currentTS = XcpHw_GetTimerCounter() / 1000;
        if (currentTS >= (previousTS + 2)) {
            //printf("T [%u::%lu]\n", currentTS, XcpHw_GetTimerCounter() / 1000);

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
//                printf("T: %0.2f ", t_env_func());
                //printf("\t\t\tTRI [%u]\n", triangle.value);
                XcpDaq_TriggerEvent(1);
            }
            ticker++;
            previousTS =  XcpHw_GetTimerCounter() / 1000;
        }
        res = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
        if (res == WAIT_OBJECT_0) {
            break;
        }
        //if (WaitForSingleObject(*quit_event, 0) == WAIT_OBJECT_0) {
        //    break;
        //}
    }
    ExitThread(0);
}
