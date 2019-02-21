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

#ifndef XCP_OW_H_INCLUDED
#define XCP_OW_H_INCLUDED

#include <Windows.h>
#include "xcp.h"

/*
**  Global Types.
*/


typedef struct tagXcp_HwFileViewType {
    HANDLE mappingHandle;
    void * mappingAddress;
} Xcp_HwFileViewType;

typedef struct tagXcp_HwMapFileType {
    HANDLE handle;
    uint64_t size;
    Xcp_HwFileViewType view;
} Xcp_HwMapFileType;

/*
**  Global Functions.
*/
bool XcpOw_MapFileOpen(char const * fname, Xcp_HwMapFileType * mf);
void XcpOw_MapFileClose(Xcp_HwMapFileType const * mf);

#endif // XCP_OW_H_INCLUDED
