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

#include "xcp.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
** Private Options.
*/
#define XCP_ENABLE_STD_COMMANDS                     XCP_ON
#define XCP_ENABLE_INTERLEAVED_MODE                 XCP_OFF

#define XCP_DRIVER_VERSION  10


typedef struct tagXcp_DaqProcessorType {
    bool running;
} Xcp_DaqProcessorType;


typedef struct tagXcp_StateType {
    bool connected;
    bool busy;
#if defined(XCP_ENABLE_DAQ_COMMANDS)
    Xcp_DaqProcessorType daq;
#endif
    bool programming;
    uint8_t mode;
    uint8_t protection;
    Xcp_MtaType mta;
} Xcp_StateType;

Xcp_PDUType Xcp_PduIn;
Xcp_PDUType Xcp_PduOut;


static Xcp_ConnectionStateType Xcp_ConnectionState = XCP_DISCONNECTED;
void Xcp_WriteMemory(void * dest, void * src, uint16_t count);
void Xcp_ReadMemory(void * dest, void * src, uint16_t count);

static Xcp_StateType Xcp_State;

static Xcp_SendCalloutType Xcp_SendCallout = NULL;
static const Xcp_StationIDType Xcp_StationID = { sizeof(XCP_STATION_ID), XCP_STATION_ID };


#define XCP_COMMAND     (cmoIn->data[0])

#define DATA_IN(idx)    (cmoIn->data[(idx)])
#define DATA_OUT(idx)   (cmoOut->data[(idx)])

#define COUNTER_IN      (cmoIn->data[1])
#define COUNTER_OUT     (cmoOut->data[2])


/*
** Local Function Prototypes.
*/
static Xcp_MtaType Xcp_GetNonPagedAddress(void const * const ptr);
static void Xcp_SetMta(Xcp_MtaType mta);

static void Xcp_ErrorResponse(uint8_t errorCode);

static void Xcp_CommandNotImplemented_Res(Xcp_PDUType const * const pdu);
static void Xcp_Connect_Res(Xcp_PDUType const * const pdu);
static void Xcp_Disconnect_Res(Xcp_PDUType const * const pdu);
static void Xcp_GetStatus_Res(Xcp_PDUType const * const pdu);
static void Xcp_Synch_Res(Xcp_PDUType const * const pdu);
#if XCP_ENABLE_GET_COMM_MODE_INFO == XCP_ON
static void Xcp_GetCommModeInfo_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_COMM_MODE_INFO
#if XCP_ENABLE_GET_ID == XCP_ON
static void Xcp_GetId_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_ID
#if XCP_ENABLE_SET_REQUEST == XCP_ON
static void Xcp_SetRequest_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_SET_REQUEST
#if XCP_ENABLE_GET_SEED == XCP_ON
static void Xcp_GetSeed_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_SEED
#if XCP_ENABLE_UNLOCK == XCP_ON
static void Xcp_Unlock_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_UNLOCK
#if XCP_ENABLE_SET_MTA == XCP_ON
static void Xcp_SetMta_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_SET_MTA
#if XCP_ENABLE_UPLOAD == XCP_ON
static void Xcp_Upload_Res(Xcp_PDUType const * const pdu);
#endif
#if XCP_ENABLE_SHORT_UPLOAD == XCP_ON
static void Xcp_ShortUpload_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_SHORT_UPLOAD
#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON
static void Xcp_BuildChecksum_Res(Xcp_PDUType const * const pdu);
#endif
#if XCP_ENABLE_TRANSPORT_LAYER_CMD == XCP_ON
static void Xcp_TransportLayerCmd_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_TRANSPORT_LAYER_CMD
#if XCP_ENABLE_USER_CMD == XCP_ON
static void Xcp_UserCmd_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_USER_CMD

#if XCP_ENABLE_CAL_COMMANDS == XCP_ON
static void Xcp_Download_Res(Xcp_PDUType const * const pdu);
#if XCP_ENABLE_DOWNLOAD_NEXT == XCP_ON
static void Xcp_DownloadNext_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_DOWNLOAD_NEXT
#if XCP_ENABLE_DOWNLOAD_MAX == XCP_ON
static void Xcp_DownloadMax_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_DOWNLOAD_MAX
#if XCP_ENABLE_SHORT_DOWNLOAD == XCP_ON
static void Xcp_ShortDownload_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_SHORT_DOWNLOAD
#if XCP_ENABLE_MODIFY_BITS == XCP_ON
static void Xcp_ModifyBits_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_MODIFY_BITS

static void Xcp_SetCalPage_Res(Xcp_PDUType const * const pdu);
static void Xcp_GetCalPage_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_CAL_COMMANDS

#if XCP_ENABLE_PAG_COMMANDS == XCP_ON
#if XCP_ENABLE_GET_PAG_PROCESSOR_INFO == XCP_ON
static void Xcp_GetPagProcessorInfo_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_PAG_PROCESSOR_INFO
#if XCP_ENABLE_GET_SEGMENT_INFO == XCP_ON
static void Xcp_GetSegmentInfo_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_SEGMENT_INFO
#if XCP_ENABLE_GET_PAGE_INFO == XCP_ON
static void Xcp_GetPageInfo_Res(Xcp_PDUType const * const pdu);
#endif
#if XCP_ENABLE_SET_SEGMENT_MODE == XCP_ON
static void Xcp_SetSegmentMode_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_SET_SEGMENT_MODE
#if XCP_ENABLE_GET_SEGMENT_MODE == XCP_ON
static void Xcp_GetSegmentMode_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_SEGMENT_MODE
#if XCP_ENABLE_COPY_CAL_PAGE
static void Xcp_CopyCalPage_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_COPY_CAL_PAGE
#endif // XCP_ENABLE_PAG_COMMANDS

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
static void Xcp_ClearDaqList_Res(Xcp_PDUType const * const pdu);
static void Xcp_SetDaqPtr_Res(Xcp_PDUType const * const pdu);
static void Xcp_WriteDaq_Res(Xcp_PDUType const * const pdu);
static void Xcp_SetDaqListMode_Res(Xcp_PDUType const * const pdu);
static void Xcp_GetDaqListMode_Res(Xcp_PDUType const * const pdu);
static void Xcp_StartStopDaqList_Res(Xcp_PDUType const * const pdu);
static void Xcp_StartStopSynch_Res(Xcp_PDUType const * const pdu);
#if XCP_ENABLE_GET_DAQ_CLOCK == XCP_ON
static void Xcp_GetDaqClock_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_DAQ_CLOCK
#if XCP_ENABLE_READ_DAQ == XCP_ON
static void Xcp_ReadDaq_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_READ_DAQ
#if XCP_ENABLE_GET_DAQ_PROCESSOR_INFO == XCP_ON
static void Xcp_GetDaqProcessorInfo_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_DAQ_PROCESSOR_INFO
#if XCP_ENABLE_GET_DAQ_RESOLUTION_INFO == XCP_ON
static void Xcp_GetDaqResolutionInfo_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_DAQ_RESOLUTION_INFO
#if XCP_ENABLE_GET_DAQ_LIST_INFO == XCP_ON
static void Xcp_GetDaqListInfo_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_DAQ_LIST_INFO
#if XCP_ENABLE_GET_DAQ_EVENT_INFO == XCP_ON
static void Xcp_GetDaqEventInfo_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_DAQ_EVENT_INFO
#if XCP_ENABLE_FREE_DAQ == XCP_ON
static void Xcp_FreeDaq_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_FREE_DAQ
#if XCP_ENABLE_ALLOC_DAQ == XCP_ON
static void Xcp_AllocDaq_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_ALLOC_DAQ
#if XCP_ENABLE_ALLOC_ODT == XCP_ON
static void Xcp_AllocOdt_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_ALLOC_ODT
#if XCP_ENABLE_ALLOC_ODT_ENTRY == XCP_ON
static void Xcp_AllocOdtEntry_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_ALLOC_ODT_ENTRY
#endif // XCP_ENABLE_DAQ_COMMANDS

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
static void Xcp_ProgramStart_Res(Xcp_PDUType const * const pdu);
static void Xcp_ProgramClear_Res(Xcp_PDUType const * const pdu);
static void Xcp_Program_Res(Xcp_PDUType const * const pdu);
static void Xcp_ProgramReset_Res(Xcp_PDUType const * const pdu);
#if XCP_ENABLE_GET_PGM_PROCESSOR_INFO == XCP_ON
static void Xcp_GetPgmProcessorInfo_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_PGM_PROCESSOR_INFO
#if XCP_ENABLE_GET_SECTOR_INFO == XCP_ON
static void Xcp_GetSectorInfo_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_GET_SECTOR_INFO
#if XCP_ENABLE_PROGRAM_PREPARE == XCP_ON
static void Xcp_ProgramPrepare_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_PROGRAM_PREPARE
#if XCP_ENABLE_PROGRAM_FORMAT == XCP_ON
static void Xcp_ProgramFormat_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_PROGRAM_FORMAT
#if XCP_ENABLE_PROGRAM_NEXT == XCP_ON
static void Xcp_ProgramNext_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_PROGRAM_NEXT
#if XCP_ENABLE_PROGRAM_MAX == XCP_ON
static void Xcp_ProgramMax_Res(Xcp_PDUType const * const pdu);
#endif // XCP_ENABLE_PROGRAM_MAX
#if XCP_ENABLE_PROGRAM_VERIFY == XCP_ON
static void Xcp_ProgramVerify_Res(Xcp_PDUType const * const pdu);
#endif
#endif // XCP_ENABLE_PGM_COMMANDS


static void Xcp_SetMta(Xcp_MtaType mta)
{
    Xcp_State.mta = mta;
}

static Xcp_MtaType Xcp_GetNonPagedAddress(void const * const ptr)
{
    Xcp_MtaType mta;

    mta.ext = 0;
    mta.address = (uint32_t)ptr;
    return mta;
}

void Xcp_CopyMemory(Xcp_MtaType dst, Xcp_MtaType src, uint32_t len)
{
    if (dst.ext == 0 && src.ext == 0) {
        DBG_PRINT("LEN: %u\n", len);
        DBG_PRINT("dst: %08X src: %08x\n", dst.address, src.address);
        Xcp_MemCopy((void*)dst.address, (void*)src.address, len);
    } else {
        // We need assistance...
    }
}


void Xcp_SendPdu(void)
{
    uint16_t len = Xcp_PduOut.len;

    Xcp_PduOut.data[0] = LOBYTE(len);
    Xcp_PduOut.data[1] = HIBYTE(len);
    Xcp_PduOut.data[2] = 0;
    Xcp_PduOut.data[3] = 0;

    //DBG_PRINT("Sending PDU: ");
    //hexdump(Xcp_PduOut.data, Xcp_PduOut.len + 4);

    XcpTl_Send(Xcp_PduOut.data, Xcp_PduOut.len + 4);
}


uint8_t * Xcp_GetOutPduPtr(void)
{
    return &(Xcp_PduOut.data[4]);
}

void Xcp_SetPduOutLen(uint16_t len)
{
    Xcp_PduOut.len = len;
}

void Xcp_Send8(uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7)
{

    uint8_t * dataOut = Xcp_GetOutPduPtr();

    Xcp_SetPduOutLen(len);

    /* Controlled fall-through */
    switch(len) {
        case 8:
            dataOut[7] = b7;
        case 7:
            dataOut[6] = b6;
        case 6:
            dataOut[5] = b5;
        case 5:
            dataOut[4] = b4;
        case 4:
            dataOut[3] = b3;
        case 3:
            dataOut[2] = b2;
        case 2:
            dataOut[1] = b1;
        case 1:
            dataOut[0] = b0;
    }

    Xcp_SendPdu();
}

static void Xcp_Upload(uint8_t len)
{
    //uint8_t len = pdu->data[1];
    uint8_t * dataOut = Xcp_GetOutPduPtr();
    Xcp_MtaType dst;

// TODO: RangeCheck / Blockmode!!!
    dataOut[0] = 0xff;

    dst.address = (uint32_t)(dataOut + 1);
    dst.ext = 0;

    Xcp_CopyMemory(dst, Xcp_State.mta, len);

    Xcp_State.mta.address += len;   // Advance MTA.

    Xcp_SetPduOutLen(len);
    Xcp_SendPdu();
}

/*
** Big, fat jump table.
*/
static const Xcp_ServerCommandType Xcp_ServerCommands[] = {

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
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
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
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
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
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
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
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
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
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
#endif
};

/*
**  Global Functions.
*/

void Xcp_Init(void)
{
    DBG_PRINT("Xcp_Init()\n");
    Xcp_ConnectionState = XCP_DISCONNECTED;

    Xcp_MemSet(&Xcp_State, '\x00', sizeof(Xcp_StateType));

#if XCP_PROTECT_CAL == XCP_ON || XCP_PROTECT_PAG == XCP_ON
    Xcp_State.protection |= XCP_RESOURCE_CAL_PAG;
#endif // XCP_PROTECT_CAL
#if XCP_PROTECT_DAQ == XCP_ON
    Xcp_State.protection |= XCP_RESOURCE_DAQ;
#endif // XCP_PROTECT_DAQ
#if XCP_PROTECT_STIM == XCP_ON
    Xcp_State.protection |= XCP_RESOURCE_STIM;
#endif // XCP_PROTECT_STIM
#if XCP_PROTECT_PGM == XCP_ON
    Xcp_State.protection |= XCP_RESOURCE_PGM;
#endif // XCP_PROTECT_PGM

    XcpHw_Init();
    XcpTl_Init();
}

#if 0
void Xcp_SetDTOValues(Xcp_PDUType * cmoOut, uint8_t type, uint8_t returnCode,
          uint8_t counter, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
    cmoOut->canID = XCP_MASTER_CANID;
    cmoOut->dlc = 8;
    DATA_OUT(0) = type;
    DATA_OUT(1) = returnCode;
    DATA_OUT(2) = counter;
    DATA_OUT(3) = b0;
    DATA_OUT(4) = b1;
    DATA_OUT(5) = b2;
    DATA_OUT(6) = b3;
    DATA_OUT(7) = b4;
}


void Xcp_SetUploadDTO(Xcp_PDUType * cmoOut, uint8_t type, uint8_t returnCode, uint8_t counter)
{
    cmoOut->canID = XCP_MASTER_CANID;
    cmoOut->dlc = 8;
    DATA_OUT(0) = type;
    DATA_OUT(1) = returnCode;
    DATA_OUT(2) = counter;
    /* Payload already set. */
}


#define Xcp_AcknowledgedCRM(ctr, b0, b1, b2, b3, b4)    Xcp_SetDTOValues(&cmoOut, COMMAND_RETURN_MESSAGE, 0, ctr, b0, b1, b2, b3, b4)
#define Xcp_AcknowledgedUploadCRM(ctr)                  Xcp_SetUploadDTO(&cmoOut, COMMAND_RETURN_MESSAGE, 0, ctr)

#endif

/**
 * Entry point, needs to be "wired" to CAN-Rx interrupt.
 *
 * @param pdu
 */
void Xcp_DispatchCommand(Xcp_PDUType const * const pdu)
{
    uint8_t cmd = pdu->data[0];

    DBG_PRINT("Req: ");
    Xcp_DumpMessageObject(pdu);

    if (Xcp_State.connected == TRUE) { // TODO: bool
        DBG_PRINT("CMD: [%02X]\n", cmd);
        Xcp_ServerCommands[0xff - cmd](pdu);   // TODO: range check!!!
    } else {    // not connected.
        if (pdu->data[0] == XCP_CONNECT) {
            Xcp_Connect_Res(pdu);
        } else {

        }
    }
#if defined(_MSC_VER)
    fflush(stdout);
#endif
}


void Xcp_SetSendCallout(Xcp_SendCalloutType callout)
{
    Xcp_SendCallout = callout;
}


/*
**
** Global Helper Functions.
**
** Note: These functions are only useful for unit-testing and debugging.
**
*/
Xcp_ConnectionStateType Xcp_GetConnectionState(void)
{
    return Xcp_ConnectionState;
}


void Xcp_DumpMessageObject(Xcp_PDUType const * pdu)
{
    //DBG_PRINT("LEN: %d CMD: %02X\n", pdu->len, pdu->data[0]);
    //DBG_PRINT("PTR: %p\n", pdu->data);
#if 0
    DBG_PRINT("%08X  %u  [%02X %02X %02X %02X %02X %02X %02X %02X]\n", cmo->canID, cmo->dlc,
           cmo->data[0], cmo->data[1], cmo->data[2], cmo->data[3], cmo->data[4], cmo->data[5], cmo->data[6], cmo->data[7]
    );
#endif
}

void Xcp_WriteMemory(void * dest, void * src, uint16_t count)
{
    Xcp_MemCopy(dest, src, count);
}

/*
** Local Functions.
*/
static void Xcp_ErrorResponse(uint8_t errorCode)
{
    //uint8_t * dataOut = XcpComm_GetPduOutPtr();
    DBG_PRINT("-> Error %02X: \n", errorCode);

    Xcp_Send8(2, 0xfe, errorCode, 0, 0, 0, 0, 0, 0);
#if 0
    Xcp_SetPduOutLen(2);
    dataOut[0] = 0xfe;
    dataOut[1] = errorCode;
    Xcp_SendPdu();
#endif
}


static void Xcp_CommandNotImplemented_Res(Xcp_PDUType const * const pdu)
{
    DBG_PRINT("Command not implemented [%02X].\n", pdu->data[0]);
    Xcp_ErrorResponse(ERR_CMD_UNKNOWN);
}


static void Xcp_Connect_Res(Xcp_PDUType const * const pdu)
{
    uint8_t resource = 0x00;
    uint8_t commModeBasic = 0x00;

    DBG_PRINT("CONNECT: \n");

    if (Xcp_State.connected == FALSE) {
        Xcp_State.connected = TRUE;
        // TODO: Init stuff
    }

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    resource |= XCP_RESOURCE_PGM;
#endif  // XCP_ENABLE_PGM_COMMANDS
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    resource |= XCP_RESOURCE_DAQ;
#endif  // XCP_ENABLE_DAQ_COMMANDS
#if XCP_ENABLE_CAL_COMMANDS == XCP_ON || XCP_ENABLE_PAG_COMMANDS == XCP_ON
    resource |= XCP_RESOURCE_CAL_PAG;
#endif
#if XCP_ENABLE_STIM == XCP_ON
    resource |= XCP_RESOURCE_STIM;
#endif  // XCP_ENABLE_STIM

    commModeBasic |= XCP_BYTE_ORDER;
    commModeBasic |= XCP_ADDRESS_GRANULARITY;
#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
    commModeBasic |= XCP_SLAVE_BLOCK_MODE;
#endif // XCP_ENABLE_SLAVE_BLOCKMODE
#if XCP_ENABLE_GET_COMM_MODE_INFO
    commModeBasic |= XCP_OPTIONAL_COMM_MODE;
#endif // XCP_ENABLE_GET_COMM_MODE_INFO


    XcpTl_SaveConnection();

    Xcp_Send8(8, 0xff, resource, commModeBasic, XCP_MAX_CTO, LOBYTE(XCP_MAX_DTO), HIBYTE(XCP_MAX_DTO), XCP_PROTOCOL_VERSION_MAJOR, XCP_TRANSPORT_LAYER_VERSION_MAJOR);
    //DBG_PRINT("MAX-DTO: %04X H: %02X L: %02X\n", XCP_MAX_DTO, HIBYTE(XCP_MAX_DTO), LOBYTE(XCP_MAX_DTO));
}


static void Xcp_Disconnect_Res(Xcp_PDUType const * const pdu)
{
    DBG_PRINT("DISCONNECT: \n");

    Xcp_Send8(1, 0xff, 0, 0, 0, 0, 0, 0, 0);
    XcpTl_ReleaseConnection();
}


static void Xcp_GetStatus_Res(Xcp_PDUType const * const pdu)   // TODO: Implement!!!
{
    DBG_PRINT("GET_STATUS: \n");

    Xcp_Send8(6, 0xff,
        0,     // Current session status
        Xcp_State.protection,  // Current resource protection status
        0x00,  // Reserved
        0,     // Session configuration id
        0,     // "                      "
        0, 0
    );
}


static void Xcp_Synch_Res(Xcp_PDUType const * const pdu)
{
    DBG_PRINT("SYNCH: \n");

    Xcp_Send8(2, 0xfe, ERR_CMD_SYNCH, 0, 0, 0, 0, 0, 0);
}


#if XCP_ENABLE_GET_COMM_MODE_INFO == XCP_ON
static void Xcp_GetCommModeInfo_Res(Xcp_PDUType const * const pdu)
{
    uint8_t commModeOptional = 0;

    DBG_PRINT("GET_COMM_MODE_INFO: \n");

#if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
    commModeOptional |= XCP_MASTER_BLOCK_MODE;
#endif // XCP_ENABLE_MASTER_BLOCKMODE

#if XCP_ENABLE_INTERLEAVED_MODE == XCP_ON
    commModeOptional |= XCP_INTERLEAVED_MODE;
#endif // XCP_ENABLE_INTERLEAVED_MODE

    Xcp_Send8(8, 0xff,
        0,     // Reserved
        commModeOptional,
        0,     // Reserved
        XCP_MAX_BS,
        XCP_MIN_ST,
        XCP_QUEUE_SIZE,
        XCP_DRIVER_VERSION
    );
}
#endif // XCP_ENABLE_GET_COMM_MODE_INFO


#if XCP_ENABLE_GET_ID == XCP_ON
static void Xcp_GetId_Res(Xcp_PDUType const * const pdu)
{
    uint8_t idType = pdu->data[1];
#if 0
0 BYTE Packet ID: 0xFF
1 BYTE Mode
2 WORD Reserved
4 DWORD Length [BYTE]
#endif

    DBG_PRINT("GET_ID [%u]: \n", idType);

    Xcp_SetMta(Xcp_GetNonPagedAddress(Xcp_StationID.name));

    Xcp_Send8(8, 0xff, 0, 0, 0, (uint8_t)Xcp_StationID.len, 0, 0, 0);
}
#endif // XCP_ENABLE_GET_ID


#if XCP_ENABLE_UPLOAD == XCP_ON
static void Xcp_Upload_Res(Xcp_PDUType const * const pdu)
{
    uint8_t len = pdu->data[1];

// TODO: RangeCheck / Blockmode!!!
    DBG_PRINT("UPLOAD [%u]\n", len);

    Xcp_Upload(len);
}
#endif // XCP_ENABLE_UPLOAD


#if XCP_ENABLE_SHORT_UPLOAD == XCP_ON
static void Xcp_ShortUpload_Res(Xcp_PDUType const * const pdu)
{
    uint8_t len = pdu->data[1];

// TODO: RangeCheck / Blockmode!!!
    DBG_PRINT("SHORT-UPLOAD [%u]\n", len);
    Xcp_State.mta.ext = pdu->data[3];
    Xcp_State.mta.address = Xcp_GetDWord(pdu, 4);
    Xcp_Upload(len);
}
#endif // XCP_ENABLE_SHORT_UPLOAD


#if XCP_ENABLE_SET_MTA == XCP_ON
static void Xcp_SetMta_Res(Xcp_PDUType const * const pdu)
{
#if 0
0 BYTE Command Code = 0xF6
1 WORD Reserved
3 BYTE Address extension
4 DWORD Address
#endif // 0
    Xcp_State.mta.ext = pdu->data[3];
    Xcp_State.mta.address = Xcp_GetDWord(pdu, 4);

    DBG_PRINT("SET_MTA %x::%x\n", Xcp_State.mta.ext, Xcp_State.mta.address);

    Xcp_Send8(1, 0xff, 0, 0, 0, 0, 0, 0, 0);
}
#endif // XCP_ENABLE_SET_MTA


#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON
static void Xcp_BuildChecksum_Res(Xcp_PDUType const * const pdu)
{
    uint32_t blockSize = Xcp_GetDWord(pdu, 4);

    DBG_PRINT("BUILD_CHECKSUM [%lu]\n", blockSize);

#if 0
0 BYTE Command Code = 0xF3
1 BYTE reserved
2 WORD reserved
4 DWORD Block size [AG]
#endif // 0

}
#endif // XCP_ENABLE_BUILD_CHECKSUM


#if 0
    XCP_VALIDATE_ADRESS
        Callout der die gültigkeit eines Speicherzugriffs überprüft [addr;length]
#endif


#if XCP_ENABLE_TRANSPORT_LAYER_CMD
static void Xcp_TransportLayerCmd_Res(Xcp_PDUType const * const pdu)
{
    XcpTl_TransportLayerCmd_Res(pdu);
}
#endif // XCP_ENABLE_TRANSPORT_LAYER_CMD


#if XCP_ENABLE_USER_CMD
static void Xcp_UserCmd_Res(Xcp_PDUType const * const pdu)
{

}
#endif // XCP_ENABLE_USER_CMD


/*
**  Helpers.
*/
INLINE uint16_t Xcp_GetWord(Xcp_PDUType const * const pdu, uint8_t offs)
{
  return (*(pdu->data + offs))        |
    ((*(pdu->data + 1 + offs)) << 8);
}


INLINE uint32_t Xcp_GetDWord(Xcp_PDUType const * const pdu, uint8_t offs)
{
  return (*(pdu->data + offs))        |
    ((*(pdu->data + 1 + offs)) << 8)  |
    ((*(pdu->data + 2 + offs)) << 16) |
    ((*(pdu->data + 3 + offs)) << 24);
}


INLINE void Xcp_SetWord(Xcp_PDUType const * const pdu, uint8_t offs, uint16_t value)
{
    (*(pdu->data + offs)) = value & 0xff;
    (*(pdu->data + 1 + offs)) = (value & 0xff) >> 8;
}

INLINE void Xcp_SetDWord(Xcp_PDUType const * const pdu, uint8_t offs, uint32_t value)
{
    (*(pdu->data + offs)) = value & 0xff;
    (*(pdu->data + 1 + offs)) = (value & 0xff00) >> 8;
    (*(pdu->data + 2 + offs)) = (value & 0xff0000) >> 16;
    (*(pdu->data + 3 + offs)) = (value & 0xff000000) >> 24;
}

void Xcp_MemCopy(void * dst, void * src, uint16_t len)
{
    uint8_t * pd = (uint8_t *)dst;
    uint8_t * ps = (uint8_t *)src;

//    ASSERT(dst != (void *)NULL);
//    ASSERT(pd >= ps + len || ps >= pd + len);
//    ASSERT(len != (uint16_t)0);

    while (len--) {
        *pd++ = *ps++;
    }

}


void Xcp_MemSet(void * dest, uint8_t fill_char, uint16_t len)
{
    uint8_t * p = (uint8_t *)dest;

//    ASSERT(dest != (void *)NULL);

    while (len--) {
        *p++ = fill_char;
    }
}
