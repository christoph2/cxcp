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

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include <windows.h>
#include <io.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include "xcp.h"
#include "xcp_hw.h"

/*
**  Local Types.
*/
typedef struct tagHwStateType {
    LARGE_INTEGER StartingTime;
    LARGE_INTEGER TicksPerSecond;
} HwStateType;


 /*
 ** Local Defines.
 */
#define TIMER_PS_1US   (1000000UL)
#define TIMER_PS_10US  (100000UL)
#define TIMER_PS_100US (10000UL)

#define TIMER_PS_1MS   (1000UL)
#define TIMER_PS_10MS  (100UL)
#define TIMER_PS_100MS (10UL)

#define TIMER_MASK_1    (0x000000FFUL)
#define TIMER_MASK_2    (0x0000FFFFUL)
#define TIMER_MASK_4    (0xFFFFFFFFUL)


/*
**  Local Function Prototypes.
*/
static void XcpHw_InitLocks(void);
static void XcpHw_DeinitLocks();

/*
**  External Function Prototypes.
*/
void XcpTl_PrintConnectionInformation(void);

static void DisplayHelp(void);
static void SystemInformation(void);

void FlsEmu_Info(void);
void XcpDaq_Info(void);
void XcpDaq_PrintDAQDetails();

bool XcpHw_MainFunction(HANDLE * quit_event);

#if (defined(_DEBUG)) || (XCP_BUILD_TYPE == XCP_DEBUG_BUILD)
void exitFunc(void);

/*
**  Local Variables.
*/
static HwStateType HwState = {0};
static CRITICAL_SECTION XcpHw_Locks[XCP_HW_LOCK_COUNT];


void exitFunc(void)
{
    printf("Exiting %s...\n", __argv[0]);
    _CrtDumpMemoryLeaks();
}
#endif

/*
**  Global Functions.
*/
void XcpHw_Init(void)
{
#if (defined(_DEBUG)) || (XCP_BUILD_TYPE == XCP_DEBUG_BUILD)
    atexit(exitFunc);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    fflush(stdout);
    //_setmode(_fileno(stdout), _O_WTEXT);    /* Permit Unicode output on console */
    //_setmode(_fileno(stdout), _O_U8TEXT);    /* Permit Unicode output on console */
    QueryPerformanceFrequency(&HwState.TicksPerSecond);
    XcpHw_InitLocks();
}

void XcpHw_Deinit(void)
{
    XcpHw_DeinitLocks();
}

uint32_t XcpHw_GetTimerCounter(void)
{
    LARGE_INTEGER Now;
    LARGE_INTEGER Elapsed;

    QueryPerformanceCounter(&Now);
    Elapsed.QuadPart = Now.QuadPart - HwState.StartingTime.QuadPart;

#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
    Elapsed.QuadPart *= TIMER_PS_1US;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10US
    Elapsed.QuadPart *= TIMER_PS_10US;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100US
    Elapsed.QuadPart *= TIMER_PS_100US;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
    Elapsed.QuadPart *= TIMER_PS_1MS;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10MS
    Elapsed.QuadPart *= TIMER_PS_10MS;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100MS
    Elapsed.QuadPart *= TIMER_PS_100MS;
#else
#error Timestamp-unit not supported.
#endif // XCP_DAQ_TIMESTAMP_UNIT

    Elapsed.QuadPart /= HwState.TicksPerSecond.QuadPart;

#if XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_1
    return (uint32_t)Elapsed.QuadPart & TIMER_MASK_1;
#elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_2
    return (uint32_t)Elapsed.QuadPart) & TIMER_MASK_2;
#elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_4
    return (uint32_t)Elapsed.QuadPart & TIMER_MASK_4;
#else
#error Timestamp-size not supported.
#endif // XCP_DAQ_TIMESTAMP_SIZE
}

static void XcpHw_InitLocks(void)
{
    uint8_t idx = UINT8(0);

    for (idx = UINT8(0); idx < XCP_HW_LOCK_COUNT; ++idx) {
        InitializeCriticalSection(&XcpHw_Locks[idx]);
    }
}

static void XcpHw_DeinitLocks(void)
{
    uint8_t idx = UINT8(0);

    for (idx = UINT8(0); idx < XCP_HW_LOCK_COUNT; ++idx) {
        DeleteCriticalSection(&XcpHw_Locks[idx]);
    }
}

void XcpHw_AcquireLock(uint8_t lockIdx)
{
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    EnterCriticalSection(&XcpHw_Locks[lockIdx]);
}

void XcpHw_ReleaseLock(uint8_t lockIdx)
{
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    LeaveCriticalSection(&XcpHw_Locks[lockIdx]);
}

void Win_ErrorMsg(char * const fun, unsigned errorCode)
{
    char buffer[1024];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  errorCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  buffer,
                  1024,
                  NULL
    );
    fprintf(stderr, "[%s] failed with: [%d] %s", fun, errorCode, buffer);
}

DWORD XcpHw_UIThread(LPVOID param)
{
    HANDLE * quit_event = (HANDLE *)param;

    XCP_FOREVER {
        if (!XcpHw_MainFunction(quit_event)) {
            break;
        }
        if (WaitForSingleObject(*quit_event, 0) == WAIT_OBJECT_0) {
            break;
        }
    }
    ExitThread(0);
}


void XcpTl_PostQuitMessage();

bool XcpHw_MainFunction(HANDLE * quit_event)
{
    HANDLE hStdin;
    DWORD cNumRead, fdwMode, idx;
    INPUT_RECORD irInBuf[128];
    KEY_EVENT_RECORD key;

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE) {
        Win_ErrorMsg("GetStdHandle", GetLastError());
    }

    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (!SetConsoleMode(hStdin, fdwMode)) {
        Win_ErrorMsg("SetConsoleMode", GetLastError());
    }

    if (!GetNumberOfConsoleInputEvents (hStdin, &cNumRead)) {
        Win_ErrorMsg("PeekConsoleInput", GetLastError());
    } else {
        if (cNumRead) {
            if (!ReadConsoleInput(hStdin, irInBuf, 128, &cNumRead)) {
                Win_ErrorMsg("ReadConsoleInput", GetLastError());
            }
            for (idx = 0; idx < cNumRead; ++idx) {
            switch(irInBuf[idx].EventType) {
                    case KEY_EVENT:
                        key = irInBuf[idx].Event.KeyEvent;
//                        printf("KeyEvent: %x %x %x %u\n", key.wVirtualKeyCode, key.wVirtualScanCode, key.dwControlKeyState, key.bKeyDown);
                        if (key.bKeyDown) {
                            if (key.wVirtualKeyCode == VK_ESCAPE) {
                                SetEvent(*quit_event);
                                XcpTl_PostQuitMessage();
                                return XCP_FALSE;
                            }
                            if (key.wVirtualKeyCode == VK_F9) {
//                                printf("\tF9\n");
                            }
                            switch (tolower(key.uChar.AsciiChar)) {
                                case 'q':
                                    SetEvent(*quit_event);
                                    XcpTl_PostQuitMessage();
                                    return XCP_FALSE;
                                case 'h':
                                    DisplayHelp();
                                    break;
                                case 'i':
                                    SystemInformation();
                                    break;
                                case 'd':
                                    XcpDaq_PrintDAQDetails();
                                    break;
                            }
                        }
                        break;
                    default:
                        //MyErrorExit("unknown event type");
                        break;
                }
            }
        }
    }
    return XCP_TRUE;
}


void XcpHw_GetCommandLineOptions(XcpHw_OptionsType * options)
{
    int idx;
    char * arg;

    options->ipv6 = XCP_FALSE;
    options->tcp = XCP_TRUE;

    if (__argc >= 2) {
        for(idx = 1; idx < __argc; ++idx) {
            arg = __argv[idx];
            if ((arg[0] != '/') && (arg[0] != '-')) {
                continue;
            }
            switch (arg[1]) {
                case '4':
                    options->ipv6 = XCP_FALSE;
                    break;
                case '6':
                    options->ipv6 = XCP_TRUE;
                    break;
                case 'u':
                    options->tcp = XCP_FALSE;
                    break;
                case 't':
                    options->tcp = XCP_TRUE;
                    break;
                case 'h':
                    break;
                default:
                    break;
            }
        }
    }
}

static void DisplayHelp(void)
{
    printf("\nh\t\tshow this help message\n");
    printf("<ESC> or q\texit %s\n", __argv[0]);
    printf("i\t\tsystem information\n");
    printf("d\t\tDAQ configuration\n");
    /* printf("d\t\tReset connection\n"); */
}

static void SystemInformation(void)
{
#if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_StateType * state;
#endif /* XCP_ENABLE_STATISTICS */

    printf("\nSystem-Information\n");
    printf("------------------\n");
    XcpTl_PrintConnectionInformation();
    printf("MAX_CTO         : %d    MAX_DTO: %d\n", XCP_MAX_CTO, XCP_MAX_DTO);
    printf("Slave-Blockmode : %s\n", (XCP_SLAVE_BLOCK_MODE == XCP_ON) ? "Yes" : "No");
    printf("Master-Blockmode: %s\n", (XCP_MASTER_BLOCK_MODE == XCP_ON) ? "Yes" : "No");

#if XCP_ENABLE_CAL_COMMANDS == XCP_ON
    printf("Calibration     : Yes   Protected: %s\n", (XCP_PROTECT_CAL == XCP_ON)  ? "Yes" : "No");
#else
    printf("Calibration     : No\n");
#endif /* XCP_ENABLE_CAL_COMMANDS */

#if XCP_ENABLE_PAG_COMMANDS == XCP_ON
    printf("Paging          : Yes   Protected: %s\n", (XCP_PROTECT_PAG == XCP_ON)  ? "Yes" : "No");
#else
    printf("Paging          : No\n");
#endif /* XCP_ENABLE_PAG_COMMANDS */

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    printf("DAQ             : Yes   Protected: (DAQ: %s STIM: %s)\n", (XCP_PROTECT_DAQ == XCP_ON)  ?
           "Yes" : "No", (XCP_PROTECT_STIM == XCP_ON)  ? "Yes" : "No"
    );
#else
    printf("DAQ             : No\n");
#endif /* XCP_ENABLE_DAQ_COMMANDS */

#if XCP_ENABLE_PGM_COMMANDS
    printf("Programming     : Yes   Protected: %s\n", (XCP_PROTECT_PGM == XCP_ON)  ? "Yes" : "No");
#else
    printf("Programming     : No\n");
#endif /* XCP_ENABLE_PGM_COMMANDS */
    printf("\n");
    XcpDaq_Info();
    FlsEmu_Info();

#if XCP_ENABLE_STATISTICS == XCP_ON
    state = Xcp_GetState();
    printf("\nStatistics\n");
    printf("----------\n");
    printf("CTOs rec'd      : %d\n", state->statistics.ctosReceived);
    printf("CROs busy       : %d\n", state->statistics.crosBusy);
    printf("CROs send       : %d\n", state->statistics.crosSend);
#endif /* XCP_ENABLE_STATISTICS */
    printf("-------------------------------------------------------------------------------\n");
}
