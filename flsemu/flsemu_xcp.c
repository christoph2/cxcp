/*
 * BlueParrot XCP
 *
 * (C) 2007-2026 by Christoph Schueler <github.com/Christoph2,
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

#include <string.h>

#include "flsemu.h"
#include "xcp.h"
#include "xcp_hw.h"

#if (XCP_ENABLE_CAL_COMMANDS == XCP_ON) || (XCP_ENABLE_PAG_COMMANDS == XCP_ON)

bool XcpHw_SetCalPage(uint8_t segment, uint8_t page, uint8_t mode) {
    if (!FlsEmu_Initialized()) {
        return false;
    }
    if (segment >= FlsEmu_GetConfig()->numSegments) {
        return false;
    }
    if (page >= FlsEmu_NumPages(segment)) {
        return false;
    }
    /* Mode: 0x01 = ECU, 0x02 = XCP, 0x03 = both. In this emulation we only have one active page per segment. */
    FlsEmu_SelectPage(segment, page);
    return true;
}

bool XcpHw_GetCalPage(uint8_t segment, uint8_t mode, uint8_t *page) {
    if (!FlsEmu_Initialized()) {
        return false;
    }
    if (segment >= FlsEmu_GetConfig()->numSegments) {
        return false;
    }
    *page = FlsEmu_GetConfig()->segments[segment]->currentPage;
    return true;
}

bool XcpHw_GetPagProcessorInfo(XcpHw_PagProcessorInfoType *info) {
    if (!FlsEmu_Initialized()) {
        return false;
    }
    info->maxSegment = FlsEmu_GetConfig()->numSegments;
    info->properties = 0x01; /* FREEZE_SUPPORTED */
    return true;
}

bool XcpHw_GetSegmentInfo(uint8_t segment, uint8_t mode, XcpHw_SegmentInfoType *info) {
    FlsEmu_SegmentType const *seg;
    (void)mode;
    if (!FlsEmu_Initialized()) {
        return false;
    }
    if (segment >= FlsEmu_GetConfig()->numSegments) {
        return false;
    }
    seg           = FlsEmu_GetConfig()->segments[segment];
    info->address = (uint32_t)seg->baseAddress;
    info->length  = (uint32_t)seg->pageSize;  // NOTE: memSize is the total size of the segment.
    if (seg->type == FLSEMU_RAM) {
        info->numPages = 1;
    } else {
        info->numPages = (uint8_t)FlsEmu_NumPages(segment);
    }
    info->addressExtension  = seg->addressExtension;
    info->maxPage           = info->numPages - 1;
    info->compressionMethod = 0;
    info->encryptionMethod  = 0;
    return true;
}

bool XcpHw_GetPageInfo(uint8_t segment, uint8_t page, XcpHw_PageInfoType *info) {
    if (!FlsEmu_Initialized()) {
        return false;
    }
    if (segment >= FlsEmu_GetConfig()->numSegments) {
        return false;
    }
    if (page >= FlsEmu_NumPages(segment)) {
        return false;
    }
    info->properties  = 0x03; /* ECU_ACCESS_READ_WRITE | XCP_ACCESS_READ_WRITE */
    info->initSegment = 0;
    return true;
}

bool XcpHw_SetSegmentMode(uint8_t segment, uint8_t mode) {
    /* Not implemented in flsemu */
    return true;
}

bool XcpHw_GetSegmentMode(uint8_t segment, uint8_t *mode) {
    *mode = 0; /* Normal mode */
    return true;
}

bool XcpHw_CopyCalPage(uint8_t srcSegment, uint8_t srcPage, uint8_t dstSegment, uint8_t dstPage) {
    FlsEmu_SegmentType *srcSeg;
    FlsEmu_SegmentType *dstSeg;
    uint8_t            *srcPtr;
    uint8_t            *dstPtr;

    if (!FlsEmu_Initialized()) {
        return false;
    }
    if (srcSegment >= FlsEmu_GetConfig()->numSegments || dstSegment >= FlsEmu_GetConfig()->numSegments) {
        return false;
    }
    if (srcPage >= FlsEmu_NumPages(srcSegment) || dstPage >= FlsEmu_NumPages(dstSegment)) {
        return false;
    }
    srcSeg = FlsEmu_GetConfig()->segments[srcSegment];
    dstSeg = FlsEmu_GetConfig()->segments[dstSegment];

    if (srcSeg->pageSize != dstSeg->pageSize) {
        return false;
    }

    /* We need to be careful with currently mapped pages.
       FlsEmu_BasePointer returns the pointer to the CURRENTLY mapped page.
       This is a bit tricky since we might want to copy from/to pages that are NOT currently mapped.
    */
    /* Temporary hack: Select pages, copy, then restore. */
    uint8_t oldSrcPage = srcSeg->currentPage;
    uint8_t oldDstPage = dstSeg->currentPage;

    FlsEmu_SelectPage(srcSegment, srcPage);
    srcPtr = (uint8_t *)FlsEmu_BasePointer(srcSegment);

    FlsEmu_SelectPage(dstSegment, dstPage);
    dstPtr = (uint8_t *)FlsEmu_BasePointer(dstSegment);

    memcpy(dstPtr, srcPtr, srcSeg->pageSize);

    FlsEmu_SelectPage(srcSegment, oldSrcPage);
    FlsEmu_SelectPage(dstSegment, oldDstPage);

    return true;
}

#endif /* XCP_ENABLE_CAL_COMMANDS || XCP_ENABLE_PAG_COMMANDS */
