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

#include <pthread.h>
#include <signal.h>

#include "xcp.h"
#include "xcp_hw.h"

#define XCP_THREAD  (0)
#define UI_THREAD   (1)
#define APP_THREAD  (2)
#define TL_THREAD   (3)

#define NUM_THREADS (4)

pthread_t threads[NUM_THREADS];

void XcpThrd_RunThreads(void)
{
    pthread_create(&threads[UI_THREAD], NULL, &XcpTerm_Thread, NULL);
    pthread_create(&threads[TL_THREAD], NULL, &XcpTl_Thread, NULL);
    pthread_create(&threads[XCP_THREAD], NULL, &Xcp_Thread, NULL);
    pthread_join(threads[UI_THREAD], NULL);
    pthread_kill(threads[TL_THREAD], SIGINT);
    pthread_kill(threads[XCP_THREAD], SIGINT);
}


void * Xcp_Thread(void * param)
{
    XCP_UNREFERENCED_PARAMETER(param);
    XCP_FOREVER {
        Xcp_MainFunction();
    }
    return NULL;
}
