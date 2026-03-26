/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
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

#if !defined(__XCP_HW_H)
    #define __XCP_HW_H

    #if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
        #if defined(__cplusplus)
extern "C" {

        #endif /* __cplusplus */
    #endif     /* XCP_EXTERN_C_GUARDS */

    /*!!! START-INCLUDE-SECTION !!!*/
    #include "xcp.h"
    #include "xcp_task.h"
    /*!!! END-INCLUDE-SECTION !!!*/

    /*
    **  Global Functions.
    */
    void XcpHw_ParseCommandLineOptions(int argc, char **argv, Xcp_OptionsType *options);
    void XcpHw_ErrorMsg(char * const function, int errorCode);
    void XcpHw_Sleep(uint64_t usec);

    /*
    **  Calibration/Paging Services (PAG).
    */
    #if (XCP_ENABLE_CAL_COMMANDS == XCP_ON) || (XCP_ENABLE_PAG_COMMANDS == XCP_ON)

    typedef struct tagXcpHw_PagProcessorInfoType {
        uint8_t maxSegment;
        uint8_t properties;
    } XcpHw_PagProcessorInfoType;

    typedef struct tagXcpHw_SegmentInfoType {
        uint32_t address;
        uint32_t length;
        uint8_t  numPages;
        uint8_t  addressExtension;
        uint8_t  maxPage;
        uint8_t  compressionMethod;
        uint8_t  encryptionMethod;
    } XcpHw_SegmentInfoType;

    typedef struct tagXcpHw_PageInfoType {
        uint8_t properties;
        uint8_t initSegment;
    } XcpHw_PageInfoType;

    bool XcpHw_SetCalPage(uint8_t segment, uint8_t page, uint8_t mode);
    bool XcpHw_GetCalPage(uint8_t segment, uint8_t mode, uint8_t *page);
    bool XcpHw_GetPagProcessorInfo(XcpHw_PagProcessorInfoType *info);
    bool XcpHw_GetSegmentInfo(uint8_t segment, uint8_t mode, XcpHw_SegmentInfoType *info);
    bool XcpHw_GetPageInfo(uint8_t segment, uint8_t page, XcpHw_PageInfoType *info);
    bool XcpHw_SetSegmentMode(uint8_t segment, uint8_t mode);
    bool XcpHw_GetSegmentMode(uint8_t segment, uint8_t *mode);
    bool XcpHw_CopyCalPage(uint8_t srcSegment, uint8_t srcPage, uint8_t dstSegment, uint8_t dstPage);

    #endif /* XCP_ENABLE_CAL_COMMANDS || XCP_ENABLE_PAG_COMMANDS */

    #if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
        #if defined(__cplusplus)
}
        #endif /* __cplusplus */
    #endif     /* XCP_EXTERN_C_GUARDS */

#endif /* __XCP_HW_H */
