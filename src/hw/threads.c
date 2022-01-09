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

#define _GNU_SOURCE
#include <pthread.h>
#include <intrin.h>
#include <sched.h>
#include <signal.h>
#include <intrin.h>

#include "xcp.h"
#include "xcp_hw.h"
#include "xcp_threads.h"

#define XCP_THREAD  (0)
#define UI_THREAD   (1)
#define APP_THREAD  (2)
#define TL_THREAD   (3)

#define NUM_THREADS (4)

pthread_t threads[NUM_THREADS];

static short XcpThrd_ShuttingDown __attribute__((aligned(8)))  = 0L;

void bye(void);

void XcpThrd_RunThreads(void)
{
    atexit(bye);
    pthread_create(&threads[UI_THREAD], NULL, &XcpTerm_Thread, NULL);
    pthread_create(&threads[TL_THREAD], NULL, &XcpTl_Thread, NULL);
    pthread_create(&threads[XCP_THREAD], NULL, &Xcp_Thread, NULL);
    pthread_join(threads[UI_THREAD], NULL);
    pthread_kill(threads[TL_THREAD], SIGINT);
    pthread_kill(threads[XCP_THREAD], SIGINT);
}


void bye(void)
{
    printf("Exiting program.\n");
}

#if 0
void XcpThrd_SetAffinity(pthread_t thrd, int cpu)
{
    int res;
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);

    CPU_SET(0, &cpuset);

    res = pthread_setaffinity_np(thrd, sizeof(cpu_set_t), &cpuset);
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_setaffinity_np()", errno);
    }

    res = pthread_getaffinity_np(thrd, sizeof(cpu_set_t), &cpuset);
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_getaffinity_np()", errno);
    }
}
#endif

void * Xcp_Thread(void * param)
{
    XCP_UNREFERENCED_PARAMETER(param);
    XCP_FOREVER {
        Xcp_MainFunction();
    }
    return NULL;
}

void XcpThrd_EnableAsyncCancellation(void)
{
    int res;

    res =  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_setcancelstate()", errno);
    }
}

void XcpThrd_ShutDown(void)
{
    int res;

    /* printf("Shutdown RQ.\n"); */
    if (XcpThrd_IsShuttingDown() > 0) {
        return;
    }
    res = pthread_cancel(threads[TL_THREAD]);   // Due to blocking accept().
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_cancel()", errno);
    }
    _InterlockedIncrement16(&XcpThrd_ShuttingDown);
}

bool XcpThrd_IsShuttingDown(void)
{
    return _InterlockedCompareExchange16(&XcpThrd_ShuttingDown, 0, 0);
}
