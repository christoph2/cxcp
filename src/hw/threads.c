/*
 * BlueParrot XCP
 *
 * (C) 2021-2025 by Christoph Schueler <github.com/Christoph2,
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

#if defined(_WIN32)
    #include <process.h>
    #include <windows.h>
#else
    #define _GNU_SOURCE
    #include <pthread.h>
    #include <sched.h>
#endif

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
** NOTE: Atomics require at least C11.
*/
#if !defined(__STDC_NO_ATOMICS__)

    #if defined(_MSC_VER)
typedef bool atomic_bool;
    #else
        #include <stdatomic.h>
    #endif

#endif /* __STDC_NO_ATOMICS__ */

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_hw.h"
#include "xcp_threads.h"
/*!!! END-INCLUDE-SECTION !!!*/

#define XCP_THREAD (0)
#define UI_THREAD  (1)
#define APP_THREAD (2)
#define TL_THREAD  (3)

#define NUM_THREADS (4)

typedef void *(*XcpThrd_ThreadFuncType)(void *);

#if defined(_WIN32)
typedef HANDLE XcpThrd_ThreadType;
#else
typedef pthread_t XcpThrd_ThreadType;
#endif

XcpThrd_ThreadType threads[NUM_THREADS];

static void XcpThrd_CreateThread(XcpThrd_ThreadType *thrd, XcpThrd_ThreadFuncType func);

static void XcpThrd_SetAffinity(XcpThrd_ThreadType thrd, int cpu);

#if defined(__STDC_NO_ATOMICS__)
static bool XcpThrd_ShuttingDown;
#else
static atomic_bool XcpThrd_ShuttingDown;
#endif /* __STDC_NO_ATOMICS__ */

void bye(void);

#if defined(_WIN32)
static unsigned __stdcall XcpThrd_WinTrampoline(void *arg) {
    XcpThrd_ThreadFuncType func = (XcpThrd_ThreadFuncType)arg;
    (void)func(NULL);
    return 0U;
}
#endif


static void XcpThrd_CreateThread(XcpThrd_ThreadType *thrd, XcpThrd_ThreadFuncType func) {
    #if defined(_WIN32)
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, XcpThrd_WinTrampoline, (void *)func, 0, NULL);
    if (hThread == NULL) {
        XcpHw_ErrorMsg("_beginthreadex()", GetLastError());
        return;
    }
    CopyMemory(thrd, &hThread, sizeof(XcpThrd_ThreadType));
    XcpThrd_SetAffinity(hThread, 1);
    #else
    int rc = pthread_create(thrd, NULL, func, NULL);
    if (rc != 0) {
        XcpHw_ErrorMsg("pthread_create()", rc);
        return;
    }
    XcpThrd_SetAffinity(*thrd, 1);
    #endif
}

void XcpThrd_RunThreads(void) {
    atexit(bye);

    XcpThrd_ShuttingDown = false;

    XcpThrd_CreateThread(&threads[UI_THREAD], XcpTerm_Thread);
    XcpThrd_CreateThread(&threads[TL_THREAD], XcpTl_Thread);
    XcpThrd_CreateThread(&threads[XCP_THREAD], Xcp_Thread);
    #if defined(_WIN32)
    WaitForSingleObject(threads[UI_THREAD], INFINITE);
    XcpThrd_ShutDown();
    /* Warte auf die anderen Threads */
    if (threads[TL_THREAD]) {
        WaitForSingleObject(threads[TL_THREAD], INFINITE);
    }
    if (threads[XCP_THREAD]) {
        WaitForSingleObject(threads[XCP_THREAD], INFINITE);
    }
    #else
    pthread_join(threads[UI_THREAD], NULL);
    XcpThrd_ShutDown();
    /* Warte auf die anderen Threads */
    pthread_join(threads[TL_THREAD], NULL);
    pthread_join(threads[XCP_THREAD], NULL);
    #endif
}


void XcpThrd_Exit(void) {
#if defined(_WIN32)
    ExitThread(0);
#else
    pthread_exit(NULL);
#endif
}

void *Xcp_Thread(void *param) {
    XCP_UNREFERENCED_PARAMETER(param);
    XCP_FOREVER {
        if (XcpThrd_IsShuttingDown()) {
            break;
        }
        Xcp_MainFunction();
    }
    return NULL;
}


void XcpThrd_EnableAsyncCancellation(void) {
    #if defined(_WIN32)
    /* Keine direkte Entsprechung unter Windows, ggf. SetThreadPriority/Waitable-Mechanik verwenden. */
    #else
    int res;

    res = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_setcancelstate()", errno);
    }
    res = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_setcanceltype()", errno);
    }
    #endif
}


void XcpThrd_ShutDown(void) {
    int res;

    printf("Shutdown RQ.\n");
    if (XcpThrd_IsShuttingDown()) {
        return;
    }
    XcpThrd_ShuttingDown = true;

    /* TL-Thread aus Blockaden holen */
    #if defined(_WIN32)
    /* Fallback: hartes Beenden, wenn TL-Thread blockiert (accept/recv). */
    res = TerminateThread(threads[TL_THREAD], 0);
    if (!res) {
        XcpHw_ErrorMsg("TerminateThread", GetLastError());
    }
    /* XCP-Thread läuft kooperativ aus (Flag oben gesetzt). */
    #else
    /* POSIX: asynchron canceln, anschließend joinen in RunThreads. */
    res = pthread_cancel(threads[TL_THREAD]);
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_cancel(TL_THREAD)", errno);
    }
    res = pthread_cancel(threads[XCP_THREAD]);
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_cancel(XCP_THREAD)", errno);
    }
    #endif
}


bool XcpThrd_IsShuttingDown(void) {
    return XcpThrd_ShuttingDown;
}

void bye(void) {
    printf("Exiting program.\n");
}

static void XcpThrd_SetAffinity(XcpThrd_ThreadType thrd, int cpu) {
    #if defined(_WIN32)
    DWORD_PTR mask = (cpu <= 0) ? 1ULL : (1ULL << cpu);
    if (SetThreadAffinityMask(thrd, mask) == 0) {
        XcpHw_ErrorMsg("SetThreadAffinityMask()", GetLastError());
    }
    #else

    #if !defined(__APPLE__)
    /* Linux only (i.e. no BSD) ??? */
    int res;

    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    res = pthread_setaffinity_np(thrd, sizeof(cpu_set_t), &cpuset);
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_setaffinity_np()", errno);
    }

    res = pthread_getaffinity_np(thrd, sizeof(cpu_set_t), &cpuset);
    if (res != 0) {
        XcpHw_ErrorMsg("pthread_getaffinity_np()", errno);
    }
    #endif

    #endif
}


