/*
 * BlueParrot XCP
 *
 * (C) 2021-2022 by Christoph Schueler <github.com/Christoph2,
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

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdlib.h>

#include <stdbool.h>

/*
** NOTE: Atomics require at least C11.
*/
#if !defined(__STDC_NO_ATOMICS__)
    #include <stdatomic.h>
#endif /* __STDC_NO_ATOMICS__ */

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_hw.h"
#include "xcp_threads.h"
/*!!! END-INCLUDE-SECTION !!!*/

#define XCP_THREAD  (0)
#define UI_THREAD   (1)
#define APP_THREAD  (2)
#define TL_THREAD   (3)

#define NUM_THREADS (4)

pthread_t threads[NUM_THREADS];

#if defined(__STDC_NO_ATOMICS__)
static bool XcpThrd_ShuttingDown;
#else
static atomic_bool XcpThrd_ShuttingDown;
#endif  /* __STDC_NO_ATOMICS__ */

void bye(void);

void XcpThrd_RunThreads(void)
{
    atexit(bye);
    pthread_create(&threads[UI_THREAD], NULL, &XcpTerm_Thread, NULL);
    pthread_create(&threads[TL_THREAD], NULL, &XcpTl_Thread, NULL);
    pthread_create(&threads[XCP_THREAD], NULL, &Xcp_Thread, NULL);
    pthread_join(threads[UI_THREAD], NULL);
    XcpThrd_ShutDown();
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

    printf("Shutdown RQ.\n");
    if (XcpThrd_IsShuttingDown()) {
        return;
    }
    res = pthread_cancel(threads[TL_THREAD]);   // Due to blocking accept().
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_cancel()", errno);
    }
  XcpThrd_ShuttingDown = true;
}

bool XcpThrd_IsShuttingDown(void)
{
    return XcpThrd_ShuttingDown;
}
