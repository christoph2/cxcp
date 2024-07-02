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

#include "xcp.h"

#if defined(_MSC_VER)
    #include <stdio.h>
#endif /* _MSC_VER */

/*
** Private Options.
*/
#define XCP_ENABLE_STD_COMMANDS     XCP_ON
#define XCP_ENABLE_INTERLEAVED_MODE XCP_OFF

#define XCP_DRIVER_VERSION (10)

/*
** Local Types.
*/

/*
**  Global Variables.
*/
Xcp_PDUType Xcp_PduIn;
Xcp_PDUType Xcp_PduOut;

/*
 *  Local Constants.
 */
XCP_STATIC const Xcp_GetIdType Xcp_GetId0 = XCP_SET_ID(XCP_GET_ID_0);
XCP_STATIC const Xcp_GetIdType Xcp_GetId1 = XCP_SET_ID(XCP_GET_ID_1);

/*
** Local Variables.
*/
XCP_STATIC Xcp_ConnectionStateType Xcp_ConnectionState = XCP_DISCONNECTED;
XCP_STATIC Xcp_StateType           Xcp_State;

XCP_STATIC Xcp_SendCalloutType Xcp_SendCallout = (Xcp_SendCalloutType)XCP_NULL;

void Xcp_WriteMemory(void *dest, void *src, uint16_t count);
void Xcp_ReadMemory(void *dest, void *src, uint16_t count);

/*
** Local Macros.
*/
#define STOP_ALL       UINT8(0x00)
#define START_SELECTED UINT8(0x01)
#define STOP_SELECTED  UINT8(0x02)

#define XCP_INCREMENT_MTA(i) Xcp_State.mta.address += UINT32((i))

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    #define XCP_ASSERT_DAQ_STOPPED()                                                                                               \
        do {                                                                                                                       \
            if (Xcp_State.daqProcessor.state == XCP_DAQ_STATE_RUNNING) {                                                           \
                Xcp_SendResult(ERR_DAQ_ACTIVE);                                                                                    \
                return;                                                                                                            \
            }                                                                                                                      \
        } while (0)
#else
    #define XCP_ASSERT_DAQ_STOPPED()
#endif /* ENABLE_DAQ_COMMANDS */

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    #define XCP_ASSERT_PGM_IDLE()                                                                                                  \
        do {                                                                                                                       \
            if (Xcp_State.pgmProcessor.state == XCP_PGM_ACTIVE) {                                                                  \
                Xcp_SendResult(ERR_PGM_ACTIVE);                                                                                    \
                return;                                                                                                            \
            }                                                                                                                      \
        } while (0)
#else
    #define XCP_ASSERT_PGM_IDLE()
#endif /* ENABLE_PGM_COMMANDS */

#define XCP_ASSERT_PGM_ACTIVE()                                                                                                    \
    do {                                                                                                                           \
        if (Xcp_State.pgmProcessor.state != XCP_PGM_ACTIVE) {                                                                      \
            Xcp_SendResult(ERR_SEQUENCE);                                                                                          \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (0)

#define XCP_ASSERT_UNLOCKED(r)                                                                                                     \
    do {                                                                                                                           \
        if (Xcp_IsProtected((r))) {                                                                                                \
            Xcp_SendResult(ERR_ACCESS_LOCKED);                                                                                     \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (0)

#if XCP_ENABLE_CHECK_MEMORY_ACCESS == XCP_ON
    #define XCP_CHECK_MEMORY_ACCESS(m, l, a, p)                                                                                    \
        do {                                                                                                                       \
            if (!Xcp_HookFunction_CheckMemoryAccess((m), (l), (a), (p))) {                                                         \
                Xcp_SendResult(ERR_ACCESS_DENIED);                                                                                 \
                return;                                                                                                            \
            }                                                                                                                      \
        } while (0)
#else
    #define XCP_CHECK_MEMORY_ACCESS(mta, access)
#endif /* XCP_ENABLE_CHECK_MEMORY_ACCESS */

/*
** Local Function Prototypes.
*/
XCP_STATIC uint8_t                     Xcp_SetResetBit8(uint8_t result, uint8_t value, uint8_t flag);
XCP_STATIC Xcp_MemoryMappingResultType Xcp_MapMemory(Xcp_MtaType const *src, Xcp_MtaType *dst);
XCP_STATIC void                        Xcp_Download_Copy(uint32_t address, uint8_t ext, uint32_t len);
XCP_STATIC void                        Xcp_PositiveResponse(void);
XCP_STATIC void                        Xcp_ErrorResponse(uint8_t errorCode);
XCP_STATIC void                        Xcp_BusyResponse(void);
XCP_STATIC bool                        Xcp_IsProtected(uint8_t resource);
XCP_STATIC void                        Xcp_DefaultResourceProtection(void);

#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
XCP_STATIC bool Xcp_SlaveBlockTransferIsActive(void);
XCP_STATIC void Xcp_SlaveBlockTransferSetActive(bool onOff);
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */

XCP_STATIC void Xcp_SendResult(Xcp_ReturnType result);

XCP_STATIC void Xcp_CommandNotImplemented_Res(Xcp_PDUType const * const pdu);

XCP_STATIC void Xcp_Connect_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_Disconnect_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_GetStatus_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_Synch_Res(Xcp_PDUType const * const pdu);
#if XCP_ENABLE_GET_COMM_MODE_INFO == XCP_ON
XCP_STATIC void Xcp_GetCommModeInfo_Res(Xcp_PDUType const * const pdu);
#endif /* XCP_ENABLE_GET_COMM_MODE_INFO */
#if XCP_ENABLE_GET_ID == XCP_ON
XCP_STATIC void Xcp_GetId_Res(Xcp_PDUType const * const pdu);
#endif /* XCP_ENABLE_GET_ID */
#if XCP_ENABLE_SET_REQUEST == XCP_ON
XCP_STATIC void Xcp_SetRequest_Res(Xcp_PDUType const * const pdu);
#endif /* XCP_ENABLE_SET_REQUEST */
#if XCP_ENABLE_GET_SEED == XCP_ON
XCP_STATIC void Xcp_GetSeed_Res(Xcp_PDUType const * const pdu);
#endif /* XCP_ENABLE_GET_SEED */
#if XCP_ENABLE_UNLOCK == XCP_ON
XCP_STATIC void Xcp_Unlock_Res(Xcp_PDUType const * const pdu);
#endif /* XCP_ENABLE_UNLOCK */
#if XCP_ENABLE_SET_MTA == XCP_ON
XCP_STATIC void Xcp_SetMta_Res(Xcp_PDUType const * const pdu);
#endif /* XCP_ENABLE_SET_MTA */
#if XCP_ENABLE_UPLOAD == XCP_ON
XCP_STATIC void Xcp_Upload_Res(Xcp_PDUType const * const pdu);
#endif
#if XCP_ENABLE_SHORT_UPLOAD == XCP_ON
XCP_STATIC void Xcp_ShortUpload_Res(Xcp_PDUType const * const pdu);
#endif /* XCP_ENABLE_SHORT_UPLOAD */
#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON
XCP_STATIC void Xcp_BuildChecksum_Res(Xcp_PDUType const * const pdu);
#endif
#if XCP_ENABLE_TRANSPORT_LAYER_CMD == XCP_ON
XCP_STATIC void Xcp_TransportLayerCmd_Res(Xcp_PDUType const * const pdu);
#endif /* XCP_ENABLE_TRANSPORT_LAYER_CMD */
#if XCP_ENABLE_USER_CMD == XCP_ON
XCP_STATIC void Xcp_UserCmd_Res(Xcp_PDUType const * const pdu);
#endif /* XCP_ENABLE_USER_CMD */

#if XCP_ENABLE_CAL_COMMANDS == XCP_ON
XCP_STATIC void Xcp_Download_Res(Xcp_PDUType const * const pdu);
    #if XCP_ENABLE_DOWNLOAD_NEXT == XCP_ON
XCP_STATIC void Xcp_DownloadNext_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_DOWNLOAD_NEXT */
    #if XCP_ENABLE_DOWNLOAD_MAX == XCP_ON
XCP_STATIC void Xcp_DownloadMax_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_DOWNLOAD_MAX */
    #if XCP_ENABLE_SHORT_DOWNLOAD == XCP_ON
XCP_STATIC void Xcp_ShortDownload_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_SHORT_DOWNLOAD */
    #if XCP_ENABLE_MODIFY_BITS == XCP_ON
XCP_STATIC void Xcp_ModifyBits_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_MODIFY_BITS */

XCP_STATIC void Xcp_SetCalPage_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_GetCalPage_Res(Xcp_PDUType const * const pdu);
#endif /* XCP_ENABLE_CAL_COMMANDS */

#if XCP_ENABLE_PAG_COMMANDS == XCP_ON
    #if XCP_ENABLE_GET_PAG_PROCESSOR_INFO == XCP_ON
XCP_STATIC void Xcp_GetPagProcessorInfo_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_GET_PAG_PROCESSOR_INFO */
    #if XCP_ENABLE_GET_SEGMENT_INFO == XCP_ON
XCP_STATIC void Xcp_GetSegmentInfo_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_GET_SEGMENT_INFO */
    #if XCP_ENABLE_GET_PAGE_INFO == XCP_ON
XCP_STATIC void Xcp_GetPageInfo_Res(Xcp_PDUType const * const pdu);
    #endif
    #if XCP_ENABLE_SET_SEGMENT_MODE == XCP_ON
XCP_STATIC void Xcp_SetSegmentMode_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_SET_SEGMENT_MODE */
    #if XCP_ENABLE_GET_SEGMENT_MODE == XCP_ON
XCP_STATIC void Xcp_GetSegmentMode_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_GET_SEGMENT_MODE */
    #if XCP_ENABLE_COPY_CAL_PAGE
XCP_STATIC void Xcp_CopyCalPage_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_COPY_CAL_PAGE */
#endif     /* XCP_ENABLE_PAG_COMMANDS */

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
XCP_STATIC void Xcp_ClearDaqList_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_SetDaqPtr_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_WriteDaq_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_SetDaqListMode_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_GetDaqListMode_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_StartStopDaqList_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_StartStopSynch_Res(Xcp_PDUType const * const pdu);
    #if XCP_ENABLE_GET_DAQ_CLOCK == XCP_ON
XCP_STATIC void Xcp_GetDaqClock_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_GET_DAQ_CLOCK */
    #if XCP_ENABLE_READ_DAQ == XCP_ON
XCP_STATIC void Xcp_ReadDaq_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_READ_DAQ */
    #if XCP_ENABLE_GET_DAQ_PROCESSOR_INFO == XCP_ON
XCP_STATIC void Xcp_GetDaqProcessorInfo_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_GET_DAQ_PROCESSOR_INFO */
    #if XCP_ENABLE_GET_DAQ_RESOLUTION_INFO == XCP_ON
XCP_STATIC void Xcp_GetDaqResolutionInfo_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_GET_DAQ_RESOLUTION_INFO */
    #if XCP_ENABLE_GET_DAQ_LIST_INFO == XCP_ON
XCP_STATIC void Xcp_GetDaqListInfo_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_GET_DAQ_LIST_INFO */
    #if XCP_ENABLE_GET_DAQ_EVENT_INFO == XCP_ON
XCP_STATIC void Xcp_GetDaqEventInfo_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_GET_DAQ_EVENT_INFO */
    #if XCP_ENABLE_FREE_DAQ == XCP_ON
XCP_STATIC void Xcp_FreeDaq_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_FREE_DAQ */
    #if XCP_ENABLE_ALLOC_DAQ == XCP_ON
XCP_STATIC void Xcp_AllocDaq_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_ALLOC_DAQ */
    #if XCP_ENABLE_ALLOC_ODT == XCP_ON
XCP_STATIC void Xcp_AllocOdt_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_ALLOC_ODT */
    #if XCP_ENABLE_ALLOC_ODT_ENTRY == XCP_ON
XCP_STATIC void Xcp_AllocOdtEntry_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_ALLOC_ODT_ENTRY */
    #if XCP_ENABLE_WRITE_DAQ_MULTIPLE == XCP_ON
XCP_STATIC void Xcp_WriteDaqMultiple_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_WRITE_DAQ_MULTIPLE */
#endif     /* XCP_ENABLE_DAQ_COMMANDS */

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
XCP_STATIC void Xcp_ProgramStart_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_ProgramClear_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_Program_Res(Xcp_PDUType const * const pdu);
XCP_STATIC void Xcp_ProgramReset_Res(Xcp_PDUType const * const pdu);
    #if XCP_ENABLE_GET_PGM_PROCESSOR_INFO == XCP_ON
XCP_STATIC void Xcp_GetPgmProcessorInfo_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_GET_PGM_PROCESSOR_INFO */
    #if XCP_ENABLE_GET_SECTOR_INFO == XCP_ON
XCP_STATIC void Xcp_GetSectorInfo_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_GET_SECTOR_INFO */
    #if XCP_ENABLE_PROGRAM_PREPARE == XCP_ON
XCP_STATIC void Xcp_ProgramPrepare_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_PROGRAM_PREPARE */
    #if XCP_ENABLE_PROGRAM_FORMAT == XCP_ON
XCP_STATIC void Xcp_ProgramFormat_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_PROGRAM_FORMAT */
    #if XCP_ENABLE_PROGRAM_NEXT == XCP_ON
XCP_STATIC void Xcp_ProgramNext_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_PROGRAM_NEXT */
    #if XCP_ENABLE_PROGRAM_MAX == XCP_ON
XCP_STATIC void Xcp_ProgramMax_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_PROGRAM_MAX */
    #if XCP_ENABLE_PROGRAM_VERIFY == XCP_ON
XCP_STATIC void Xcp_ProgramVerify_Res(Xcp_PDUType const * const pdu);
    #endif /* XCP_ENABLE_PROGRAM_VERIFY */
#endif     /* XCP_ENABLE_PGM_COMMANDS */

/*
** Big, fat jump table.
*/

XCP_STATIC const Xcp_ServerCommandType Xcp_ServerCommands[] = {
/* lint -save -e632      Assignment to strong type 'Xcp_ServerCommandType'
 * considered harmless in this context. */
#if XCP_ENABLE_STD_COMMANDS == XCP_ON
    Xcp_Connect_Res,
    Xcp_Disconnect_Res,
    Xcp_GetStatus_Res,
    Xcp_Synch_Res,
    #if XCP_ENABLE_GET_COMM_MODE_INFO == XCP_ON
    Xcp_GetCommModeInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_ID == XCP_ON
    Xcp_GetId_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_SET_REQUEST == XCP_ON
    Xcp_SetRequest_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_SEED == XCP_ON
    Xcp_GetSeed_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_UNLOCK == XCP_ON
    Xcp_Unlock_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_SET_MTA == XCP_ON
    Xcp_SetMta_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_UPLOAD == XCP_ON
    Xcp_Upload_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_SHORT_UPLOAD == XCP_ON
    Xcp_ShortUpload_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON
    Xcp_BuildChecksum_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_TRANSPORT_LAYER_CMD == XCP_ON
    Xcp_TransportLayerCmd_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_USER_CMD == XCP_ON
    Xcp_UserCmd_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
#else
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
#endif

#if XCP_ENABLE_CAL_COMMANDS == XCP_ON
    Xcp_Download_Res,
    #if XCP_ENABLE_DOWNLOAD_NEXT == XCP_ON
    Xcp_DownloadNext_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_DOWNLOAD_MAX == XCP_ON
    Xcp_DownloadMax_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_SHORT_DOWNLOAD == XCP_ON
    Xcp_ShortDownload_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_MODIFY_BITS == XCP_ON
    Xcp_ModifyBits_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
#else
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
#endif

#if XCP_ENABLE_PAG_COMMANDS == XCP_ON
    Xcp_SetCalPage_Res,
    Xcp_GetCalPage_Res,
    #if XCP_ENABLE_GET_PAG_PROCESSOR_INFO == XCP_ON
    Xcp_GetPagProcessorInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_SEGMENT_INFO == XCP_ON
    Xcp_GetSegmentInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_PAGE_INFO == XCP_ON
    Xcp_GetPageInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_SET_SEGMENT_MODE == XCP_ON
    Xcp_SetSegmentMode_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_SEGMENT_MODE == XCP_ON
    Xcp_GetSegmentMode_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_COPY_CAL_PAGE == XCP_ON
    Xcp_CopyCalPage_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
#else
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
#endif

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    Xcp_ClearDaqList_Res,
    Xcp_SetDaqPtr_Res,
    Xcp_WriteDaq_Res,
    Xcp_SetDaqListMode_Res,
    Xcp_GetDaqListMode_Res,
    Xcp_StartStopDaqList_Res,
    Xcp_StartStopSynch_Res,
    #if XCP_ENABLE_GET_DAQ_CLOCK == XCP_ON
    Xcp_GetDaqClock_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_READ_DAQ == XCP_ON
    Xcp_ReadDaq_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_DAQ_PROCESSOR_INFO == XCP_ON
    Xcp_GetDaqProcessorInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_DAQ_RESOLUTION_INFO == XCP_ON
    Xcp_GetDaqResolutionInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_DAQ_LIST_INFO == XCP_ON
    Xcp_GetDaqListInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_DAQ_EVENT_INFO == XCP_ON
    Xcp_GetDaqEventInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_FREE_DAQ == XCP_ON
    Xcp_FreeDaq_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_ALLOC_DAQ == XCP_ON
    Xcp_AllocDaq_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_ALLOC_ODT == XCP_ON
    Xcp_AllocOdt_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_ALLOC_ODT_ENTRY == XCP_ON
    Xcp_AllocOdtEntry_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
#else
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
#endif

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    Xcp_ProgramStart_Res,
    Xcp_ProgramClear_Res,
    Xcp_Program_Res,
    Xcp_ProgramReset_Res,
    #if XCP_ENABLE_GET_PGM_PROCESSOR_INFO == XCP_ON
    Xcp_GetPgmProcessorInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_SECTOR_INFO == XCP_ON
    Xcp_GetSectorInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_PROGRAM_PREPARE == XCP_ON
    Xcp_ProgramPrepare_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_PROGRAM_FORMAT == XCP_ON
    Xcp_ProgramFormat_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_PROGRAM_NEXT == XCP_ON
    Xcp_ProgramNext_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_PROGRAM_MAX == XCP_ON
    Xcp_ProgramMax_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_PROGRAM_VERIFY == XCP_ON
    Xcp_ProgramVerify_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
#else
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
#endif
#if (XCP_ENABLE_DAQ_COMMANDS == XCP_ON) && (XCP_ENABLE_WRITE_DAQ_MULTIPLE == XCP_ON)
    Xcp_WriteDaqMultiple_Res,
#else
    Xcp_CommandNotImplemented_Res,
#endif
    /* lint -restore */
};

/*
**  Global Functions.
*/
void Xcp_Init(void) {
    Xcp_ConnectionState = XCP_DISCONNECTED;

    XcpHw_Init();
    XcpUtl_MemSet(&Xcp_State, UINT8(0), (uint32_t)sizeof(Xcp_StateType));
    Xcp_State.busy = (bool)XCP_FALSE;

    Xcp_DefaultResourceProtection();

#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
    Xcp_State.slaveBlockModeState.blockTransferActive = (bool)XCP_FALSE;
    Xcp_State.slaveBlockModeState.remaining           = UINT8(0);
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */
#if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
    Xcp_State.masterBlockModeState.blockTransferActive = (bool)XCP_FALSE;
    Xcp_State.masterBlockModeState.remaining           = UINT8(0);
#endif /* XCP_ENABLE_MASTER_BLOCKMODE */
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    XcpDaq_Init();
    Xcp_State.daqProcessor.state = XCP_DAQ_STATE_STOPPED;
    XcpDaq_SetPointer(0, 0, 0);
#endif /* XCP_ENABLE_DAQ_COMMANDS */
#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    Xcp_State.pgmProcessor.state = XCP_PGM_STATE_UNINIT;
#endif /* ENABLE_PGM_COMMANDS */
#if XCP_TRANSPORT_LAYER_COUNTER_SIZE != 0
    Xcp_State.counter = (uint16_t)0;
#endif /* XCP_TRANSPORT_LAYER_COUNTER_SIZE */

#if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_State.statistics.crosBusy     = UINT32(0);
    Xcp_State.statistics.crosSend     = UINT32(0);
    Xcp_State.statistics.ctosReceived = UINT32(0);
#endif /* XCP_ENABLE_STATISTICS */
    XcpTl_Init();

#if (XCP_ENABLE_BUILD_CHECKSUM == XCP_ON) && (XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON)
    Xcp_ChecksumInit();
#endif /* XCP_ENABLE_BUILD_CHECKSUM */
}

XCP_STATIC void Xcp_DefaultResourceProtection(void) {
#if XCP_ENABLE_RESOURCE_PROTECTION == XCP_ON
    Xcp_State.resourceProtection = UINT8(0);
    Xcp_State.seedRequested      = UINT8(0);
    #if (XCP_PROTECT_CAL == XCP_ON) || (XCP_PROTECT_PAG == XCP_ON)
    Xcp_State.resourceProtection |= XCP_RESOURCE_CAL_PAG;
    #endif /* XCP_PROTECT_CAL */
    #if XCP_PROTECT_DAQ == XCP_ON
    Xcp_State.resourceProtection |= XCP_RESOURCE_DAQ;
    #endif /* XCP_PROTECT_DAQ */
    #if XCP_PROTECT_STIM == XCP_ON
    Xcp_State.resourceProtection |= XCP_RESOURCE_STIM;
    #endif /* XCP_PROTECT_STIM */
    #if XCP_PROTECT_PGM == XCP_ON
    Xcp_State.resourceProtection |= XCP_RESOURCE_PGM;
    #endif /* XCP_PROTECT_PGM */
#endif     /* XCP_ENABLE_RESOURCE_PROTECTION */
}

void Xcp_Disconnect(void) {
    XcpTl_ReleaseConnection();
    Xcp_DefaultResourceProtection();
    XcpDaq_Init();
}

void Xcp_MainFunction(void) {
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    XcpDaq_MainFunction();
#endif /* XCP_ENABLE_DAQ_COMMANDS */

#if 0
    #if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
        Xcp_UploadSingleBlock();
    #endif /* XCP_ENABLE_SLAVE_BLOCKMODE */
#endif

#if (XCP_ENABLE_BUILD_CHECKSUM == XCP_ON) && (XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON)
    Xcp_ChecksumMainFunction();
#endif /* (XCP_ENABLE_BUILD_CHECKSUM) && (XCP_CHECKSUM_CHUNKED_CALCULATION ==                                                      \
          XCP_ON) */
}

void Xcp_SetMta(Xcp_MtaType mta) {
    Xcp_State.mta = mta;
}

Xcp_MtaType Xcp_GetNonPagedAddress(void const * const ptr) {
    Xcp_MtaType mta;

    mta.ext     = (uint8_t)0;
    mta.address = (uint32_t)ptr;
    return mta;
}

void Xcp_SendPdu(void) {
#if XCP_TRANSPORT_LAYER_LENGTH_SIZE != 0
    const uint16_t len = Xcp_PduOut.len;
#endif /* XCP_TRANSPORT_LAYER_LENGTH_SIZE */

#if XCP_TRANSPORT_LAYER_LENGTH_SIZE == 1
    Xcp_PduOut.data[0] = XCP_LOBYTE(len);
#elif XCP_TRANSPORT_LAYER_LENGTH_SIZE == 2
    Xcp_PduOut.data[0] = XCP_LOBYTE(len);
    Xcp_PduOut.data[1] = XCP_HIBYTE(len);
#endif /* XCP_TRANSPORT_LAYER_LENGTH_SIZE */

#if XCP_TRANSPORT_LAYER_COUNTER_SIZE == 1
    Xcp_PduOut.data[XCP_TRANSPORT_LAYER_LENGTH_SIZE] = XCP_LOBYTE(Xcp_State.counter);
    Xcp_State.counter++;
#elif XCP_TRANSPORT_LAYER_COUNTER_SIZE == 2
    Xcp_PduOut.data[XCP_TRANSPORT_LAYER_LENGTH_SIZE]     = XCP_LOBYTE(Xcp_State.counter);
    Xcp_PduOut.data[XCP_TRANSPORT_LAYER_LENGTH_SIZE + 1] = XCP_HIBYTE(Xcp_State.counter);
    Xcp_State.counter++;
#endif /* XCP_TRANSPORT_LAYER_COUNTER_SIZE */

#if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_State.statistics.crosSend++;
#endif /* XCP_ENABLE_STATISTICS */

    XcpTl_Send(Xcp_PduOut.data, Xcp_PduOut.len + (uint16_t)XCP_TRANSPORT_LAYER_BUFFER_OFFSET);
}

uint8_t *Xcp_GetOutPduPtr(void) {
    return &(Xcp_PduOut.data[XCP_TRANSPORT_LAYER_BUFFER_OFFSET]);
}

void Xcp_SetPduOutLen(uint16_t len) {
    Xcp_PduOut.len = len;
}

void Xcp_Send8(uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) {
    uint8_t *dataOut = Xcp_GetOutPduPtr();

#if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
    len = XCP_MAX_CTO;
#endif /* XCP_ON_CAN_MAX_DLC_REQUIRED */

    Xcp_SetPduOutLen((uint16_t)len);

    /* MISRA 2004 violation Rule 15.2               */
    /* Controlled fall-through (copy optimization)  */
    switch (len) {
        case 8:
            dataOut[7] = b7;
            /*lint -fallthrough */
        case 7:
            dataOut[6] = b6;
            /*lint -fallthrough */
        case 6:
            dataOut[5] = b5;
            /*lint -fallthrough */
        case 5:
            dataOut[4] = b4;
            /*lint -fallthrough */
        case 4:
            dataOut[3] = b3;
            /*lint -fallthrough */
        case 3:
            dataOut[2] = b2;
            /*lint -fallthrough */
        case 2:
            dataOut[1] = b1;
            /*lint -fallthrough */
        case 1:
            dataOut[0] = b0;
            break;
    }

    Xcp_SendPdu();
}

#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
XCP_STATIC bool Xcp_SlaveBlockTransferIsActive(void) {
    return Xcp_State.slaveBlockModeState.blockTransferActive;
}

XCP_STATIC void Xcp_SlaveBlockTransferSetActive(bool onOff) {
    XCP_ENTER_CRITICAL();
    Xcp_SetBusy(onOff); /* Active slave block-mode also means command processor
                           is busy. */
    Xcp_State.slaveBlockModeState.blockTransferActive = onOff;
    XCP_LEAVE_CRITICAL();
}

void Xcp_UploadSingleBlock(void) {
    uint8_t    *dataOut = XCP_NULL;
    uint8_t     length  = UINT8(0x00);
    Xcp_MtaType dst     = { 0 };

    if (!Xcp_SlaveBlockTransferIsActive()) {
        return;
    }
    dataOut     = Xcp_GetOutPduPtr();
    dataOut[0]  = (uint8_t)ERR_SUCCESS;
    dst.address = (uint32_t)(dataOut + 1);
    dst.ext     = (uint8_t)0;

    if (Xcp_State.slaveBlockModeState.remaining <= (XCP_MAX_CTO - 1)) {
        length = Xcp_State.slaveBlockModeState.remaining;

    #if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
        Xcp_SetPduOutLen(UINT16(XCP_MAX_CTO));
    #else
        Xcp_SetPduOutLen(length + UINT16(1));
    #endif /* XCP_ON_CAN_MAX_DLC_REQUIRED */

        // printf("PART BLOCK: %08x LEN% 02x\n", Xcp_State.mta.address, length);
        Xcp_CopyMemory(dst, Xcp_State.mta, (uint32_t)length);
        XCP_INCREMENT_MTA(length);
        Xcp_State.slaveBlockModeState.remaining -= length;
    } else {
        Xcp_SetPduOutLen(UINT16(XCP_MAX_CTO));
        // printf("FULL BLOCK: %08x\n", Xcp_State.mta.address);
        Xcp_CopyMemory(dst, Xcp_State.mta, (uint32_t)(XCP_MAX_CTO - 1));
        XCP_INCREMENT_MTA((XCP_MAX_CTO - 1));
        Xcp_State.slaveBlockModeState.remaining -= (XCP_MAX_CTO - 1);
    }

    Xcp_SendPdu();
    if (Xcp_State.slaveBlockModeState.remaining == UINT8(0)) {
        Xcp_SlaveBlockTransferSetActive((bool)XCP_FALSE);
        // printf("FINISHED.\n");
        // printf("----------------------------------------\n");
    }
}
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */

XCP_STATIC void Xcp_Upload(uint8_t len) {
    uint8_t *dataOut = Xcp_GetOutPduPtr();
#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_OFF
    Xcp_MtaType dst = { 0 };
    dataOut[0]      = (uint8_t)ERR_SUCCESS;
    dst.address     = (uint32_t)(dataOut + 1);
    dst.ext         = (uint8_t)0;

    Xcp_CopyMemory(dst, Xcp_State.mta, (uint32_t)len);
    XCP_INCREMENT_MTA(len);
    #if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
    Xcp_SetPduOutLen(UINT16(XCP_MAX_CTO));
    #else
    Xcp_SetPduOutLen(len + UINT16(1));
    #endif /* XCP_ON_CAN_MAX_DLC_REQUIRED */
    Xcp_SendPdu();
#else
    Xcp_State.slaveBlockModeState.remaining = len;

    // printf("----------------------------------------\n");
    Xcp_SlaveBlockTransferSetActive((bool)XCP_TRUE);
    Xcp_UploadSingleBlock();
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */
}

/**
 * Entry point, needs to be "wired" to CAN-Rx interrupt.
 *
 * @param pdu
 */

void Xcp_DispatchCommand(Xcp_PDUType const * const pdu) {
    const uint8_t cmd = pdu->data[0];
    DBG_TRACE1("<- ");

    if (Xcp_State.connected == (bool)XCP_TRUE) {
        /*DBG_PRINT2("CMD: [%02X]\n", cmd); */

        if (Xcp_IsBusy()) {
            Xcp_BusyResponse();
            return;
        } else {
#if XCP_ENABLE_STATISTICS == XCP_ON
            Xcp_State.statistics.ctosReceived++;
#endif /* XCP_ENABLE_STATISTICS */
            Xcp_ServerCommands[UINT8(0xff) - cmd](pdu);
        }
    } else { /* not connected. */
#if XCP_ENABLE_STATISTICS == XCP_ON
        Xcp_State.statistics.ctosReceived++;
#endif /* XCP_ENABLE_STATISTICS */
        if (pdu->data[0] == UINT8(XCP_CONNECT)) {
            Xcp_Connect_Res(pdu);
        } else {
        }
    }
#if defined(_MSC_VER)
    fflush(stdout);
#endif
}

void Xcp_SetSendCallout(Xcp_SendCalloutType callout) {
    Xcp_SendCallout = callout;
}

/*
**
** Global Helper Functions.
**
** Note: These functions are only useful for unit-testing and debugging.
**
*/
Xcp_ConnectionStateType Xcp_GetConnectionState(void) {
    return Xcp_ConnectionState;
}

/*
**
** Local Functions.
**
*/
XCP_STATIC void Xcp_CommandNotImplemented_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE2("Command not implemented [%02X].\n", pdu->data[0]);
    Xcp_ErrorResponse(UINT8(ERR_CMD_UNKNOWN));
}

XCP_STATIC void Xcp_Connect_Res(Xcp_PDUType const * const pdu) {
    uint8_t resource      = UINT8(0x00);
    uint8_t commModeBasic = UINT8(0x00);

    DBG_TRACE1("CONNECT\n");

    if (Xcp_State.connected == (bool)XCP_FALSE) {
        Xcp_State.connected = (bool)XCP_TRUE;
        /* TODO: Init stuff */
    }

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    resource |= XCP_RESOURCE_PGM;
#endif /* XCP_ENABLE_PGM_COMMANDS */
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    resource |= XCP_RESOURCE_DAQ;
#endif /* XCP_ENABLE_DAQ_COMMANDS */
#if (XCP_ENABLE_CAL_COMMANDS == XCP_ON) || (XCP_ENABLE_PAG_COMMANDS == XCP_ON)
    resource |= XCP_RESOURCE_CAL_PAG;
#endif
#if XCP_ENABLE_STIM == XCP_ON
    resource |= XCP_RESOURCE_STIM;
#endif /* XCP_ENABLE_STIM */

    commModeBasic |= XCP_BYTE_ORDER;
    commModeBasic |= XCP_ADDRESS_GRANULARITY;
#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
    commModeBasic |= XCP_SLAVE_BLOCK_MODE;
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */
#if XCP_ENABLE_GET_COMM_MODE_INFO
    commModeBasic |= XCP_OPTIONAL_COMM_MODE;
#endif /* XCP_ENABLE_GET_COMM_MODE_INFO */

    XcpTl_SaveConnection();

    Xcp_Send8(
        UINT8(8), UINT8(0xff), UINT8(resource), UINT8(commModeBasic), UINT8(XCP_MAX_CTO), XCP_LOBYTE(XCP_MAX_DTO),
        XCP_HIBYTE(XCP_MAX_DTO), UINT8(XCP_PROTOCOL_VERSION_MAJOR), UINT8(XCP_TRANSPORT_LAYER_VERSION_MAJOR)
    );
    /*DBG_PRINT("MAX-DTO: %04X H: %02X L: %02X\n", XCP_MAX_DTO,
     * HIBYTE(XCP_MAX_DTO), LOBYTE(XCP_MAX_DTO)); */
}

XCP_STATIC void Xcp_Disconnect_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("DISCONNECT\n\n");

    XCP_ASSERT_PGM_IDLE();
    Xcp_PositiveResponse();
    Xcp_Disconnect();
}

XCP_STATIC void Xcp_GetStatus_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("GET_STATUS\n");

    Xcp_Send8(
        UINT8(6), UINT8(0xff), UINT8(0), /* Current session status */
#if XCP_ENABLE_RESOURCE_PROTECTION == XCP_ON
        Xcp_State.resourceProtection, /* Current resource protection status */
#else
        UINT8(0x00), /* Everything is unprotected. */
#endif               /* XCP_ENABLE_RESOURCE_PROTECTION */
        UINT8(0x00), /* Reserved */
        UINT8(0),    /* Session configuration id */
        UINT8(0),    /* "                      " */
        UINT8(0), UINT8(0)
    );
}

XCP_STATIC void Xcp_Synch_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("SYNCH\n");
    Xcp_ErrorResponse(UINT8(ERR_CMD_SYNCH));
}

#if XCP_ENABLE_GET_COMM_MODE_INFO == XCP_ON
XCP_STATIC void Xcp_GetCommModeInfo_Res(Xcp_PDUType const * const pdu) {
    uint8_t commModeOptional = UINT8(0);

    DBG_TRACE1("GET_COMM_MODE_INFO\n");

    #if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
    commModeOptional |= XCP_MASTER_BLOCK_MODE;
    #endif /* XCP_ENABLE_MASTER_BLOCKMODE */

    #if XCP_ENABLE_INTERLEAVED_MODE == XCP_ON
    commModeOptional |= XCP_INTERLEAVED_MODE;
    #endif /* XCP_ENABLE_INTERLEAVED_MODE */

    Xcp_Send8(
        UINT8(8), UINT8(0xff), UINT8(0), /* Reserved */
        commModeOptional, UINT8(0),      /* Reserved */
        UINT8(XCP_MAX_BS), UINT8(XCP_MIN_ST), UINT8(XCP_QUEUE_SIZE), UINT8(XCP_DRIVER_VERSION)
    );
}
#endif /* XCP_ENABLE_GET_COMM_MODE_INFO */

#if XCP_ENABLE_GET_ID == XCP_ON
XCP_STATIC void Xcp_GetId_Res(Xcp_PDUType const * const pdu) {
    uint8_t      idType       = Xcp_GetByte(pdu, UINT8(1));
    static char *response     = XCP_NULL;
    uint32_t     response_len = UINT32(0);

    DBG_TRACE2("GET_ID [type: 0x%02x]\n", idType);

    if (idType == UINT8(0)) {
        response     = Xcp_GetId0.name;
        response_len = Xcp_GetId0.len;
    } else if (idType == UINT8(1)) {
        response     = Xcp_GetId1.name;
        response_len = Xcp_GetId1.len;
    }
    #if XCP_ENABLE_GET_ID_HOOK == XCP_ON
    else {
        if (!Xcp_HookFunction_GetId(idType, &response, &response_len)) {
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
        }
    }
    #else
    else {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }
    #endif /* XCP_ENABLE_GET_ID_HOOK */

    Xcp_SetMta(Xcp_GetNonPagedAddress(response));
    Xcp_Send8(
        UINT8(8), UINT8(0xff), UINT8(0), UINT8(0), UINT8(0), XCP_LOBYTE(XCP_LOWORD(response_len)),
        XCP_HIBYTE(XCP_LOWORD(response_len)), XCP_LOBYTE(XCP_HIWORD(response_len)), XCP_HIBYTE(XCP_HIWORD(response_len))
    );
}
#endif /* XCP_ENABLE_GET_ID */

#if XCP_ENABLE_GET_SEED == XCP_ON
XCP_STATIC void Xcp_GetSeed_Res(Xcp_PDUType const * const pdu) {
    uint8_t         mode     = Xcp_GetByte(pdu, UINT8(1));
    uint8_t         resource = Xcp_GetByte(pdu, UINT8(2));
    Xcp_1DArrayType seed     = { 0 };
    uint8_t         length   = UINT8(0);
    uint8_t        *dataOut  = Xcp_GetOutPduPtr();

    DBG_TRACE3("GET_SEED [mode: %02x resource: %02x]\n", mode, resource);

    XCP_ASSERT_PGM_IDLE();
    switch (resource) {
        case XCP_RESOURCE_PGM:
    #if XCP_ENABLE_PGM_COMMANDS == XCP_OFF
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
    #else
            break;
    #endif /* XCP_ENABLE_PGM_COMMANDS */
        case XCP_RESOURCE_STIM:
    #if XCP_ENABLE_STIM == XCP_OFF
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
    #else
            break;
    #endif /* XCP_ENABLE_STIM */
        case XCP_RESOURCE_DAQ:
    #if XCP_ENABLE_DAQ_COMMANDS == XCP_OFF
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
    #else
            break;
    #endif /* XCP_ENABLE_DAQ_COMMANDS */
        case XCP_RESOURCE_CAL_PAG:
    #if (XCP_ENABLE_CAL_COMMANDS == XCP_OFF) && (XCP_ENABLE_PAG_COMMANDS == XCP_OFF)
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
    #else
            break;
    #endif /* XCP_ENABLE_CAL_COMMANDS */
        default:
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
    }

    if (Xcp_IsProtected(resource)) {
        Xcp_HookFunction_GetSeed(resource, &seed); /* User supplied callout. */
        length = XCP_MIN(XCP_MAX_CTO - UINT16(2), seed.length);
        XcpUtl_MemCopy(dataOut + UINT16(2), seed.data, (uint32_t)length);
    } else {
        /* Resource already unlocked. */
        length = UINT8(0);
    }
    Xcp_State.seedRequested |= resource;
    dataOut[0] = UINT8(ERR_SUCCESS);
    dataOut[1] = length;
    #if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
    Xcp_SetPduOutLen(UINT16(XCP_MAX_CTO));
    #else
    Xcp_SetPduOutLen(length + UINT16(2));
    #endif /* XCP_ON_CAN_MAX_DLC_REQUIRED */
    Xcp_SendPdu();
}
#endif /* XCP_ENABLE_GET_SEED */

#if XCP_ENABLE_UNLOCK == XCP_ON
XCP_STATIC void Xcp_Unlock_Res(Xcp_PDUType const * const pdu) {
    uint8_t         length = Xcp_GetByte(pdu, UINT8(1));
    Xcp_1DArrayType key    = { 0 };

    DBG_TRACE2("UNLOCK [length: %u]\n", length);

    XCP_ASSERT_PGM_IDLE();
    if (Xcp_State.seedRequested == UINT8(0)) {
        Xcp_ErrorResponse(UINT8(ERR_SEQUENCE));
        return;
    }

    key.length = length;
    key.data   = pdu->data + 2;

    if (Xcp_HookFunction_Unlock(Xcp_State.seedRequested, &key)) {          /* User supplied callout. */
        Xcp_State.resourceProtection &= UINT8(~(Xcp_State.seedRequested)); /* OK, unlock. */
        Xcp_Send8(
            UINT8(2), UINT8(0xff), Xcp_State.resourceProtection, /* Current resource protection status. */
            UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0)
        );

    } else {
        Xcp_ErrorResponse(UINT8(ERR_ACCESS_LOCKED));
        Xcp_Disconnect();
    }
    Xcp_State.seedRequested = UINT8(0x00);
}
#endif /* XCP_ENABLE_UNLOCK */

#if XCP_ENABLE_UPLOAD == XCP_ON
XCP_STATIC void Xcp_Upload_Res(Xcp_PDUType const * const pdu) {
    uint8_t len = Xcp_GetByte(pdu, UINT8(1));

    DBG_TRACE2("UPLOAD [len: %u]\n", len);

    XCP_ASSERT_PGM_IDLE();
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, len, XCP_MEM_ACCESS_READ, XCP_FALSE);
    #if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_OFF
    if (len > UINT8(XCP_MAX_CTO - 1)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }
    #else

    #endif /* XCP_ENABLE_SLAVE_BLOCKMODE */

    Xcp_Upload(len);
}
#endif /* XCP_ENABLE_UPLOAD */

#if XCP_ENABLE_SHORT_UPLOAD == XCP_ON
XCP_STATIC void Xcp_ShortUpload_Res(Xcp_PDUType const * const pdu) {
    uint8_t len = Xcp_GetByte(pdu, UINT8(1));

    DBG_TRACE2("SHORT-UPLOAD [len: %u]\n", len);

    XCP_ASSERT_PGM_IDLE();
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, len, XCP_MEM_ACCESS_READ, (bool)XCP_FALSE);
    if (len > UINT8(XCP_MAX_CTO - 1)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }

    Xcp_State.mta.ext     = Xcp_GetByte(pdu, UINT8(3));
    Xcp_State.mta.address = Xcp_GetDWord(pdu, UINT8(4));
    Xcp_Upload(len);
}
#endif /* XCP_ENABLE_SHORT_UPLOAD */

#if XCP_ENABLE_SET_MTA == XCP_ON
XCP_STATIC void Xcp_SetMta_Res(Xcp_PDUType const * const pdu) {
    Xcp_State.mta.ext     = Xcp_GetByte(pdu, UINT8(3));
    Xcp_State.mta.address = Xcp_GetDWord(pdu, UINT8(4));

    DBG_TRACE3("SET_MTA [address: 0x%08x ext: 0x%02x]\n", Xcp_State.mta.address, Xcp_State.mta.ext);

    Xcp_PositiveResponse();
}
#endif /* XCP_ENABLE_SET_MTA */

#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON
void Xcp_SendChecksumPositiveResponse(Xcp_ChecksumType checksum) {
    Xcp_Send8(
        UINT8(8), UINT8(0xff), UINT8(XCP_CHECKSUM_METHOD), UINT8(0), UINT8(0), XCP_LOBYTE(XCP_LOWORD(checksum)),
        XCP_HIBYTE(XCP_LOWORD(checksum)), XCP_LOBYTE(XCP_HIWORD(checksum)), XCP_HIBYTE(XCP_HIWORD(checksum))
    );
}

void Xcp_SendChecksumOutOfRangeResponse(void) {
    Xcp_Send8(
        UINT8(8), UINT8(0xfe), UINT8(ERR_OUT_OF_RANGE), UINT8(0), UINT8(0), XCP_LOBYTE(XCP_LOWORD(XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE)),
        XCP_HIBYTE(XCP_LOWORD(XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE)), XCP_LOBYTE(XCP_HIWORD(XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE)),
        XCP_HIBYTE(XCP_HIWORD(XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE))
    );
}

XCP_STATIC void Xcp_BuildChecksum_Res(Xcp_PDUType const * const pdu) {
    uint32_t         blockSize = Xcp_GetDWord(pdu, UINT8(4));
    Xcp_ChecksumType checksum  = (Xcp_ChecksumType)0;
    uint8_t const   *ptr       = XCP_NULL;

    DBG_TRACE2("BUILD_CHECKSUM [blocksize: %u]\n", blockSize);

    XCP_ASSERT_PGM_IDLE();
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, blockSize, XCP_MEM_ACCESS_READ, (bool)XCP_FALSE);
    #if XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE > 0
    /* We need to range check. */
    if (blockSize > UINT32(XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE)) {
        Xcp_SendChecksumOutOfRangeResponse();
        return;
    }
    #endif

    ptr = (uint8_t const *)Xcp_State.mta.address;
    /* The MTA will be post-incremented by the block size. */

    #if XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_OFF
    checksum = Xcp_CalculateChecksum(ptr, blockSize, (Xcp_ChecksumType)0, XCP_TRUE);
    Xcp_SendChecksumPositiveResponse(checksum);
    #else
    if (blockSize <= UINT32(XCP_CHECKSUM_CHUNK_SIZE)) {
        checksum = Xcp_CalculateChecksum(ptr, blockSize, (Xcp_ChecksumType)0, (bool)XCP_TRUE);
        Xcp_SendChecksumPositiveResponse(checksum);
    } else {
        Xcp_StartChecksumCalculation(ptr, blockSize);
    }
    #endif /* XCP_CHECKSUM_CHUNKED_CALCULATION */
}
#endif /* XCP_ENABLE_BUILD_CHECKSUM */

#if 0
    XCP_VALIDATE_ADRESS
        Callout der die gültigkeit eines Speicherzugriffs überprüft [addr;length]
#endif

#if XCP_ENABLE_TRANSPORT_LAYER_CMD
XCP_STATIC void Xcp_TransportLayerCmd_Res(Xcp_PDUType const * const pdu) {
    XCP_ASSERT_PGM_IDLE();
    XcpTl_TransportLayerCmd_Res(pdu);
}
#endif /* XCP_ENABLE_TRANSPORT_LAYER_CMD */

#if XCP_ENABLE_USER_CMD
XCP_STATIC void Xcp_UserCmd_Res(Xcp_PDUType const * const pdu) {
    XCP_ASSERT_PGM_IDLE();
}
#endif /* XCP_ENABLE_USER_CMD */

/*
**
**  CAL Commands.
**
*/
#if XCP_ENABLE_CAL_COMMANDS == XCP_ON
XCP_STATIC void Xcp_Download_Res(Xcp_PDUType const * const pdu) {
    uint8_t     len = Xcp_GetByte(pdu, UINT8(1));
    Xcp_MtaType src = { 0 };

    DBG_TRACE2("DOWNLOAD [len: %u]\n", len);

    XCP_ASSERT_PGM_IDLE();
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, len, XCP_MEM_ACCESS_WRITE, XCP_FALSE);

    #if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
    Xcp_State.masterBlockModeState.blockTransferActive = (bool)XCP_FALSE;
    if (len > (XCP_MAX_BS * XCP_DOWNLOAD_PAYLOAD_LENGTH)) {
        Xcp_ErrorResponse(ERR_OUT_OF_RANGE); /* Request exceeds max. block size. */
        return;
    } else if (len > XCP_DOWNLOAD_PAYLOAD_LENGTH) {
        /* OK, regular first-frame transfer. */
        Xcp_State.masterBlockModeState.blockTransferActive = (bool)XCP_TRUE;
        Xcp_State.masterBlockModeState.remaining           = len - XCP_DOWNLOAD_PAYLOAD_LENGTH;
        Xcp_Download_Copy(UINT32(pdu->data + 2), UINT8(0), UINT32(XCP_DOWNLOAD_PAYLOAD_LENGTH));
        return;
    }
    #endif /* XCP_ENABLE_MASTER_BLOCKMODE */

    #if XCP_ENABLE_MASTER_BLOCKMODE == XCP_OFF
    if (len > XCP_DOWNLOAD_PAYLOAD_LENGTH) {
        Xcp_ErrorResponse(ERR_OUT_OF_RANGE); /* Request exceeds max. payload size. */
        return;
    }
    #endif
    Xcp_Download_Copy(UINT32(pdu->data + 2), UINT8(0), UINT32(len));
    Xcp_PositiveResponse();
}

    #if XCP_ENABLE_DOWNLOAD_NEXT == XCP_ON
        #if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
XCP_STATIC void Xcp_DownloadNext_Res(Xcp_PDUType const * const pdu) {
    uint8_t     remaining = Xcp_GetByte(pdu, UINT8(1));
    uint8_t     len;
    Xcp_MtaType src = { 0 };

    DBG_TRACE2("DOWNLOAD_NEXT [remaining: %u]\n", remaining);

    XCP_ASSERT_PGM_IDLE();
    if (!Xcp_State.masterBlockModeState.blockTransferActive) {
        Xcp_ErrorResponse(ERR_SEQUENCE); /* Check: Is it really necessary to start a
                                            block-mode transfer with Xcp_Download_Res? */
        return;
    }
    len = XCP_MIN(remaining, XCP_DOWNLOAD_PAYLOAD_LENGTH);
    Xcp_Download_Copy(UINT32(pdu->data + 2), UINT8(0), UINT32(len));
    Xcp_State.masterBlockModeState.remaining -= len;
    if (Xcp_State.masterBlockModeState.remaining == UINT8(0)) {
        Xcp_State.masterBlockModeState.blockTransferActive = (bool)XCP_FALSE;
        Xcp_PositiveResponse();
    }
}
        #endif /* XCP_ENABLE_MASTER_BLOCKMODE */
    #endif     /* XCP_ENABLE_DOWNLOAD_NEXT */

    #if XCP_ENABLE_DOWNLOAD_MAX == XCP_ON
XCP_STATIC void Xcp_DownloadMax_Res(Xcp_PDUType const * const pdu) {
    Xcp_MtaType src = { 0 };

    DBG_TRACE1("DOWNLOAD_MAX\n");

    XCP_ASSERT_PGM_IDLE();
    Xcp_Download_Copy(UINT32(pdu->data + 1), UINT8(0), UINT32(XCP_DOWNLOAD_PAYLOAD_LENGTH + 1));
    Xcp_PositiveResponse();
}
    #endif /* XCP_ENABLE_DOWNLOAD_MAX */

    #if XCP_ENABLE_SHORT_DOWNLOAD == XCP_ON
XCP_STATIC void Xcp_ShortDownload_Res(Xcp_PDUType const * const pdu) {
    uint8_t     len     = Xcp_GetByte(pdu, UINT8(1));
    uint8_t     addrExt = Xcp_GetByte(pdu, UINT8(3));
    uint32_t    address = Xcp_GetDWord(pdu, UINT8(4));
    Xcp_MtaType src     = { 0 };
    Xcp_MtaType dst     = { 0 };

    DBG_TRACE4("SHORT-DOWNLOAD [len: %u address: 0x%08x ext: 0x%02x]\n", len, address, addrExt);
    dst.address = address;
    dst.ext     = addrExt;

    XCP_ASSERT_PGM_IDLE();
    XCP_CHECK_MEMORY_ACCESS(dst, len, XCP_MEM_ACCESS_WRITE, (bool)XCP_FALSE);
    if (len > (XCP_MAX_CTO - UINT8(8))) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }

    src.address = (uint32_t)pdu->data + 8;
    src.ext     = UINT8(0);
    Xcp_CopyMemory(dst, src, (uint32_t)len);

    XCP_INCREMENT_MTA(len);

    Xcp_PositiveResponse();
}
    #endif /* XCP_ENABLE_SHORT_DOWNLOAD */

    #if XCP_ENABLE_MODIFY_BITS == XCP_ON
XCP_STATIC void Xcp_ModifyBits_Res(Xcp_PDUType const * const pdu) {
    uint8_t   shiftValue = Xcp_GetByte(pdu, UINT8(1));
    uint16_t  andMask    = Xcp_GetWord(pdu, UINT8(2));
    uint16_t  xorMask    = Xcp_GetWord(pdu, UINT8(4));
    uint32_t *vp         = XCP_NULL;

    DBG_TRACE4(
        "MODIFY-BITS [shiftValue: 0x%02X andMask: 0x%04x ext: xorMask: "
        "0x%04x]\n",
        shiftValue, andMask, xorMask
    );
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, 2, XCP_MEM_ACCESS_WRITE, (bool)XCP_FALSE);
    vp  = (uint32_t *)Xcp_State.mta.address;
    *vp = ((*vp) & (((~((uint32_t)(((uint16_t)~andMask) << shiftValue)))) ^ ((uint32_t)(xorMask << shiftValue))));

    Xcp_PositiveResponse();
}
    #endif /* XCP_ENABLE_MODIFY_BITS */

#endif /* XCP_ENABLE_CAL_COMMANDS */

/*
**
**  DAQ Commands.
**
*/
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
XCP_STATIC void Xcp_ClearDaqList_Res(Xcp_PDUType const * const pdu) {
    XcpDaq_ListIntegerType daqListNumber = (XcpDaq_ListIntegerType)0;

    daqListNumber = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));

    DBG_TRACE2("CLEAR_DAQ_LIST [daq: %u] \n", daqListNumber);

    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    Xcp_PositiveResponse();
}

XCP_STATIC void Xcp_SetDaqPtr_Res(Xcp_PDUType const * const pdu) {
    XcpDaq_ListIntegerType     daqList  = (XcpDaq_ListIntegerType)0;
    XcpDaq_ODTIntegerType      odt      = (XcpDaq_ODTIntegerType)0;
    XcpDaq_ODTEntryIntegerType odtEntry = (XcpDaq_ODTEntryIntegerType)0;

    XCP_ASSERT_DAQ_STOPPED();
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    daqList  = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    odt      = Xcp_GetByte(pdu, UINT8(4));
    odtEntry = Xcp_GetByte(pdu, UINT8(5));

    /* TODO: Check for Predef. lists only */

    if (!XcpDaq_ValidateOdtEntry(daqList, odt, odtEntry)) {
        /* If the specified list is not available, ERR_OUT_OF_RANGE will be
         * returned. */
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }

    XcpDaq_SetPointer(daqList, odt, odtEntry);

    DBG_TRACE4(
        "SET_DAQ_PTR [daq: %u odt: %u odtEntry: %u]\n", Xcp_State.daqPointer.daqList, Xcp_State.daqPointer.odt,
        Xcp_State.daqPointer.odtEntry
    );

    Xcp_PositiveResponse();
}

XCP_STATIC void Xcp_WriteDaq_Res(Xcp_PDUType const * const pdu) {
    XcpDaq_ODTEntryType *entry     = XCP_NULL;
    const uint8_t        bitOffset = Xcp_GetByte(pdu, UINT8(1));
    const uint8_t        elemSize  = Xcp_GetByte(pdu, UINT8(2));
    const uint8_t        adddrExt  = Xcp_GetByte(pdu, UINT8(3));
    const uint32_t       address   = Xcp_GetDWord(pdu, UINT8(4));

    DBG_TRACE5("WRITE_DAQ [address: 0x%08x ext: 0x%02x size: %u offset: %u]\n", address, adddrExt, elemSize, bitOffset);

    XCP_ASSERT_DAQ_STOPPED();
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    /* WRITE_DAQ is only possible for elements in configurable DAQ lists. */
    if (Xcp_State.daqPointer.daqList < XcpDaq_PredefinedListCount) {
        Xcp_SendResult(ERR_WRITE_PROTECTED);
        return;
    }

    #endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */

    entry = XcpDaq_GetOdtEntry(Xcp_State.daqPointer.daqList, Xcp_State.daqPointer.odt, Xcp_State.daqPointer.odtEntry);

    #if XCP_DAQ_BIT_OFFSET_SUPPORTED == XCP_ON
    entry->bitOffset = bitOffset;
    #endif /* XCP_DAQ_ENABLE_BIT_OFFSET */
    entry->length      = elemSize;
    entry->mta.address = address;
    #if XCP_DAQ_ADDR_EXT_SUPPORTED == XCP_ON
    entry->mta.ext = adddrExt;
    #endif /* XCP_DAQ_ENABLE_ADDR_EXT */

    /* Advance ODT entry pointer within  one  and  the same ODT. After writing
    to
    the last ODT entry of an ODT, the value of the DAQ pointer is undefined! */
    Xcp_State.daqPointer.odtEntry += (XcpDaq_ODTEntryIntegerType)1;

    Xcp_PositiveResponse();
}

XCP_STATIC void Xcp_SetDaqListMode_Res(Xcp_PDUType const * const pdu) {
    XcpDaq_ListStateType        *entry              = XCP_NULL;
    const uint8_t                mode               = Xcp_GetByte(pdu, UINT8(1));
    const XcpDaq_ListIntegerType daqListNumber      = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    const uint16_t               eventChannelNumber = Xcp_GetWord(pdu, UINT8(4));
    const uint8_t                prescaler          = Xcp_GetByte(pdu, UINT8(6));
    const uint8_t                priority           = Xcp_GetByte(pdu, UINT8(7));

    DBG_TRACE6(
        "SET_DAQ_LIST_MODE [mode: 0x%02x daq: %03u event: %03u prescaler: %03u "
        "priority: %03u]\n",
        mode, daqListNumber, eventChannelNumber, prescaler, priority
    );
    XCP_ASSERT_DAQ_STOPPED();
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    #if XCP_ENABLE_STIM == XCP_OFF
    if ((mode & XCP_DAQ_LIST_MODE_DIRECTION) == XCP_DAQ_LIST_MODE_DIRECTION) {
        Xcp_ErrorResponse(UINT8(ERR_CMD_SYNTAX));
        return;
    }
    #endif /* XCP_ENABLE_STIM */
    /*
    The master is not allowed to set the ALTERNATING flag and the TIMESTAMP flag at
    the same time.
    */
    #if XCP_DAQ_ALTERNATING_SUPPORTED == XCP_OFF
    if ((mode & XCP_DAQ_LIST_MODE_ALTERNATING) == XCP_DAQ_LIST_MODE_ALTERNATING) {
        Xcp_ErrorResponse(UINT8(ERR_CMD_SYNTAX));
        return;
    }
    #endif /* XCP_DAQ_ENABLE_ALTERNATING */
    #if XCP_DAQ_PRIORITIZATION_SUPPORTED == XCP_OFF
    /* Needs to be 0 */
    if (priority > UINT8(0)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }
    #endif /* XCP_DAQ_ENABLE_PRIORITIZATION */
    #if XCP_DAQ_PRESCALER_SUPPORTED == XCP_OFF
    /* Needs to be 1 */
    if (prescaler > UINT8(1)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }
    #endif /* XCP_DAQ_ENABLE_PRESCALER */

    entry = XcpDaq_GetListState(daqListNumber);
    XcpDaq_AddEventChannel(daqListNumber, eventChannelNumber);

    entry->mode = Xcp_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_TIMESTAMP);
    entry->mode = Xcp_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_ALTERNATING);
    entry->mode = Xcp_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_DIRECTION);
    entry->mode = Xcp_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_PID_OFF);
    entry->mode = Xcp_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_SELECTED);
    entry->mode = Xcp_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_STARTED);
    #if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
    entry->prescaler = prescaler;
    #endif /* XCP_DAQ_ENABLE_PRESCALER */

    Xcp_PositiveResponse();
}

XCP_STATIC void Xcp_StartStopDaqList_Res(Xcp_PDUType const * const pdu) {
    XcpDaq_ListStateType        *entry         = XCP_NULL;
    const uint8_t                mode          = Xcp_GetByte(pdu, UINT8(1));
    XcpDaq_ODTIntegerType        firstPid      = (XcpDaq_ODTIntegerType)0;
    const XcpDaq_ListIntegerType daqListNumber = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    #if 0
1  BYTE  Mode
    00 = stop
    01 = start
    02 = select
2  WORD  DAQ_LIST_NUMBER [0,1,..MAX_DAQ-1]
    #endif /* 0 */

    DBG_TRACE3("START_STOP_DAQ_LIST [mode: 0x%02x daq: %03u]\n", mode, daqListNumber);
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    if (daqListNumber >= XcpDaq_GetListCount()) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
    }

    entry = XcpDaq_GetListState(daqListNumber);

    if (mode == UINT8(0)) {
    } else if (mode == UINT8(1)) {
    } else if (mode == UINT8(2)) {
        entry->mode |= XCP_DAQ_LIST_MODE_SELECTED;
    } else {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE)); /* correct? */
        return;
    }

    XcpDaq_GetFirstPid(daqListNumber, &firstPid);

    Xcp_Send8(UINT8(8), UINT8(0xff), firstPid, UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0));
}

XCP_STATIC void Xcp_StartStopSynch_Res(Xcp_PDUType const * const pdu) {
    const uint8_t mode = Xcp_GetByte(pdu, UINT8(1));

    DBG_TRACE2("START_STOP_SYNCH [mode: 0x%02x]\n", mode);
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    if (mode == START_SELECTED) {
        XcpDaq_StartSelectedLists();
        XcpDaq_SetProcessorState(XCP_DAQ_STATE_RUNNING);
    } else if (mode == STOP_ALL) {
        XcpDaq_StopAllLists();
        XcpDaq_SetProcessorState(XCP_DAQ_STATE_STOPPED);
    } else if (mode == STOP_SELECTED) {
        XcpDaq_StopSelectedLists();
    } else {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }
    Xcp_PositiveResponse();
}

XCP_STATIC void Xcp_GetDaqListMode_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("GET_DAQ_LIST_MODE\n");
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    Xcp_PositiveResponse();
}

    #if XCP_ENABLE_GET_DAQ_PROCESSOR_INFO == XCP_ON
XCP_STATIC void Xcp_GetDaqProcessorInfo_Res(Xcp_PDUType const * const pdu) {
    uint8_t                properties = UINT8(0);
    XcpDaq_ListIntegerType listCount  = (XcpDaq_ListIntegerType)0;

    DBG_TRACE1("GET_DAQ_PROCESSOR_INFO\n");
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    XcpDaq_GetProperties(&properties);
    listCount = XcpDaq_GetListCount();

        #if 0
    0  BYTE  Packet ID: 0xFF
    1  BYTE  DAQ_PROPERTIES     General properties of DAQ lists
    2  WORD  MAX_DAQ            Total number of available DAQ lists
    4  WORD  MAX_EVENT_CHANNEL  Total number of available event channels

    6  BYTE  MIN_DAQ            Total number of predefined DAQ lists
    7  BYTE  DAQ_KEY_BYTE
        #endif /* 0 */
    Xcp_Send8(
        UINT8(8), UINT8(0xff), properties, XCP_LOBYTE(listCount), XCP_HIBYTE(listCount),
        XCP_LOBYTE(UINT16(XCP_DAQ_MAX_EVENT_CHANNEL)), XCP_HIBYTE(UINT16(XCP_DAQ_MAX_EVENT_CHANNEL)),
        #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
        XcpDaq_PredefinedListCount,
        #else
        UINT8(0),
        #endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */
        UINT8(0)
    );
}
    #endif /* XCP_ENABLE_GET_DAQ_PROCESSOR_INFO */

    #if XCP_ENABLE_GET_DAQ_LIST_INFO == XCP_ON
XCP_STATIC void Xcp_GetDaqListInfo_Res(Xcp_PDUType const * const pdu) {
    const XcpDaq_ListIntegerType        daqListNumber = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    XcpDaq_ListConfigurationType const *listConf      = XCP_NULL;
    XcpDaq_ListStateType               *listState     = XCP_NULL;
    uint8_t                             properties    = UINT8(0x00);

    DBG_TRACE2("GET_DAQ_LIST_INFO [daq: %u] \n", daqListNumber);
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    if (daqListNumber >= XcpDaq_GetListCount()) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
    }

    listConf  = XcpDaq_GetListConfiguration(daqListNumber);
    listState = XcpDaq_GetListState(daqListNumber);

    properties |= DAQ_LIST_PROPERTY_PREDEFINED;  /* Hardcoded for now. */
    properties |= DAQ_LIST_PROPERTY_EVENT_FIXED; /* "                " */
    properties |= DAQ_LIST_PROPERTY_DAQ;         /* "                " */

        #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    Xcp_Send8(
        UINT8(8), UINT8(0xff), properties, listConf->numOdts, 0, /* Hardcoded for now. */
        0,                                                       /* Hardcoded for now. */
        UINT8(0), UINT8(0), UINT8(0)
    );
        #else
    Xcp_ErrorResponse(UINT8(ERR_CMD_SYNTAX));
        #endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */
}
    #endif /* XCP_ENABLE_GET_DAQ_LIST_INFO */

    #if XCP_ENABLE_WRITE_DAQ_MULTIPLE == XCP_ON
XCP_STATIC void Xcp_WriteDaqMultiple_Res(Xcp_PDUType const * const pdu) {
    XcpDaq_ODTEntryType *entry = XCP_NULL;

    uint8_t numElements = Xcp_GetByte(pdu, UINT8(1));
    /*
        const uint8_t bitOffset = Xcp_GetByte(pdu, UINT8(1));
        const uint8_t elemSize  = Xcp_GetByte(pdu, UINT8(2));
        const uint8_t adddrExt  = Xcp_GetByte(pdu, UINT8(3));
        const uint32_t address  = Xcp_GetDWord(pdu, UINT8(4));
    */

    DBG_TRACE2("WRITE_DAQ_MULTIPLE [numElements: 0x%08x]\n", numElements);

    XCP_ASSERT_DAQ_STOPPED();
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
}
    #endif  // XCP_ENABLE_WRITE_DAQ_MULTIPLE

    #if XCP_ENABLE_FREE_DAQ == XCP_ON
XCP_STATIC void Xcp_FreeDaq_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("FREE_DAQ\n");
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    XCP_ASSERT_PGM_IDLE();
    Xcp_SendResult(XcpDaq_Free());
}
    #endif /* XCP_ENABLE_FREE_DAQ */

    #if XCP_ENABLE_ALLOC_DAQ == XCP_ON
XCP_STATIC void Xcp_AllocDaq_Res(Xcp_PDUType const * const pdu) {
    XcpDaq_ListIntegerType daqCount = (XcpDaq_ListIntegerType)0;

    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    daqCount = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    DBG_TRACE2("ALLOC_DAQ [count: %u] \n", daqCount);
    Xcp_SendResult(XcpDaq_Alloc(daqCount));
}
    #endif

    #if XCP_ENABLE_ALLOC_ODT == XCP_ON
XCP_STATIC void Xcp_AllocOdt_Res(Xcp_PDUType const * const pdu) {
    XcpDaq_ListIntegerType daqListNumber = (XcpDaq_ListIntegerType)0;
    XcpDaq_ODTIntegerType  odtCount      = (XcpDaq_ODTIntegerType)0;

    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    daqListNumber = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    odtCount      = (XcpDaq_ODTIntegerType)Xcp_GetByte(pdu, UINT8(4));
    DBG_TRACE3("ALLOC_ODT [daq: %u count: %u] \n", daqListNumber, odtCount);
    Xcp_SendResult(XcpDaq_AllocOdt(daqListNumber, odtCount));
}
    #endif /* XCP_ENABLE_ALLOC_ODT */

    #if XCP_ENABLE_ALLOC_ODT_ENTRY == XCP_ON
XCP_STATIC void Xcp_AllocOdtEntry_Res(Xcp_PDUType const * const pdu) {
    XcpDaq_ListIntegerType     daqListNumber   = (XcpDaq_ListIntegerType)0;
    XcpDaq_ODTIntegerType      odtNumber       = (XcpDaq_ODTIntegerType)0;
    XcpDaq_ODTEntryIntegerType odtEntriesCount = (XcpDaq_ODTEntryIntegerType)0;

    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    daqListNumber   = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    odtNumber       = (XcpDaq_ODTIntegerType)Xcp_GetByte(pdu, UINT8(4));
    odtEntriesCount = (XcpDaq_ODTEntryIntegerType)Xcp_GetByte(pdu, UINT8(5));
    DBG_TRACE4("ALLOC_ODT_ENTRY: [daq: %u odt: %u count: %u]\n", daqListNumber, odtNumber, odtEntriesCount);
    Xcp_SendResult(XcpDaq_AllocOdtEntry(daqListNumber, odtNumber, odtEntriesCount));
}
    #endif /* XCP_ENABLE_ALLOC_ODT_ENTRY */

    #if XCP_ENABLE_GET_DAQ_CLOCK == XCP_ON
XCP_STATIC void Xcp_GetDaqClock_Res(Xcp_PDUType const * const pdu) {
    uint32_t timestamp = UINT32(0);

    XCP_ASSERT_PGM_IDLE();
        #if XCP_DAQ_CLOCK_ACCESS_ALWAYS_SUPPORTED == XCP_OFF
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
        #endif /* XCP_DAQ_ENABLE_CLOCK_ACCESS_ALWAYS */

    timestamp = XcpHw_GetTimerCounter();
    DBG_TRACE2("GET_DAQ_CLOCK [timestamp: %u]\n", timestamp);
    Xcp_Send8(
        UINT8(8), UINT8(0xff), UINT8(0), UINT8(0), UINT8(0), XCP_LOBYTE(XCP_LOWORD(timestamp)), XCP_HIBYTE(XCP_LOWORD(timestamp)),
        XCP_LOBYTE(XCP_HIWORD(timestamp)), XCP_HIBYTE(XCP_HIWORD(timestamp))
    );
}
    #endif /* XCP_ENABLE_GET_DAQ_CLOCK */

    #if XCP_ENABLE_GET_DAQ_RESOLUTION_INFO == XCP_ON
XCP_STATIC void Xcp_GetDaqResolutionInfo_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("GET_DAQ_RESOLUTION_INFO\n");
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    Xcp_Send8(
        UINT8(8), UINT8(0xff), UINT8(1),   /* Granularity for size of ODT entry (DIRECTION = DAQ) */
        UINT8(XCP_DAQ_MAX_ODT_ENTRY_SIZE), /* Maximum size of ODT entry
                                              (DIRECTION = DAQ) */
        UINT8(1),                          /* Granularity for size of ODT entry (DIRECTION = STIM) */
        UINT8(XCP_DAQ_MAX_ODT_ENTRY_SIZE), /* Maximum size of ODT entry
                                              (DIRECTION = STIM) */
        UINT8(0x34),                       /* Timestamp unit and size */
        UINT8(1),                          /* Timestamp ticks per unit (WORD) */
        UINT8(0)
    );
}
    #endif /* XCP_ENABLE_GET_DAQ_RESOLUTION_INFO */

    #if XCP_ENABLE_GET_DAQ_EVENT_INFO == XCP_ON
XCP_STATIC void Xcp_GetDaqEventInfo_Res(Xcp_PDUType const * const pdu) {
    uint16_t                eventChannel = Xcp_GetWord(pdu, UINT8(2));
    uint8_t                 nameLen      = UINT8(0);
    XcpDaq_EventType const *event        = XCP_NULL;

    DBG_TRACE2("GET_DAQ_EVENT_INFO [eventChannel: %d]\n", eventChannel);
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    if (eventChannel >= UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)) {
        Xcp_SendResult(ERR_OUT_OF_RANGE);
        return;
    }
    event = XcpDaq_GetEventConfiguration(eventChannel);
    if ((event->name == XCP_NULL) || (event->nameLen == UINT8(0))) {
        nameLen = UINT8(0);
    } else {
        nameLen = event->nameLen;
        Xcp_SetMta(Xcp_GetNonPagedAddress(event->name));
    }

    Xcp_Send8(
        UINT8(7), UINT8(0xff), UINT8(event->properties), /* DAQ_EVENT_PROPERTIES */
        UINT8(1),                                        /* maximum number of DAQ lists in this event channel */
        UINT8(nameLen),                                  /* EVENT_CHANNEL_NAME_LENGTH in bytes 0 – If not
                                                            available */
        UINT8(event->cycle),                             /* EVENT_CHANNEL_TIME_CYCLE 0 – Not cyclic */
        UINT8(event->timeunit),                          /* EVENT_CHANNEL_TIME_UNIT don’t care if
                                                            Event channel time cycle = 0 */
        UINT8(0),                                        /* EVENT_CHANNEL_PRIORITY (FF highest) */
        UINT8(0)
    );
}
    #endif /* XCP_ENABLE_GET_DAQ_EVENT_INFO */

#endif /* XCP_ENABLE_DAQ_COMMANDS */

/*
**
** PGM Services.
**
*/
#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
XCP_STATIC void Xcp_ProgramStart_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("PROGRAM_START\n");
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_PGM);
    XCP_ASSERT_DAQ_STOPPED();
    XcpPgm_SetProcessorState(XCP_PGM_ACTIVE);
    #if 0
0 BYTE Packet ID: 0xFF
1 BYTE Reserved
2 BYTE COMM_MODE_PGM
3 BYTE MAX_CTO_PGM [BYTES]Maximum CTO size for PGM
4 BYTE MAX_BS_PGM
5 BYTE MIN_ST_PGM
6 BYTE QUEUE_SIZE_PGM
    #endif
    Xcp_Send8(
        UINT8(7), UINT8(0xff), UINT8(0), /* Reserved */
        UINT8(0),                        /* COMM_MODE_PGM */
        UINT8(XCP_MAX_CTO),              /* MAX_CTO_PGM [BYTES]Maximum CTO size for PGM */
        UINT8(XCP_MAX_BS_PGM),           /* MAX_BS_PGM */
        UINT8(XCP_MIN_ST_PGM),           /* MIN_ST_PGM */
        UINT8(0),                        /* QUEUE_SIZE_PGM */
        UINT8(0)
    );
}

XCP_STATIC void Xcp_ProgramClear_Res(Xcp_PDUType const * const pdu) {
    uint8_t  mode       = Xcp_GetByte(pdu, UINT8(1));
    uint32_t clearRange = (XcpDaq_ListIntegerType)Xcp_GetDWord(pdu, UINT8(4));

    DBG_TRACE3("PROGRAM_CLEAR [mode: %d clearRange: 0x%x]\n", mode, clearRange);
    XCP_ASSERT_PGM_ACTIVE();
}

XCP_STATIC void Xcp_Program_Res(Xcp_PDUType const * const pdu) {
    uint8_t     len = Xcp_GetByte(pdu, UINT8(1));
    Xcp_MtaType src = { 0 };

    DBG_TRACE2("PROGRAM [len: %u]\n", len);

    XCP_ASSERT_PGM_ACTIVE();
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, len, XCP_MEM_ACCESS_WRITE, (bool)XCP_TRUE);
    src.address = (uint32_t)pdu->data + 2;
    src.ext     = UINT8(0);
    //    Xcp_CopyMemory(Xcp_State.mta, src, (uint32_t)len);

    XCP_INCREMENT_MTA(len);

    Xcp_PositiveResponse();
}

XCP_STATIC void Xcp_ProgramReset_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("PROGRAM_RESET\n");
    XCP_ASSERT_PGM_ACTIVE();

    XcpPgm_SetProcessorState(XCP_PGM_IDLE);
    Xcp_PositiveResponse();
}

    #if XCP_ENABLE_GET_PGM_PROCESSOR_INFO == XCP_ON
XCP_STATIC void Xcp_GetPgmProcessorInfo_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("GET_PGM_PROCESSOR_INFO\n");
    /* XCP_ASSERT_UNLOCKED(XCP_RESOURCE_PGM); */

    Xcp_Send8(
        UINT8(3), UINT8(0xff), XCP_PGM_PROPERIES, /* PGM_PROPERTIES */
        XCP_MAX_SECTOR_PGM,                       /* MAX_SECTOR */
        UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0)
    );
}
    #endif /* XCP_ENABLE_GET_PGM_PROCESSOR_INFO */

    #if XCP_ENABLE_GET_SECTOR_INFO == XCP_ON
XCP_STATIC void Xcp_GetSectorInfo_Res(Xcp_PDUType const * const pdu) {
    uint8_t mode         = Xcp_GetByte(pdu, UINT8(1));
    uint8_t sectorNumber = Xcp_GetByte(pdu, UINT8(2));

    DBG_TRACE3("GET_SECTOR_INFO [sectorNumber: %d mode :%d]\n", sectorNumber, mode);

    /* XCP_ASSERT_UNLOCKED(XCP_RESOURCE_PGM); */

    if (sectorNumber >= XCP_MAX_SECTOR_PGM) {
        Xcp_SendResult(ERR_SEGMENT_NOT_VALID);
        return;
    }

    if (mode == UINT8(0)) {
        /* Start-Address */
    } else if (mode == UINT8(1)) {
        /* Length */
    } else {
        Xcp_SendResult(ERR_MODE_NOT_VALID);
        return;
    }

    Xcp_Send8(
        UINT8(8), UINT8(0xff), UINT8(sectorNumber * 2), /* Clear Sequence Number */
        UINT8((sectorNumber * 2) + 1),                  /* Program Sequence Number */
        UINT8(0),                                       /* Programming method */
        UINT8(0),                                       /* SECTOR_INFO - DWORD*/
        UINT8(0), UINT8(0), UINT8(0)
    );
}
    #endif /* XCP_ENABLE_GET_SECTOR_INFO */

    #if XCP_ENABLE_PROGRAM_PREPARE == XCP_ON
XCP_STATIC void Xcp_ProgramPrepare_Res(Xcp_PDUType const * const pdu) {
    uint16_t codeSize = Xcp_GetWord(pdu, UINT8(2));

    DBG_TRACE2("PROGRAM_PREPARE [codeSize: %d]\n", codeSize);
    XCP_ASSERT_PGM_ACTIVE();

    Xcp_PositiveResponse();
}
    #endif /* XCP_ENABLE_PROGRAM_PREPARE */

    #if XCP_ENABLE_PROGRAM_FORMAT == XCP_ON
XCP_STATIC void Xcp_ProgramFormat_Res(Xcp_PDUType const * const pdu) {
    uint8_t compressionMethod = Xcp_GetByte(pdu, UINT8(1));
    uint8_t encryptionMethod  = Xcp_GetByte(pdu, UINT8(2));
    uint8_t programmingMethod = Xcp_GetByte(pdu, UINT8(3));
    uint8_t accessMethod      = Xcp_GetByte(pdu, UINT8(4));

    DBG_TRACE5(
        "PROGRAM_FORMAT [compression: %d encryption: %d programming: %d "
        "access: "
        "%d]\n",
        compressionMethod, encryptionMethod, programmingMethod, accessMethod
    );
    XCP_ASSERT_PGM_ACTIVE();

    Xcp_PositiveResponse();
}
    #endif /* XCP_ENABLE_PROGRAM_FORMAT */

    #if XCP_ENABLE_PROGRAM_NEXT == XCP_ON
XCP_STATIC void Xcp_ProgramNext_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("PROGRAM_NEXT\n");
    XCP_ASSERT_PGM_ACTIVE();
}
    #endif /* XCP_ENABLE_PROGRAM_NEXT */

    #if XCP_ENABLE_PROGRAM_MAX == XCP_ON
XCP_STATIC void Xcp_ProgramMax_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("PROGRAM_MAX\n");
    XCP_ASSERT_PGM_ACTIVE();
}
    #endif /* XCP_ENABLE_PROGRAM_MAX */

    #if XCP_ENABLE_PROGRAM_VERIFY == XCP_ON
XCP_STATIC void Xcp_ProgramVerify_Res(Xcp_PDUType const * const pdu) {
    DBG_TRACE1("PROGRAM_VERIFY\n");
    XCP_ASSERT_PGM_ACTIVE();
}
    #endif /* XCP_ENABLE_PROGRAM_VERIFY */

#endif /* XCP_ENABLE_PGM_COMMANDS */

/*
**  Helpers.
*/
XCP_STATIC void Xcp_SendResult(Xcp_ReturnType result) {
    if (result == ERR_SUCCESS) {
        Xcp_PositiveResponse();
    } else {
        Xcp_ErrorResponse(UINT8(result));
    }
}

void Xcp_WriteMemory(void *dest, void *src, uint16_t count) {
    XcpUtl_MemCopy(dest, src, UINT32(count));
}

#if XCP_REPLACE_STD_COPY_MEMORY == XCP_OFF
void Xcp_CopyMemory(Xcp_MtaType dst, Xcp_MtaType src, uint32_t len) {
    #if XCP_ENABLE_ADDRESS_MAPPER == XCP_ON
    Xcp_MtaType                 tmpD = { 0 };
    Xcp_MtaType                 tmpS = { 0 };
    Xcp_MemoryMappingResultType res  = XCP_MEMORY_ADDRESS_INVALID;

    tmpD.address = dst.address;
    tmpS.address = src.address;

    res = Xcp_HookFunction_AddressMapper(&tmpD, &dst);
    if (res == XCP_MEMORY_MAPPED) {
    } else if (res == XCP_MEMORY_NOT_MAPPED) {
    } else if (res == XCP_MEMORY_ADDRESS_INVALID) {
    } else {
    }
    res = Xcp_HookFunction_AddressMapper(&tmpS, &src);
    if (res == XCP_MEMORY_MAPPED) {
    } else if (res == XCP_MEMORY_NOT_MAPPED) {
    } else if (res == XCP_MEMORY_ADDRESS_INVALID) {
    } else {
    }
    XcpUtl_MemCopy((void *)tmpD.address, (void *)tmpS.address, len);
    #else

    /* Without address-mapper we don't know how to handle address extensions. */
    XcpUtl_MemCopy((void *)dst.address, (void *)src.address, len);
    #endif /* XCP_ENABLE_ADDRESS_MAPPER */
}
#endif /* XCP_REPLACE_STD_COPY_MEMORY */

INLINE uint8_t Xcp_GetByte(Xcp_PDUType const * const pdu, uint8_t offs) {
    return (*(pdu->data + offs));
}

INLINE uint16_t Xcp_GetWord(Xcp_PDUType const * const pdu, uint8_t offs) {
    return (*(pdu->data + offs)) | ((*(pdu->data + 1 + offs)) << 8);
}

INLINE uint32_t Xcp_GetDWord(Xcp_PDUType const * const pdu, uint8_t offs) {
    uint16_t h;
    uint16_t l;

    l = (*(pdu->data + offs)) | ((*(pdu->data + 1 + offs)) << 8);
    h = (*(pdu->data + 2 + offs)) | ((*(pdu->data + 3 + offs)) << 8);
    /* l = Xcp_GetWord(pdu, 0); */
    /* h = Xcp_GetWord(pdu, 2); */

    return (uint32_t)(h * 0x10000) + l;
}

INLINE void Xcp_SetByte(Xcp_PDUType const * const pdu, uint8_t offs, uint8_t value) {
    (*(pdu->data + offs)) = value;
}

INLINE void Xcp_SetWord(Xcp_PDUType const * const pdu, uint8_t offs, uint16_t value) {
    (*(pdu->data + offs))     = value & 0xff;
    (*(pdu->data + 1 + offs)) = (value & 0xff00) >> 8;
}

INLINE void Xcp_SetDWord(Xcp_PDUType const * const pdu, uint8_t offs, uint32_t value) {
    (*(pdu->data + offs))     = value & 0xff;
    (*(pdu->data + 1 + offs)) = (value & 0xff00) >> 8;
    (*(pdu->data + 2 + offs)) = (value & 0xff0000) >> 16;
    (*(pdu->data + 3 + offs)) = (value & 0xff000000) >> 24;
}

XCP_STATIC uint8_t Xcp_SetResetBit8(uint8_t result, uint8_t value, uint8_t flag) {
    if ((value & flag) == flag) {
        result |= flag;
    } else {
        result &= ~(flag);
    }
    return result;
}

#if XCP_ENABLE_RESOURCE_PROTECTION == XCP_ON
XCP_STATIC bool Xcp_IsProtected(uint8_t resource) {
    return ((Xcp_State.resourceProtection & resource) == resource);
}
#endif /* XCP_ENABLE_RESOURCE_PROTECTION */

void Xcp_SetBusy(bool enable) {
    XCP_ENTER_CRITICAL();
    Xcp_State.busy = enable;
    XCP_LEAVE_CRITICAL();
}

bool Xcp_IsBusy(void) {
    return Xcp_State.busy;
}

Xcp_StateType *Xcp_GetState(void) {
    Xcp_StateType *tState;

    XCP_DAQ_ENTER_CRITICAL();
    tState = &Xcp_State;
    XCP_DAQ_LEAVE_CRITICAL();

    return tState;
}

XCP_STATIC void Xcp_PositiveResponse(void) {
    Xcp_Send8(UINT8(1), UINT8(0xff), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0));
}

XCP_STATIC void Xcp_ErrorResponse(uint8_t errorCode) {
    Xcp_Send8(UINT8(2), UINT8(0xfe), UINT8(errorCode), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0));
}

XCP_STATIC void Xcp_BusyResponse(void) {
#if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_State.statistics.crosBusy++;
#endif /* XCP_ENABLE_STATISTICS */
    Xcp_ErrorResponse(ERR_CMD_BUSY);
}

#if 0
XCP_STATIC Xcp_MemoryMappingResultType Xcp_MapMemory(Xcp_MtaType const * src, Xcp_MtaType * dst)
{
    Xcp_MemoryMappingResultType mappingResult;

/*    FlsEmu_MemoryMapper(tmpAddr); */

    Xcp_MtaType dest;
    printf("addr: %x ext: %d\n", mta->address, mta->ext);
    if ((mta->address >= 0x4000) && (mta->address < 0x5000)) {
        mta->address = (0x00A90000 - 0x4000) + mta->address;
    }
    printf("MAPPED: addr: %x ext: %d\n", mta->address, mta->ext);
}
#endif

XCP_STATIC void Xcp_Download_Copy(uint32_t address, uint8_t ext, uint32_t len) {
    Xcp_MtaType src = { 0 };

    src.address = address;
    src.ext     = ext;
    Xcp_CopyMemory(Xcp_State.mta, src, (uint32_t)len);
    XCP_INCREMENT_MTA(len);
}

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
void XcpPgm_SetProcessorState(XcpPgm_ProcessorStateType state) {
    Xcp_StateType *Xcp_State = { 0 };

    Xcp_State = Xcp_GetState();
    XCP_PGM_ENTER_CRITICAL();
    Xcp_State->pgmProcessor.state = state;
    XCP_PGM_LEAVE_CRITICAL();
}
#endif /* ENABLE_PGM_COMMANDS */

#if defined(_WIN32)
void Xcp_DisplayInfo(void) {
    XcpTl_PrintConnectionInformation();
    printf("Press h for help.\n");
    fflush(stdout);
}
#endif
