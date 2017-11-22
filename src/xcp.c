/*
 * pySART - Simplified AUTOSAR-Toolkit for Python.
 *
 * (C) 2007-2017 by Christoph Schueler <github.com/Christoph2,
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

#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
** Private Options.
*/
#define XCP_ENABLE_STD_COMMANDS                     XCP_ON
#define XCP_ENABLE_INTERLEAVED_MODE                 XCP_OFF


typedef struct tagXcp_StateType {
    bool connected;
    bool busy;
    bool daqRunning;
    bool programming;
} Xcp_StateType;


static Xcp_ConnectionStateType Xcp_ConnectionState = XCP_DISCONNECTED;
void Xcp_WriteMemory(void * dest, void * src, uint16_t count);
void Xcp_ReadMemory(void * dest, void * src, uint16_t count);

static uint32_t Xcp_Mta;
static uint8_t Xcp_MtaExtension;

static Xcp_SendCalloutType Xcp_SendCallout = NULL;
static const Xcp_StationIDType Xcp_StationID = { sizeof(XCP_STATION_ID), XCP_STATION_ID };

#if defined(XCP_SIMULATOR)
static uint8_t Xcp_SimulatedMemory[XCP_SIMULATED_MEMORY_SIZE];
#endif

#define XCP_COMMAND     (cmoIn->data[0])

#define DATA_IN(idx)    (cmoIn->data[(idx)])
#define DATA_OUT(idx)   (cmoOut->data[(idx)])

#define COUNTER_IN      (cmoIn->data[1])
#define COUNTER_OUT     (cmoOut->data[2])


/*
** Local Function Prototypes.
*/
static void Xcp_CommandNotImplemented_Res(Xcp_XPDUType const * const pdu);
static void Xcp_Connect_Res(Xcp_XPDUType const * const pdu);
static void Xcp_Disconnect_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetStatus_Res(Xcp_XPDUType const * const pdu);
static void Xcp_Synch_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetCommModeInfo_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetId_Res(Xcp_XPDUType const * const pdu);
static void Xcp_SetRequest_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetSeed_Res(Xcp_XPDUType const * const pdu);
static void Xcp_Unlock_Res(Xcp_XPDUType const * const pdu);
static void Xcp_SetMta_Res(Xcp_XPDUType const * const pdu);
static void Xcp_Upload_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ShortUpload_Res(Xcp_XPDUType const * const pdu);
static void Xcp_BuildChecksum_Res(Xcp_XPDUType const * const pdu);
static void Xcp_TransportLayerCmd_Res(Xcp_XPDUType const * const pdu);
static void Xcp_UserCmd_Res(Xcp_XPDUType const * const pdu);
static void Xcp_Download_Res(Xcp_XPDUType const * const pdu);
static void Xcp_DownloadNext_Res(Xcp_XPDUType const * const pdu);
static void Xcp_DownloadMax_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ShortDownload_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ModifyBits_Res(Xcp_XPDUType const * const pdu);
static void Xcp_SetCalPage_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetCalPage_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetPagProcessorInfo_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetSegmentInfo_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetPageInfo_Res(Xcp_XPDUType const * const pdu);
static void Xcp_SetSegmentMode_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetSegmentMode_Res(Xcp_XPDUType const * const pdu);
static void Xcp_CopyCalPage_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ClearDaqList_Res(Xcp_XPDUType const * const pdu);
static void Xcp_SetDaqPtr_Res(Xcp_XPDUType const * const pdu);
static void Xcp_WriteDaq_Res(Xcp_XPDUType const * const pdu);
static void Xcp_SetDaqListMode_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetDaqListMode_Res(Xcp_XPDUType const * const pdu);
static void Xcp_StartStopDaqList_Res(Xcp_XPDUType const * const pdu);
static void Xcp_StartStopSynch_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetDaqClock_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ReadDaq_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetDaqProcessorInfo_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetDaqResolutionInfo_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetDaqListInfo_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetDaqEventInfo_Res(Xcp_XPDUType const * const pdu);
static void Xcp_FreeDaq_Res(Xcp_XPDUType const * const pdu);
static void Xcp_AllocDaq_Res(Xcp_XPDUType const * const pdu);
static void Xcp_AllocOdt_Res(Xcp_XPDUType const * const pdu);
static void Xcp_AllocOdtEntry_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ProgramStart_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ProgramClear_Res(Xcp_XPDUType const * const pdu);
static void Xcp_Program_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ProgramReset_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetPgmProcessorInfo_Res(Xcp_XPDUType const * const pdu);
static void Xcp_GetSectorInfo_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ProgramPrepare_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ProgramFormat_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ProgramNext_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ProgramMax_Res(Xcp_XPDUType const * const pdu);
static void Xcp_ProgramVerify_Res(Xcp_XPDUType const * const pdu);

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
** Global Functions.
*/

void Xcp_Init(void)
{
    printf("Xcp_Init()\n");
    Xcp_ConnectionState = XCP_DISCONNECTED;
    Xcp_Mta = 0x000000000L;
    Xcp_MtaExtension = 0x00;
#if defined(XCP_SIMULATOR)
    memcpy(&Xcp_SimulatedMemory, &Xcp_StationID.name, Xcp_StationID.len);
#endif
}

void Xcp_SetDTOValues(Xcp_MessageObjectType * cmoOut, uint8_t type, uint8_t returnCode,
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


void Xcp_SetUploadDTO(Xcp_MessageObjectType * cmoOut, uint8_t type, uint8_t returnCode, uint8_t counter)
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

/**
 * Entry point, needs to be "wired" to CAN-Rx interrupt.
 *
 * @param pdu
 */
void Xcp_DispatchCommand(Xcp_XPDUType const * const pdu)
{
    Xcp_MessageObjectType cmoOut = {0};
    uint8_t cmd = pdu->data[0];

    printf("Req: ");
    Xcp_DumpMessageObject(pdu);

    if (Xcp_ConnectionState == XCP_CONNECTED) { // TODO: bool
        Xcp_ServerCommands[cmd](pdu);   // TODO: range check!!!
    } else {    // not connected.
        if (pdu->data[0] == CONNECT) {
            Xcp_Connect_Res(pdu);
        } else {

        }
    }
    fflush(stdout);

#if 0
    if (Xcp_ConnectionState == XCP_CONNECTED) {
        switch (XCP_COMMAND) {
/*
            case GET_XCP_VERSION:
                Xcp_AcknowledgedCRM(COUNTER_IN, XCP_VERSION_MAJOR, XCP_VERSION_RELEASE,0, 0, 0);
                Xcp_SendCmo(&cmoOut);
                break;
            case EXCHANGE_ID:
                Xcp_AcknowledgedCRM(COUNTER_IN,
                                    Xcp_StationID.len,
                                    0 ,                 // data type qualifier of slave device ID (optional and implementation specific).
                                    PGM | DAQ | CAL,    // TODO: config.
                                    0,                  // No protection.
                                    0
                );
                Xcp_SendCmo(&cmoOut);
                Xcp_Mta0 = (uint32_t)&Xcp_StationID.name;
                break;
*/
            case SET_MTA:
                printf("SetMTA\n");
                if (DATA_IN(2) == 0) {
                    Xcp_Mta0 = (DATA_IN(4) << 24) | (DATA_IN(5) << 16) | (DATA_IN(6) << 8) | DATA_IN(7);
                } else if (DATA_IN(2) == 1) {
                    Xcp_Mta1 = (DATA_IN(4) << 24) | (DATA_IN(5) << 16) | (DATA_IN(6) << 8) | DATA_IN(7);
                } else {
                    /* Invalid MTA number.*/
                    break;
                }
                Xcp_MtaExtension = DATA_IN(3);

                Xcp_AcknowledgedCRM(COUNTER_IN, 0, 0,0, 0, 0);
                Xcp_SendCmo(&cmoOut);
                break;
            case DOWNLOAD:
                printf("Download\n");
                Xcp_WriteMemory(&Xcp_Mta0, &DATA_IN(3), DATA_IN(2));
                Xcp_Mta0 += DATA_IN(2);
                Xcp_AcknowledgedCRM(
                    COUNTER_IN,
                    Xcp_MtaExtension,
                    (Xcp_Mta0 & 0xff000000) >> 24,
                    (Xcp_Mta0 & 0x00ff0000) >> 16,
                    (Xcp_Mta0 & 0x0000ff00) >> 8,
                    Xcp_Mta0 & 0xff
                );
                Xcp_SendCmo(&cmoOut);
            case UPLOAD:
                printf("Upload\n");
                //Xcp_ReadMemory(&Xcp_Mta0, &DATA_OUT(3), DATA_IN(2));
                Xcp_Mta0 += DATA_IN(2);
                Xcp_AcknowledgedUploadCRM(COUNTER_IN);
                break;
        }
    } else {
        /*
        ** Handle unconnected commands.
        */
        if (XCP_COMMAND == CONNECT) {
            stationAddress = DATA_IN(2) | (DATA_IN(3) << 8);

            //printf("Connect [%u] [%u]\n", XCP_STATION_ADDRESS, stationAddress);
            if (XCP_STATION_ADDRESS == stationAddress) {
                Xcp_AcknowledgedCRM(COUNTER_IN, 0, 0, 0, 0, 0);
                Xcp_SendCmo(&cmoOut);
                Xcp_ConnectionState = XCP_CONNECTED;
            } else {
                /* "A CONNECT command to another station temporary disconnects the active station " */
                //printf("Disconnecting...\n");
                Xcp_ConnectionState = XCP_DISCONNECTED;
            }
        }
    }
    /*
    // Mandatory Commands.
    DNLOAD              = 0x03,
    UPLOAD              = 0x04,
    GET_DAQ_SIZE        = 0x14,
    SET_DAQ_PTR         = 0x15,
    WRITE_DAQ           = 0x16,
    START_STOP          = 0x06,
    DISCONNECT          = 0x07
    */
#endif
}


void Xcp_SendCmo(Xcp_MessageObjectType const * cmoOut)
{
    /*
    **
    ** Note: A callout is only needed for unit-testing.
    ** TODO: Conditional compilation (testing vs. "real-world").
    **
    */
    if (Xcp_SendCallout) {
        (*Xcp_SendCallout)(cmoOut);
    }
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

uint32_t Xcp_GetMta(void)
{
    return Xcp_Mta;
}

void Xcp_DumpMessageObject(Xcp_XPDUType const * pdu)
{
    //printf("LEN: %d CMD: %02X\n", pdu->len, pdu->data[0]);
    //printf("PTR: %p\n", pdu->data);
#if 0
    printf("%08X  %u  [%02X %02X %02X %02X %02X %02X %02X %02X]\n", cmo->canID, cmo->dlc,
           cmo->data[0], cmo->data[1], cmo->data[2], cmo->data[3], cmo->data[4], cmo->data[5], cmo->data[6], cmo->data[7]
    );
#endif
}

void Xcp_WriteMemory(void * dest, void * src, uint16_t count)
{
#if defined(XCP_SIMULATOR)
    ptrdiff_t  diff;
    printf("Dest: %p -- SimMem: %p\n", dest, &Xcp_SimulatedMemory);
#else
    memcpy(dest, src, count);
#endif
}

/*
** Local Functions.
*/
static void Xcp_CommandNotImplemented_Res(Xcp_XPDUType const * const pdu)
{
    printf("Command not implemented.\n");
}

static void Xcp_Connect_Res(Xcp_XPDUType const * const pdu)
{
    uint8_t resource = 0x00;
    uint8_t commModeBasic = 0x00;

    printf("CONNECT: \n");

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    resource |= XCP_RESOURCE_PGM;
#endif
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    resource |= XCP_RESOURCE_DAQ;
#endif
#if XCP_ENABLE_CAL_COMMANDS == XCP_ON || XCP_ENABLE_PAG_COMMANDS == XCP_ON
    resource |= XCP_RESOURCE_CAL_PAG;
#endif
#if XCP_ENABLE_STIM == XCP_ON
    resource |= XCP_RESOURCE_STIM;
#endif

}

static void Xcp_Disconnect_Res(Xcp_XPDUType const * const pdu)
{
    printf("DISCONNECT: \n");
}

static void Xcp_GetStatus_Res(Xcp_XPDUType const * const pdu)
{

}

static void Xcp_Synch_Res(Xcp_XPDUType const * const pdu)
{

}

#if 0
    XCP_VALIDATE_ADRESS
        Callout der die gültigkeit eines Speicherzugriffs überprüft [addr;length]
#endif

