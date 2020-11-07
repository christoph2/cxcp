/*
 * BlueParrot XCP
 *
 * (C) 2007-2020 by Christoph Schueler <github.com/Christoph2,
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

#if !defined(__XCP_TUI_H)
#define __XCP_TUI_H

#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
extern "C"
{
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */

void XcpTui_Init(void);
void XcpTui_Deinit(void);
void * XcpTui_MainFunction(void * param);

#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
}
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */


#endif /* __XCP_TUI_H */
