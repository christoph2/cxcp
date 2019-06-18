/*
 * BlueParrot XCP
 *
 * (C) 2007-2019 by Christoph Schueler <github.com/Christoph2,
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


/** @file xcp_macros.h
 *  @brief Common macros and function like macros.
 *
 * All prefixed with **XCP_** to prevent collisions
 * esp. with Windows.h
 *
 *  @author Christoph Schueler (cpu12.gems@googlemail.com)
 */

#if !defined(__XCP_MACROS_H)
#define __XCP_MACROS_H


#if XCP_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
extern "C"
{
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */


#if defined(_WIN32) && (XCP_BUILD_TYPE == XCP_DEBUG_BUILD)
#include <stdio.h>
#endif // defined(_WIN32)

#if !defined(XCP_LOBYTE)
#define XCP_LOBYTE(w)   ((uint8_t)((w) & (uint8_t)0xff))
#endif

#if !defined(XCP_HIBYTE)
#define XCP_HIBYTE(w)   ((uint8_t)(((w)  & (uint16_t)0xff00) >> 8))
#endif

#if !defined(XCP_LOWORD)
#define XCP_LOWORD(w)   ((uint16_t)((w) & (uint16_t)0xffff))
#endif

#if !defined(XCP_HIWORD)
#define XCP_HIWORD(w)   ((uint16_t)(((w)  & (uint32_t)0xffff0000) >> 16))
#endif

#if !defined(XCP_MAX)
#define XCP_MAX(l, r) ((l) > (r) ? (l) : (r))
#endif

#if !defined(XCP_MIN)
#define XCP_MIN(l, r) ((l) < (r) ? (l) : (r))
#endif

#if !defined(XCP_TRUE)
#define XCP_TRUE    (1)
#endif

#if !defined(XCP_FALSE)
#define XCP_FALSE   (0)
#endif

#if !defined(XCP_NULL)
#if defined(__cplusplus)
#define XCP_NULL (0)
#else
#define XCP_NULL    ((void*)0)
#endif
#endif

#define XCP_ON_CAN_EXT_IDENTIFIER  (0x80000000)

#if !defined(XCP_UNREFERENCED_PARAMETER)
#define XCP_UNREFERENCED_PARAMETER(x)   (x) = (x)   /*lint  -esym( 714, x ) */
#endif


#if defined(_WIN32) && (XCP_BUILD_TYPE == XCP_DEBUG_BUILD)
#define INLINE __inline
#define DBG_PRINT1(a)                   printf(a)
#define DBG_PRINT2(a, b)                printf(a, b)
#define DBG_PRINT3(a, b, c)             printf(a, b, c)
#define DBG_PRINT4(a, b, c, d)          printf(a, b, c, d)
#define DBG_PRINT5(a, b, c, d, e)       printf(a, b, c, d, e)
#define DBG_PRINT6(a, b, c, d, e, f)    printf(a, b, c, d, e, f)
#else
#define INLINE
#define DBG_PRINT1(a)
#define DBG_PRINT2(a, b)
#define DBG_PRINT3(a, b, c)
#define DBG_PRINT4(a, b, c, d)
#define DBG_PRINT5(a, b, c, d, e)
#define DBG_PRINT6(a, b, c, d, e, f)
#endif // defined(_MSC_VER)

#define XCP_ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr[0])))


#if XCP_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
}
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */


#endif /* __XCP_MACROS_H */

