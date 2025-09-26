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

#if !defined(__XCP_THREADS_H)
    #define __XCP_THREADS_H

void XcpThrd_RunThreads(void);

void XcpThrd_Exit(void);
void XcpThrd_ShutDown(void);
bool XcpThrd_IsShuttingDown(void);
void XcpThrd_EnableAsyncCancellation(void);

void *Xcp_Thread(void *param);
void *XcpTerm_Thread(void *param);
void *XcpTl_Thread(void *param);

#endif /* __XCP_THREADS_H */
