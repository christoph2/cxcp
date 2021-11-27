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

#include <ctype.h>
#include <pthread.h>

#include "xcp.h"
#include "xcp_hw.h"
#include "terminal.h"
#include "flsemu.h"

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/select.h>
    #include <termios.h>
#endif

#if !defined(_WIN32)
struct termios orig_termios;

static void reset_terminal_mode(void);
static void set_options(void);
static void set_raw_terminal_mode(void);
static int kbhit(void);
static int getch(void);
#endif


static void SystemInformation(void);
static void DisplayHelp(void);
void FlsEmu_Info(void);
void Xcp_DisplayInfo(void);
void XcpDaq_PrintDAQDetails(void);
void XcpDaq_Info(void);


#if defined(_WIN32)
void * XcpTerm_Thread(void * param)
{
    HANDLE hStdin;
    DWORD cNumRead, fdwMode, idx;
    INPUT_RECORD irInBuf[128];
    KEY_EVENT_RECORD key;

    XCP_UNREFERENCED_PARAMETER(param);

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
static void reset_terminal_mode(void)
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

static void set_options(void)
{
    struct termios term;

    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ECHO | ISIG | IEXTEN);
    term.c_iflag &= ~(IXON | ICRNL | ISTRIP | BRKINT | INPCK);
    term.c_cflag |= CS8;
    term.c_oflag |= OPOST;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

static void set_raw_terminal_mode(void)
{
    struct termios new_termios;

    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    set_options();
    tcsetattr(0, TCSANOW, &new_termios);
}

static int kbhit(void)
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

static int getch(void)
{
    int len;
    unsigned char buf[256];

    if ((len = read(0, &buf, sizeof(buf))) < 0) {
        return -1;
    } else {
        // printf("%d %x %c\n", len, buf[0], buf[0]);
        if (len == 1) {
            // Only plain chars for now.
            return buf[0];
        } else {
            return -1;
        }
    }
}

void * XcpTerm_Thread(void * param)
{
    int key = 0;

    set_raw_terminal_mode();
    while (1) {
        while(!kbhit()) {
        }
        key = getch();
        if (key != -1 ) {
            switch (tolower(key)) {
                case '\x1b':
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
    }
}
#endif // defined

static void SystemInformation(void)
{
#if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_StateType * state;
#endif /* XCP_ENABLE_STATISTICS */

    printf("\n\rSystem-Information\n\r");
    printf("------------------\n\r");
    XcpTl_PrintConnectionInformation();
    printf("MAX_CTO         : %d    MAX_DTO: %d\n\r", XCP_MAX_CTO, XCP_MAX_DTO);
    printf("Slave-Blockmode : %s\n\r", (XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON) ? "Yes" : "No");
    printf("Master-Blockmode: %s\n\r", (XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON) ? "Yes" : "No");

#if XCP_ENABLE_CAL_COMMANDS == XCP_ON
    printf("Calibration     : Yes   Protected: %s\n\r", (XCP_PROTECT_CAL == XCP_ON)  ? "Yes" : "No");
#else
    printf("Calibration     : No\n\r");
#endif /* XCP_ENABLE_CAL_COMMANDS */

#if XCP_ENABLE_PAG_COMMANDS == XCP_ON
    printf("Paging          : Yes   Protected: %s\n\r", (XCP_PROTECT_PAG == XCP_ON)  ? "Yes" : "No");
#else
    printf("Paging          : No\n\r");
#endif /* XCP_ENABLE_PAG_COMMANDS */

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    printf("DAQ             : Yes   Protected: [DAQ: %s STIM: %s]\n\r", (XCP_PROTECT_DAQ == XCP_ON)  ?
           "Yes" : "No", (XCP_PROTECT_STIM == XCP_ON)  ? "Yes" : "No"
    );
#else
    printf("DAQ             : No\n\r");
#endif /* XCP_ENABLE_DAQ_COMMANDS */

#if XCP_ENABLE_PGM_COMMANDS
    printf("Programming     : Yes   Protected: %s\n\r", (XCP_PROTECT_PGM == XCP_ON)  ? "Yes" : "No");
#else
    printf("Programming     : No\n\r");
#endif /* XCP_ENABLE_PGM_COMMANDS */
    printf("\n\r");
    XcpDaq_Info();
    FlsEmu_Info();

#if XCP_ENABLE_STATISTICS == XCP_ON
    state = Xcp_GetState();
    printf("\n\rStatistics\n\r");
    printf("----------\n\r");
    printf("CTOs rec'd      : %d\n\r", state->statistics.ctosReceived);
    printf("CROs busy       : %d\n\r", state->statistics.crosBusy);
    printf("CROs send       : %d\n\r", state->statistics.crosSend);
#endif /* XCP_ENABLE_STATISTICS */
    printf("-------------------------------------------------------------------------------\n\r");
}

static void DisplayHelp(void)
{
    printf("\n\rh\t\tshow this help message\n\r");
    printf("<ESC> or q\texit\n\r");
    printf("i\t\tsystem information\n\r");
    printf("d\t\tDAQ configuration\n\r");
    /* printf("d\t\tReset connection\n"); */
}

void FlsEmu_Info(void)
{
    uint8_t idx;
    uint8_t * ptr;
    FlsEmu_SegmentType * segment;

    printf("\n\rFlash-Emulator\n\r");
    printf("--------------\n\r");
    printf("Segment              Mapped     Virtual    Size(KB) Pagesize(KB) #Pages\n\r");
    for (idx = 0; idx < FlsEmu_GetConfig()->numSegments; ++idx) {
        ptr = FlsEmu_BasePointer(idx);
        segment = FlsEmu_GetConfig()->segments[idx];
        printf("%-20.20s 0x%p 0x%p %8d         %4d %6d\n\r", segment->name, (void*)segment->baseAddress, ptr, segment->memSize / 1024, segment->pageSize / 1024, FlsEmu_NumPages(idx));
    }
    printf("\n\r");

}

void Xcp_DisplayInfo(void)
{
    XcpTl_PrintConnectionInformation();
    printf("Press h for help.\n\r");
    fflush(stdout);
}

void XcpDaq_PrintDAQDetails(void)
{
    XcpDaq_ListIntegerType listIdx;
    XcpDaq_ODTIntegerType odtIdx;
    XcpDaq_ODTEntryIntegerType odtEntriyIdx;
    XcpDaq_ListConfigurationType const * listConf;
    XcpDaq_ListStateType * listState;
    XcpDaq_ODTType const * odt;
    XcpDaq_ODTEntryType * entry;
    uint8_t mode;
    uint32_t total;
    XcpDaq_ODTIntegerType firstPid;

    printf("\n\rDAQ configuration\n\r");
    printf("-----------------\n\r");
    for (listIdx = (XcpDaq_ListIntegerType)0; listIdx < XcpDaq_GetListCount(); ++listIdx) {
        listState = XcpDaq_GetListState(listIdx);
        listConf = XcpDaq_GetListConfiguration(listIdx);
        mode = listState->mode;
        total = UINT16(0);
        XcpDaq_GetFirstPid(listIdx, &firstPid);
#if XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_DYNAMIC
        printf("DAQ-List #%d [%s] firstPid: %d mode: ", listIdx, (listIdx < XCP_MIN_DAQ) ? "predefined" : "dynamic",
               firstPid
        );
#elif XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_STATIC
        printf("DAQ-List #%d [%s] firstPid: %d mode: ", listIdx, (listIdx < XCP_MIN_DAQ) ? "predefined" : "static",
               firstPid
        );
#elif XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_NONE
    printf("DAQ-List #%d [predefined] firstPid: %d mode: ", listIdx, firstPid);
#endif /* XCP_DAQ_CONFIG_TYPE */

        if (mode & XCP_DAQ_LIST_MODE_DIRECTION) {
            printf("STIM ");
        } else {
            printf("DAQ ");
        }
        if (mode & XCP_DAQ_LIST_MODE_SELECTED) {
            printf("SELECTED ");
        }
        if (mode & XCP_DAQ_LIST_MODE_STARTED) {
            printf("STARTED ");
        }
        if (mode & XCP_DAQ_LIST_MODE_ALTERNATING) {
            printf("ALTERNATING ");
        }
        if (mode & XCP_DAQ_LIST_MODE_PID_OFF) {
            printf("PID_OFF ");
        }
        if (mode & XCP_DAQ_LIST_MODE_TIMESTAMP) {
            printf("TIMESTAMP ");
        }
        printf("\n\r");
        for (odtIdx = (XcpDaq_ODTIntegerType)0; odtIdx < listConf->numOdts; ++odtIdx) {
            odt = XcpDaq_GetOdt(listIdx, odtIdx);
            printf("    ODT #%d\n\r", odtIdx);
            for (odtEntriyIdx = (XcpDaq_ODTEntryIntegerType)0; odtEntriyIdx < odt->numOdtEntries; ++odtEntriyIdx) {
                entry = XcpDaq_GetOdtEntry(listIdx, odtIdx, odtEntriyIdx);
                printf("        Entry #%d [0x%08x] %d Byte(s)\n\r", odtEntriyIdx, entry->mta.address, entry->length);
                total += entry->length;
            }
        }
        printf("                          -------------\n\r");
        printf("                          %-5d Byte(s)\n\r", total);
    }
    printf("-------------------------------------------------------------------------------\n\r");
}

void XcpDaq_Info(void)
{
    Xcp_StateType const * Xcp_State;

    Xcp_State = Xcp_GetState();

    printf("DAQ\n\r---\n\r");
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    printf("Processor state       : ");
    switch (Xcp_State->daqProcessor.state) {
        case XCP_DAQ_STATE_UNINIT:
            printf("Uninitialized");
            break;
        case XCP_DAQ_STATE_CONFIG_INVALID:
            printf("Configuration invalid");
            break;
        case XCP_DAQ_STATE_CONFIG_VALID:
            printf("Configuration valid");
            break;
        case XCP_DAQ_STATE_STOPPED:
            printf("Stopped");
            break;
        case XCP_DAQ_STATE_RUNNING:
            printf("Running");
            break;
    }
    printf("\n\r");
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
    printf("Allocated DAQ entities: %d of %d\n\r", XcpDaq_EntityCount, XCP_DAQ_MAX_DYNAMIC_ENTITIES);
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

#else
    printf("\tfunctionality not supported.\n\r");
#endif
}

