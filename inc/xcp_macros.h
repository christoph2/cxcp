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


/** @file xcp_macros.h
 *  @brief Common macros and function like macros.
 *
 *  All macros are prefixed with **XCP_** to prevent collisions
 *  esp. with Windows.h
 *
 *  @author Christoph Schueler (cpu12.gems@googlemail.com)
 */

#if !defined(__XCP_MACROS_H)
#define __XCP_MACROS_H


#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
extern "C"
{
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */


#define XCP_DEBUG_BUILD         (1)
#define XCP_RELEASE_BUILD       (2)

#include "xcp_config.h"

#if defined(_WIN32) || (XCP_BUILD_TYPE == XCP_DEBUG_BUILD)
#include <stdio.h>
#endif /* defined(_WIN32) */

#if !defined(XCP_LOBYTE)
#define XCP_LOBYTE(w)   ((uint8_t)((w) & (uint8_t)0xff))    /**< Get the low-byte from a word. */
#endif

#if !defined(XCP_HIBYTE)
#define XCP_HIBYTE(w)   ((uint8_t)(((w)  & (uint16_t)0xff00U) >> 8U)) /**< Get the high-byte from a word. */
#endif

#if !defined(XCP_LOWORD)
#define XCP_LOWORD(w)   ((uint16_t)((w) & (uint16_t)0xffff))    /**< Get the low-word from a double-word. */
#endif

#if !defined(XCP_HIWORD)
#define XCP_HIWORD(w)   ((uint16_t)(((w)  & (uint32_t)0xffff0000) >> 16U))   /**< Get the high-word from a doubleword. */
#endif

#if !defined(XCP_MAKEWORD)
#define XCP_MAKEWORD(h, l) ((((uint16_t)((h) & ((uint8_t)0xff))) <<  (uint16_t)8) | ((uint16_t)((l) & ((uint8_t)0xff)))) /**< Make word from high and low bytes. */
#endif

#if !defined(XCP_MAKEDWORD)
#define XCP_MAKEDWORD(h, l)  ((((uint32_t)((h) & ((uint16_t)0xffffu))) << (uint32_t)16) | ((uint32_t)((l) & ((uint16_t)0xffffu)))) /**< Make double-word from high and low words. */
#endif

#if !defined(XCP_MAX)
#define XCP_MAX(l, r) (((l) > (r)) ? (l) : (r))   /**< Computes the maximum of \a l and \a r. */
#endif

#if !defined(XCP_MIN)
#define XCP_MIN(l, r) (((l) < (r)) ? (l) : (r))   /**< Computes the minimum of \a l and \a r. */
#endif

#if !defined(XCP_TRUE)
#define XCP_TRUE    (1)     /**< Boolean \a TRUE value. */
#endif

#if !defined(XCP_FALSE)
#define XCP_FALSE   (0)     /**< Boolean \a FALSE value. */
#endif

#if !defined(XCP_NULL)
#if defined(__cplusplus)
#define XCP_NULL (0)
#else
#define XCP_NULL    ((void*)0)
#endif
#endif

#if XCP_BUILD_TYPE == XCP_DEBUG_BUILD
#define XCP_STATIC          /**< Static only on DEBUG builds. Rationale: (unit-) testing. */
#else
#define XCP_STATIC static
#endif /* XCP_BUILD_TYPE */

#define XCP_ON_CAN_EXT_IDENTIFIER  (0x80000000)

#if !defined(XCP_UNREFERENCED_PARAMETER)
#define XCP_UNREFERENCED_PARAMETER(x)   (x) = (x)   /*lint  -esym( 714, x ) */
#endif

#define XCP_FOREVER     for(;;)

#if XCP_BUILD_TYPE == XCP_DEBUG_BUILD
    #define INLINE
    #if defined(_WIN32)
        #define DBG_PRINT1(A)                   printf(A)
        #define DBG_PRINT2(A, B)                printf(A, B)
        #define DBG_PRINT3(A, B, C)             printf(A, B, C)
        #define DBG_PRINT4(A, B, C, D)          printf(A, B, C, D)
        #define DBG_PRINT5(A, B, C, D, E)       printf(A, B, C, D, E)
        #define DBG_PRINT6(A, B, C, D, E, F)    printf(A, B, C, D, E, F)

        #define DBG_TRACE1(A)                   DBG_PRINT1(A)
        #define DBG_TRACE2(A, B)                DBG_PRINT2(A, B)
        #define DBG_TRACE3(A, B, C)             DBG_PRINT3(A, B, C)
        #define DBG_TRACE4(A, B, C, D)          DBG_PRINT4(A, B, C, D)
        #define DBG_TRACE5(A, B, C, D, E)       DBG_PRINT5(A, B, C, D, E)
        #define DBG_TRACE6(A, B, C, D, E, F)    DBG_PRINT6(A, B, C, D, E, F)
    #else
        #define INLINE
        #define DBG_PRINT1(A)
        #define DBG_PRINT2(A, B)
        #define DBG_PRINT3(A, B, C)
        #define DBG_PRINT4(A, B, C, D)
        #define DBG_PRINT5(A, B, C, D, E)
        #define DBG_PRINT6(A, B, C, D, E, F)

        #define DBG_TRACE1(A)                   DBG_PRINT1(A)
        #define DBG_TRACE2(A, B)                DBG_PRINT2(A, B)
        #define DBG_TRACE3(A, B, C)             DBG_PRINT3(A, B, C)
        #define DBG_TRACE4(A, B, C, D)          DBG_PRINT4(A, B, C, D)
        #define DBG_TRACE5(A, B, C, D, E)       DBG_PRINT5(A, B, C, D, E)
        #define DBG_TRACE6(A, B, C, D, E, F)    DBG_PRINT6(A, B, C, D, E, F)
    #endif // defined(_WIN32)
#else
    #define INLINE
    #define DBG_PRINT1(a)
    #define DBG_PRINT2(a, b)
    #define DBG_PRINT3(a, b, c)
    #define DBG_PRINT4(a, b, c, d)
    #define DBG_PRINT5(a, b, c, d, e)
    #define DBG_PRINT6(a, b, c, d, e, f)

    #define DBG_TRACE1(a)
    #define DBG_TRACE2(a, b)
    #define DBG_TRACE3(a, b, c)
    #define DBG_TRACE4(a, b, c, d)
    #define DBG_TRACE5(a, b, c, d, e)
    #define DBG_TRACE6(a, b, c, d, e, f)
#endif /* XCP_BUILD_TYPE == XCP_DEBUG_BUILD */

#define XCP_ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))  /**< Calculates the number of elements of \a arr */


#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
}
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */


#endif /* __XCP_MACROS_H */

