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

/*
 *
 * Time-out handling functions for Transport-Layer.
 * Currently only applies to character-wise transport layers.
 * Could (should?) be generalized.
 *
 */

#ifndef __XCP_TL_TIMEOUT_H
#define __XCP_TL_TIMEOUT_H

#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp_types.h"

/*!!! END-INCLUDE-SECTION !!!*/

static void XcpTl_TimeoutInit(uint16_t timeout_value,
                              void (*timeout_function)(void));

static void XcpTl_TimeoutStart(void);

static void XcpTl_TimeoutStop(void);

static void XcpTl_TimeoutCheck(void);

static void XcpTl_TimeoutReset(void);

#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */

#endif // __XCP_TL_TIMEOUT_H
