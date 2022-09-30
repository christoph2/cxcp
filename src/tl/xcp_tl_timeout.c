/*
 * BlueParrot XCP
 *
 * (C) 2007-2022 by Christoph Schueler <github.com/Christoph2,
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

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp_tl_timeout.h"
#include "xcp.h"
/*!!! END-INCLUDE-SECTION !!!*/

/*
 *
 * Time-out handling functions for Transport-Layer.
 *
 */

typedef void (*void_function)(void);

static uint32_t XcpTl_TimeoutValue = 0UL;
static void_function XcpTl_TimeoutFunction = XCP_NULL;
static bool XcpTl_TimeoutRunning = XCP_FALSE;

static void XcpTl_TimeoutInit(uint16_t timeout_value,
                              void (*timeout_function)(void)) {
  XcpTl_TimeoutValue = timeout_value;
  XcpTl_TimeoutRunning = XCP_FALSE;
  XcpTl_TimeoutFunction = timeout_function;
}

static void XcpTl_TimeoutStart(void) {
  XcpTl_TimeoutValue = XcpHw_GetTimerCounterMS();
  XcpTl_TimeoutRunning = XCP_TRUE;
}

static void XcpTl_TimeoutStop(void) { XcpTl_TimeoutRunning = XCP_FALSE; }

static void XcpTl_TimeoutCheck(void) {
  if (!XcpTl_TimeoutRunning) {
    return;
  }
  if ((XcpHw_GetTimerCounterMS() - XcpTl_TimeoutValue) > XcpTl_TimeoutValue) {
    if (XcpTl_TimeoutFunction != XCP_NULL) {
      XcpTl_TimeoutFunction();
    }
  }
}

static void XcpTl_TimeoutReset(void) {
  XcpTl_TimeoutValue = XcpHw_GetTimerCounterMS();
}
