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

/*
**	Common POSIX functionality.
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
**  Local Function Prototypes.
*/
static void XcpHw_InitLocks(void);
static void XcpHw_DeinitLocks();
static void InitTUI(void);
static void DeinitTUI(void);

/*
**  External Function Prototypes.
*/

void *XcpHw_MainFunction();

bool XcpDaq_QueueEmpty(void);
bool XcpDaq_QueueDequeue(uint16_t *len, uint8_t *data);

void exitFunc(void);

/*
** Global Variables.
*/
pthread_t XcpHw_ThreadID[4];

/*
 * Local Types.
 */
#define XCPHW_APPLICATION_STATES (32)

typedef struct tagXcpHw_ApplicationStateType {
    volatile uint8_t counter[XCPHW_APPLICATION_STATES];
} XcpHw_ApplicationStateType;

/*
**  Local Variables.
*/
pthread_mutex_t XcpHw_Locks[XCP_HW_LOCK_COUNT];
static XcpHw_ApplicationStateType XcpHw_ApplicationState;
static pthread_cond_t XcpHw_TransmissionEvent;
static pthread_mutex_t XcpHw_TransmissionMutex;


void XcpHw_PosixInit(void) {
	XcpHw_InitLocks();
    pthread_cond_init(&XcpHw_TransmissionEvent, NULL);
    pthread_mutex_init(&XcpHw_TransmissionMutex, NULL);
}

void XcpHw_Deinit(void) {
    XcpHw_DeinitLocks();
    pthread_cond_destroy(&XcpHw_TransmissionEvent);
    pthread_cond_destroy(&XcpHw_TransmissionEvent);
    pthread_mutex_destroy(&XcpHw_TransmissionMutex);
}

void XcpHw_TransmitDtos(void) {
    uint16_t len;
    uint8_t data[XCP_MAX_DTO + XCP_TRANSPORT_LAYER_BUFFER_OFFSET];
    uint8_t *dataOut = Xcp_GetDtoOutPtr();

    while (!XcpDaq_QueueEmpty()) {
        XcpDaq_QueueDequeue(&len, dataOut);
        // printf("\tDTO -- len: %d data: \t", len);
        Xcp_SetDtoOutLen(len);
        Xcp_SendDto();
    }
}

static void XcpHw_InitLocks(void) {
    uint8_t idx = UINT8(0);

    for (idx = UINT8(0); idx < XCP_HW_LOCK_COUNT; ++idx) {
        pthread_mutex_init(&XcpHw_Locks[idx], NULL);
    }
}

static void XcpHw_DeinitLocks(void) {
    uint8_t idx = UINT8(0);

    for (idx = UINT8(0); idx < XCP_HW_LOCK_COUNT; ++idx) {
        pthread_mutex_destroy(&XcpHw_Locks[idx]);
    }
}

void XcpHw_AcquireLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    pthread_mutex_lock(&XcpHw_Locks[lockIdx]);
}

void XcpHw_ReleaseLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    pthread_mutex_unlock(&XcpHw_Locks[lockIdx]);
}

void XcpHw_SignalTransmitRequest(void) {
    pthread_mutex_lock(&XcpHw_TransmissionMutex);
    pthread_cond_signal(&XcpHw_TransmissionEvent);
}

void XcpHw_WaitTransmitRequest(void) { pthread_cond_wait(&XcpHw_TransmissionEvent, &XcpHw_TransmissionMutex); }

void XcpHw_ErrorMsg(char *const fun, int errorCode) { fprintf(stderr, "[%s] failed with: [%d]\n", fun, errorCode); }

void XcpHw_Sleep(uint64_t usec) { 
	usleep(usec); 
}
