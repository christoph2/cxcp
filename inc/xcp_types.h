/*
 * BlueParrot XCP
 *
 * (C) 2007-2018 by Christoph Schueler <github.com/Christoph2,
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

#if !defined(__XCP_TYPES_H)
#define __XCP_TYPES_H


#if XCP_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
extern "C"
{
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */


/* check for C99-Compiler */
#if defined(__STDC_VERSION__)
    #if __STDC_VERSION__ >= 199901L
        #define C99_COMPILER
    #endif
#endif

/* check for C1x-Compiler */
#if defined(__STDC_VERSION__)
    #if __STDC_VERSION__>= 201112L
        #define C11_COMPILER
    #endif
#endif


#if defined(__CSMC__)  || !defined(C99_COMPILER) || !defined(C11_COMPILER)
typedef unsigned char       bool;

typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
typedef signed long         int32_t;
typedef unsigned long       uint32_t;

typedef signed long long    int64_t;
typedef unsigned long long  uint64_t;
#else


#include <stdbool.h>
#include <stdint.h>

#endif // defined

#define UINT8(x)    ((uint8_t)(x))
#define INT8(x)     ((int8_t)(x))

#define UINT16(x)   ((uint16_t)(x))
#define INT16(x)    ((int16_t)(x))

#define UINT32(x)   ((uint32_t)(x))
#define INT32(x)    ((int32_t)(x))

#define UINT64(x)   ((uint64_t)(x))
#define INT64(x)    ((int64_t)(x))


#if XCP_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
}
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */


#endif /* __XCP_TYPES_H */
