/*
 * BlueParrot XCP
 *
 * (C) 2007-2023 by Christoph Schueler <github.com/Christoph2,
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

#define _GNU_C_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_hw.h"

/*!!! END-INCLUDE-SECTION !!!*/

/*
**  Local Types.
*/

typedef struct tagHwStateType {
    struct timespec StartingTime;
bool            Initialized;
} HwStateType;

/*
** Local Defines.
*/
#define TIMER_PS_1NS   (1UL)
#define TIMER_PS_10NS  (10UL)
#define TIMER_PS_100NS (100UL)
#define TIMER_PS_1US   (1000UL)
#define TIMER_PS_10US  (10000UL)
#define TIMER_PS_100US (100000UL)
#define TIMER_PS_1MS   (1000000UL)
#define TIMER_PS_10MS  (10000000UL)
#define TIMER_PS_100MS (100000000UL)
#define TIMER_PS_1S    (1000000000UL)

/* Set timer prescaler */
// ... existing code ...
#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1NS
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_1NS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10NS
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_10NS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100NS
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_100NS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_1US)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10US
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_10US)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100US
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_100US)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_1MS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10MS
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_10MS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100MS
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_100MS)
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1S
#define XCP_HW_TIMER_PRESCALER (TIMER_PS_1S)
#else
#error Timestamp-unit not supported.
#endif  // XCP_DAQ_TIMESTAMP_UNIT


#define TIMER_MASK_1 (0x000000FFUL)
#define TIMER_MASK_2 (0x0000FFFFUL)
#define TIMER_MASK_4 (0xFFFFFFFFUL)

#define SIG SIGRTMIN

/*
**  Local Function Prototypes.
*/
static void            XcpHw_InitLocks(void);
static uint64_t        XcpHw_GetElapsedTime(uint32_t prescaler);
static void            XcpHw_DeinitLocks();
static struct timespec Timespec_Diff(struct timespec start, struct timespec end);
static void            InitTUI(void);
static void            DeinitTUI(void);

void exitFunc(void);

/*
**  Local Variables.
*/
static HwStateType        HwState               = { 0 };
static struct timespec    XcpHw_TimerResolution = { 0 };
static timer_t            XcpHw_AppMsTimer;
static unsigned long long XcpHw_FreeRunningCounter = 0ULL;
static volatile sig_atomic_t XcpHw_TimerSignalSeen = 0;

static void termination_handler(int sig) {
    const char *msg = "Terminating due to signal.\n";
    /* Use async-signal-safe functions only */
    (void)write(STDERR_FILENO, msg, (unsigned)strlen(msg));
    _exit(128 + sig);
}

static void handler(int sig, siginfo_t *si, void *uc) {
    (void)sig;
    (void)si;
    (void)uc;
    /* Only set a flag — keep handler async-signal-safe */
    XcpHw_TimerSignalSeen = 1;
}


void XcpHw_Init(void) {
    int               status        = 0;
    struct sigevent   sev           = { 0 };
    timer_t           timerid       = 0;
    struct itimerspec its           = { 0 };
    long long         freq_nanosecs = 1000 * 1000 * 1000LL;
    sigset_t          mask          = { 0 };
    struct sigaction  sa            = { 0 };

    XCP_UNREFERENCED_PARAMETER(timerid);
    XCP_UNREFERENCED_PARAMETER(status);

    signal(SIGQUIT, termination_handler);
    signal(SIGILL, termination_handler);
    signal(SIGTRAP, termination_handler);
    signal(SIGABRT, termination_handler);
    signal(SIGSEGV, termination_handler);

    XcpHw_FreeRunningCounter = 0ULL;
    XcpUtl_MemSet(&XcpHw_AppMsTimer, '\x00', sizeof(XcpHw_AppMsTimer));

    if (clock_getres(CLOCK_MONOTONIC_RAW, &XcpHw_TimerResolution) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::clock_getres()", errno);
    }

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &HwState.StartingTime) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::clock_gettime()", errno);
    }
    HwState.Initialized = true;

    fflush(stdout);
#if 0
    _setmode(_fileno(stdout), _O_WTEXT);    /* Permit Unicode output on console */
    _setmode(_fileno(stdout), _O_U8TEXT);    /* Permit Unicode output on console */
#endif

    /* Establish handler for timer signal */
    // printf("Establishing handler for signal %d\n", SIG);
    sa.sa_flags     = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::sigaction()", errno);
    }

    /* Block timer signal temporarily */
    // printf("Blocking signal %d\n", SIG);
    sigemptyset(&mask);
    sigaddset(&mask, SIG);
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::sigprocmask()", errno);
    }

    /* Create the timer */
    sev.sigev_notify          = SIGEV_SIGNAL;
    sev.sigev_signo           = SIG;
    sev.sigev_value.sival_ptr = &XcpHw_AppMsTimer;
    // Parameter sevp could be NULL!?
    if (timer_create(CLOCK_MONOTONIC, &sev, &XcpHw_AppMsTimer) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::timer_create()", errno);
    }

    /* Start the timer */
    its.it_value.tv_sec     = freq_nanosecs / 1000000000;
    its.it_value.tv_nsec    = freq_nanosecs % 1000000000;
    its.it_interval.tv_sec  = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
    if (timer_settime(XcpHw_AppMsTimer, 0, &its, NULL) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::timer_settime()", errno);
    }

    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::sigprocmask()", errno);
    }
    XcpHw_PosixInit();
}

static struct timespec Timespec_Diff(struct timespec start, struct timespec end) {
    struct timespec temp;

    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec  = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000L + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec  = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

static uint64_t XcpHw_GetElapsedTime(uint32_t prescaler) {
    struct timespec now       = { 0 };
    struct timespec dt        = { 0 };
    uint64_t        timestamp = 0ULL;

    if (!HwState.Initialized || prescaler == 0U) {
        return 0ULL;
    }

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &now) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::clock_getres()", errno);
        return 0ULL;
    }
    dt = Timespec_Diff(HwState.StartingTime, now);
    XcpHw_FreeRunningCounter =
        ((unsigned long long)(dt.tv_sec) * (unsigned long long)1000 * 1000 * 1000) + ((unsigned long long)dt.tv_nsec);
    timestamp = XcpHw_FreeRunningCounter / prescaler;
    return timestamp;
}

uint32_t XcpHw_GetTimerCounter(void) {
    uint64_t timestamp = XcpHw_GetElapsedTime(XCP_HW_TIMER_PRESCALER);

#if XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_1
    timestamp &= TIMER_MASK_1;
#elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_2
    timestamp &= TIMER_MASK_2;
#elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_4
    timestamp &= TIMER_MASK_4;
#else
    #error Timestamp-size not supported.
#endif  // XCP_DAQ_TIMESTAMP_SIZE

    return (uint32_t)timestamp;
}

uint32_t XcpHw_GetTimerCounterMS(void) {
    uint64_t timestamp = XcpHw_GetElapsedTime(TIMER_PS_1MS);

    return (uint32_t)timestamp & TIMER_MASK_4;
}
