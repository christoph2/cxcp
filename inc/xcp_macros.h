/*
 * pySART - Simplified AUTOSAR-Toolkit for Python.
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

 /********************************************//**
  * \brief Common function like macros.
  *
  * All prefixed with **XCP_** to prevent collisions
  * esp. with Windows.h
  *
  *
  ***********************************************/

#if !defined(__XCP_MACROS_H)
#define __XCP_MACROS_H


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

#if !defined(XCP_UNREFERENCED_PARAMETER)
#define XCP_UNREFERENCED_PARAMETER(x)   (x) = (x)   /*lint  -esym( 714, x ) */
#endif // UNREFERENCED_PARAMETER

#endif /* __XCP_MACROS_H */
