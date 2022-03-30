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

#if defined(_WIN32)
#include <process.h>
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#endif

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

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

#define XCP_THREAD (0)
#define UI_THREAD (1)
#define APP_THREAD (2)
#define TL_THREAD (3)

#define NUM_THREADS (4)

typedef void (*XcpThrd_ThreadFuncType)(void *);

#if defined(_WIN32)
typedef HANDLE XcpThrd_ThreadType;
#else
typedef pthread_t XcpThrd_ThreadType;
#endif

XcpThrd_ThreadType threads[NUM_THREADS];

static void XcpThrd_CreateThread(XcpThrd_ThreadType *thrd,
                                 XcpThrd_ThreadFuncType *func);
static void XcpThrd_SetAffinity(XcpThrd_ThreadType thrd, int cpu);

#if defined(__STDC_NO_ATOMICS__)
static bool XcpThrd_ShuttingDown;
#else
static atomic_bool XcpThrd_ShuttingDown;
#endif /* __STDC_NO_ATOMICS__ */

void bye(void);

static void XcpThrd_CreateThread(XcpThrd_ThreadType *thrd,
                                 XcpThrd_ThreadFuncType *func) {
#if defined(_WIN32)
  XcpThrd_ThreadType res;
  res = (HANDLE)_beginthread(func, 0, NULL);
  CopyMemory(thrd, &res, sizeof(XcpThrd_ThreadType));
  XcpThrd_SetAffinity(res, 1);
#else
  pthread_create(thrd, NULL, func, NULL);
  XcpThrd_SetAffinity(thrd, 1);
#endif
}

void XcpThrd_RunThreads(void) {
  atexit(bye);

  XcpThrd_CreateThread(&threads[UI_THREAD], XcpTerm_Thread);
  XcpThrd_CreateThread(&threads[TL_THREAD], XcpTl_Thread);
  XcpThrd_CreateThread(&threads[XCP_THREAD], Xcp_Thread);
#if defined(_WIN32)
  WaitForSingleObject(threads[UI_THREAD], INFINITE);
  XcpThrd_ShutDown();
#else
  pthread_join(threads[UI_THREAD], NULL);
  XcpThrd_ShutDown();
  pthread_kill(threads[TL_THREAD], SIGINT);
  pthread_kill(threads[XCP_THREAD], SIGINT);
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
  XCP_FOREVER { Xcp_MainFunction(); }
  return NULL;
}

void XcpThrd_EnableAsyncCancellation(void) {
#if defined(_WIN32)

#else
  int res;

  res = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  if (res != 0) {
    XcpHw_ErrorMsg("pthread_setcancelstate()", errno);
  }
#endif
}

void XcpThrd_ShutDown(void) {
  int res;

  printf("Shutdown RQ.\n");
  if (XcpThrd_IsShuttingDown()) {
    return;
  }
  // Due to blocking accept().
#if defined(_WIN32)
  res = TerminateThread(threads[TL_THREAD], 0);
  if (!res) {
    XcpHw_ErrorMsg("TerminateThread", GetLastError());
  }
#else
  res = pthread_cancel(threads[TL_THREAD]);
  if (res != 0) {
    XcpHw_ErrorMsg("pthread_cancel()", errno);
  }
#endif
  XcpThrd_ShuttingDown = true;
}

bool XcpThrd_IsShuttingDown(void) { return XcpThrd_ShuttingDown; }

void bye(void) { printf("Exiting program.\n"); }

static void XcpThrd_SetAffinity(XcpThrd_ThreadType thrd, int cpu) {
#if defined(_WIN32)
  if (SetThreadAffinityMask(thrd, cpu) == 0) {
    XcpHw_ErrorMsg("SetThreadAffinityMask()", GetLastError());
  }
#else
  /* Linux only (i.e. no BSD) ??? */
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
#endif
}
