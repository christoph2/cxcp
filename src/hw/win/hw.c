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
**  Local Variables.
*/
static HwStateType HwState = {0};

void exitFunc(void);


void exitFunc(void)
{
    printf("Exiting %s...\n", __argv[0]);
    _CrtDumpMemoryLeaks();
}


/*
**  Global Functions.
*/
void XcpHw_Init(void)
{
    atexit(exitFunc);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    QueryPerformanceFrequency(&HwState.TicksPerSecond);
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


void Win_ErrorMsg(char * const fun, unsigned errorCode)
{
    //LPWSTR buffer = NULL;
    char * buffer = XCP_NULL;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, XCP_NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &buffer, 0, XCP_NULL);
    if (buffer != XCP_NULL) {
        fprintf(stderr, "[%s] failed with: [%d] %s", fun, errorCode, buffer);
        LocalFree((HLOCAL)buffer);
    } else {
        //DBG_PRINT("FormatMessage failed!\n");
    }
}


void XcpHw_MainFunction(bool * finished)
{
    HANDLE hStdin;
    DWORD cNumRead, fdwMode, idx;
    INPUT_RECORD irInBuf[128];
    KEY_EVENT_RECORD key;

    *finished = XCP_FALSE;

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
                                *finished = XCP_TRUE;
                                return;
                            }
                            if (key.wVirtualKeyCode == VK_F9) {
//                                printf("\tF9\n");
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
