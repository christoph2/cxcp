/*
 * BlueParrot XCP
 *
 * (C) 2021 by Christoph Schueler <github.com/Christoph2,
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

#include "terminal.h"

#if defined(_WIN32)
#include <windows.h>
#else

#endif


static void SystemInformation(void);
static void DisplayHelp(void);


#if defined(_WIN32)
void * XcpTerm_Thread(void * param)
{
    HANDLE hStdin;
    DWORD cNumRead, fdwMode, idx;
    INPUT_RECORD irInBuf[128];
    KEY_EVENT_RECORD key;

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE) {
        XcpHw_ErrorMsg("GetStdHandle", GetLastError());
    }

    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (!SetConsoleMode(hStdin, fdwMode)) {
        XcpHw_ErrorMsg("SetConsoleMode", GetLastError());
    }

    XCP_FOREVER {
        WaitForSingleObject(hStdin, 1000);

        if (!GetNumberOfConsoleInputEvents (hStdin, &cNumRead)) {
            XcpHw_ErrorMsg("PeekConsoleInput", GetLastError());
        } else {
            if (cNumRead) {
                if (!ReadConsoleInput(hStdin, irInBuf, 128, &cNumRead)) {
                    XcpHw_ErrorMsg("ReadConsoleInput", GetLastError());
                }
                for (idx = 0; idx < cNumRead; ++idx) {
                switch(irInBuf[idx].EventType) {
                        case KEY_EVENT:
                            key = irInBuf[idx].Event.KeyEvent;
                            if (key.bKeyDown) {
                                if (key.wVirtualKeyCode == VK_ESCAPE) {
                                    pthread_exit(NULL);
                                }
                                switch (tolower(key.uChar.AsciiChar)) {
                                    case 'q':
                                        pthread_exit(NULL);
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
                            break;
                    }
                }
            }
        }
    }
}
#else
void * XcpTerm_Thread(void * param)
{

}
#endif // defined

static void SystemInformation(void)
{
#if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_StateType * state;
#endif /* XCP_ENABLE_STATISTICS */

    printf("\nSystem-Information\n");
    printf("------------------\n");
    XcpTl_PrintConnectionInformation();
    printf("MAX_CTO         : %d    MAX_DTO: %d\n", XCP_MAX_CTO, XCP_MAX_DTO);
    printf("Slave-Blockmode : %s\n", (XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON) ? "Yes" : "No");
    printf("Master-Blockmode: %s\n", (XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON) ? "Yes" : "No");

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
    printf("DAQ             : Yes   Protected: [DAQ: %s STIM: %s]\n", (XCP_PROTECT_DAQ == XCP_ON)  ?
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

static void DisplayHelp(void)
{
    printf("\nh\t\tshow this help message\n");
    printf("<ESC> or q\texit\n");
    printf("i\t\tsystem information\n");
    printf("d\t\tDAQ configuration\n");
    /* printf("d\t\tReset connection\n"); */
}
