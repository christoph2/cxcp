/*
 * BlueParrot XCP
 *
 * (C) 2007-2021 by Christoph Schueler <github.com/Christoph2,
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

#define  _GNU_C_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
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
} HwStateType;

 /*
 ** Local Defines.
 */
#define TIMER_PS_1NS    (1UL)
#define TIMER_PS_10NS   (10UL)
#define TIMER_PS_100NS  (100UL)
#define TIMER_PS_1US    (1000UL)
#define TIMER_PS_10US   (10000UL)
#define TIMER_PS_100US  (100000UL)
#define TIMER_PS_1MS    (1000000UL)
#define TIMER_PS_10MS   (10000000UL)
#define TIMER_PS_100MS  (100000000UL)
#define TIMER_PS_1S     (1000000000UL)

#define TIMER_MASK_1    (0x000000FFUL)
#define TIMER_MASK_2    (0x0000FFFFUL)
#define TIMER_MASK_4    (0xFFFFFFFFUL)

#define SIG SIGRTMIN

/*
**  Local Function Prototypes.
*/
static void XcpHw_InitLocks(void);
static void XcpHw_DeinitLocks();
static struct timespec Timespec_Diff(struct timespec start, struct timespec end);
static void InitTUI(void);
static void DeinitTUI(void);

/*
**  External Function Prototypes.
*/

void * XcpHw_MainFunction();

bool XcpDaq_QueueEmpty(void);
bool XcpDaq_QueueDequeue(uint16_t * len, uint8_t * data);

void exitFunc(void);


/*
** Global Variables.
*/
pthread_t XcpHw_ThreadID[4];


/*
 * Local Types.
 */
#define XCPHW_APPLICATION_STATES    (32)

typedef struct tagXcpHw_ApplicationStateType {
    volatile uint8_t counter[XCPHW_APPLICATION_STATES];
} XcpHw_ApplicationStateType;


/*
**  Local Variables.
*/
static HwStateType HwState = {0};
pthread_mutex_t XcpHw_Locks[XCP_HW_LOCK_COUNT];
static struct timespec XcpHw_TimerResolution = {0};
static timer_t XcpHw_AppMsTimer;
static unsigned long long XcpHw_FreeRunningCounter = 0ULL;
static XcpHw_ApplicationStateType XcpHw_ApplicationState;
static pthread_cond_t XcpHw_TransmissionEvent;
static pthread_mutex_t XcpHw_TransmissionMutex;

/*
**  Global Functions.
*/

static void termination_handler(int sig)
{
    printf("Terminating due to ");
    switch (sig) {
        case SIGKILL:
            printf("SIGKILL");
            break;
        case SIGSTOP:
            printf("SIGSTOP");
            break;
        case SIGQUIT:
            printf("SIGQUIT");
            break;
        case SIGILL:
            printf("SIGILL");
            break;
        case SIGTRAP:
            printf("SIGTRAP");
            break;
        case SIGABRT:
            printf("SIGABRT");
            break;
        case SIGSEGV:
            printf("SIGSEGV");
            break;
        default:
            printf("Signal: %u", sig);
    }
    printf(".\n");
    exit(9);
}

static void handler(int sig, siginfo_t *si, void *uc)
{
    /* Note: calling printf() from a signal handler is not safe
     * (and should not be done in production programs), since
     * printf() is not async-signal-safe; see signal-safety(7).
     *  Nevertheless, we use printf() here as a simple way of
     * showing that the handler was called. */

    //printf("Caught signal %u %lu\n", sig, XcpHw_GetTimerCounter());

    //print_siginfo(si);
    //signal(sig, SIG_IGN);

}

void XcpHw_Init(void)
{
    int status = 0;
    struct sigevent sev = {0};
    timer_t timerid = 0;
    struct itimerspec its = {0};
    long long freq_nanosecs = 1000 * 1000 * 1000LL;
    sigset_t mask = {0};
    struct sigaction sa = {0};

    XCP_UNREFERENCED_PARAMETER(timerid);
    XCP_UNREFERENCED_PARAMETER(status);

    signal(SIGKILL, termination_handler);
    signal(SIGSTOP, termination_handler);
    signal(SIGQUIT, termination_handler);
    signal(SIGILL, termination_handler);
    signal(SIGTRAP, termination_handler);
    signal(SIGABRT, termination_handler);
    signal(SIGSEGV, termination_handler);

    XcpHw_FreeRunningCounter = 0ULL;

    if (clock_getres(CLOCK_MONOTONIC, &XcpHw_TimerResolution) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::clock_getres()", errno);
    }

    if (clock_gettime(CLOCK_MONOTONIC, &HwState.StartingTime) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::clock_gettime()", errno);
    }
    fflush(stdout);
#if 0
    _setmode(_fileno(stdout), _O_WTEXT);    /* Permit Unicode output on console */
    _setmode(_fileno(stdout), _O_U8TEXT);    /* Permit Unicode output on console */
#endif

    /* Establish handler for timer signal */
    //printf("Establishing handler for signal %d\n", SIG);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::sigaction()", errno);
    }

    /* Block timer signal temporarily */
    //printf("Blocking signal %d\n", SIG);
    sigemptyset(&mask);
    sigaddset(&mask, SIG);
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::sigprocmask()", errno);
    }

    /* Create the timer */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &XcpHw_AppMsTimer;
    // Parameter sevp could be NULL!?
    if (timer_create(CLOCK_MONOTONIC, &sev, &XcpHw_AppMsTimer) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::timer_create()", errno);
    }

    /* Start the timer */
    its.it_value.tv_sec = freq_nanosecs / 1000000000;
    its.it_value.tv_nsec = freq_nanosecs % 1000000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
    if (timer_settime(XcpHw_AppMsTimer, 0, &its, NULL) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::timer_settime()", errno);
    }

    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::sigprocmask()", errno);
    }
    XcpHw_InitLocks();
    pthread_cond_init(&XcpHw_TransmissionEvent, NULL);
    pthread_mutex_init(&XcpHw_TransmissionMutex, NULL);
}

void XcpHw_Deinit(void)
{
    XcpHw_DeinitLocks();
    pthread_cond_destroy(&XcpHw_TransmissionEvent);
    pthread_cond_destroy(&XcpHw_TransmissionEvent);
    pthread_mutex_destroy(&XcpHw_TransmissionMutex);
}

static struct timespec Timespec_Diff(struct timespec start, struct timespec end)
{
    struct timespec temp;

    if ((end.tv_nsec-start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec-start.tv_sec - 1;
        temp.tv_nsec = 1000000000L + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

uint32_t XcpHw_GetTimerCounter(void)
{
    struct timespec now = {0};
    struct timespec dt = {0};
    unsigned long long timestamp = 0ULL;

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &now) == -1) {
        XcpHw_ErrorMsg("XcpHw_Init::clock_getres()", errno);
    }

    dt = Timespec_Diff(HwState.StartingTime, now);

    XcpHw_FreeRunningCounter = ((unsigned long long)(dt.tv_sec) * (unsigned long long)1000 * 1000 * 1000) + ((unsigned long long)dt.tv_nsec);

    timestamp = XcpHw_FreeRunningCounter;
//    printf("\tTS: %llu\n", XcpHw_FreeRunningCounter);
#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1NS
    timestamp /= TIMER_PS_1NS;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10NS
    timestamp /= TIMER_PS_10NS;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100NS
    timestamp /= TIMER_PS_100NS;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
    timestamp /= TIMER_PS_1US;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10US
    timestamp /= TIMER_PS_10US;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100US
    timestamp /= TIMER_PS_100US;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
    timestamp /= TIMER_PS_1MS;
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_10MS
    timestamp /= TIMER_PS_10MS;
//#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_100MS
//    timestamp /= TIMER_PS_100MS;
#else
#error Timestamp-unit not supported.
#endif // XCP_DAQ_TIMESTAMP_UNIT

    timestamp = XcpHw_FreeRunningCounter % UINT_MAX;

#if XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_1
    timestamp &= TIMER_MASK_1;
#elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_2
    timestamp &= TIMER_MASK_2;
#elif XCP_DAQ_TIMESTAMP_SIZE == XCP_DAQ_TIMESTAMP_SIZE_4
    timestamp &= TIMER_MASK_4;
#else
#error Timestamp-size not supported.
#endif // XCP_DAQ_TIMESTAMP_SIZE

    return (uint32_t)timestamp;
}

void XcpHw_TransmitDtos(void)
{
    uint16_t len;
    uint8_t data[XCP_MAX_DTO + XCP_TRANSPORT_LAYER_BUFFER_OFFSET];
    uint8_t * dataOut = Xcp_GetDtoOutPtr();

    while (!XcpDaq_QueueEmpty()) {
        XcpDaq_QueueDequeue(&len, dataOut);
        //printf("\tDTO -- len: %d data: \t", len);
        Xcp_SetDtoOutLen(len);
        Xcp_SendDto();
    }
}

static void XcpHw_InitLocks(void)
{
    uint8_t idx = UINT8(0);

    for (idx = UINT8(0); idx < XCP_HW_LOCK_COUNT; ++idx) {
        pthread_mutex_init(&XcpHw_Locks[idx], NULL);
    }
}

static void XcpHw_DeinitLocks(void)
{
    uint8_t idx = UINT8(0);

    for (idx = UINT8(0); idx < XCP_HW_LOCK_COUNT; ++idx) {
        pthread_mutex_destroy(&XcpHw_Locks[idx]);
    }
}

void XcpHw_AcquireLock(uint8_t lockIdx)
{
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    pthread_mutex_lock(&XcpHw_Locks[lockIdx]);
}

void XcpHw_ReleaseLock(uint8_t lockIdx)
{
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    pthread_mutex_unlock(&XcpHw_Locks[lockIdx]);
}

void XcpHw_SignalTransmitRequest(void)
{
    pthread_mutex_lock(&XcpHw_TransmissionMutex);
    pthread_cond_signal(&XcpHw_TransmissionEvent);
}

void XcpHw_WaitTransmitRequest(void)
{
    pthread_cond_wait(&XcpHw_TransmissionEvent, &XcpHw_TransmissionMutex);
}

void XcpHw_ErrorMsg(char * const fun, int errorCode)
{
    fprintf(stderr, "[%s] failed with: [%d]\n", fun, errorCode);
}

/*
 *
 * Sleep for `usec` microseconds.
 *
 */
void XcpHw_Sleep(uint64_t usec)
{
    usleep(usec);
}
