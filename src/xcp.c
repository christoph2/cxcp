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
#include <stdio.h>
#include <stdlib.h>

static Xcp_ConnectionStateType Xcp_ConnectionState = XCP_DISCONNECTED;
void Xcp_WriteMemory(void * dest, void * src, uint16_t count);
void Xcp_ReadMemory(void * dest, void * src, uint16_t count);

/*
Memory transfer addresses:
---
    - MTA0 is used by the commands DNLOAD, UPLOAD, DNLOAD_6, SELECT_CAL_PAGE, CLEAR_MEMORY, PROGRAM and PROGRAM_6.
    - MTA1 is used by the MOVE command
*/
static uint32_t Xcp_Mta0, Xcp_Mta1;
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


void Xcp_Init(void)
{
    printf("Xcp_Init()\n");
    Xcp_ConnectionState = XCP_DISCONNECTED;
    Xcp_Mta0 = Xcp_Mta1 = 0x000000000L;
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

#define Xcp_AcknowledgedCRM(ctr, b0, b1, b2, b3, b4)    Xcp_SetDTOValues(&cmoOut, COMMAND_RETURN_MESSAGE, ACKNOWLEDGE, ctr, b0, b1, b2, b3, b4)
#define Xcp_AcknowledgedUploadCRM(ctr)                  Xcp_SetUploadDTO(&cmoOut, COMMAND_RETURN_MESSAGE, ACKNOWLEDGE, ctr)

/**
 * Entry point, needs to be "wired" to CAN-Rx interrupt.
 *
 * @param cmoIn
 */
void Xcp_DispatchCommand(Xcp_MessageObjectType const * cmoIn)
{
    Xcp_MessageObjectType cmoOut = {0};
    uint16_t stationAddress;

    printf("Req: ");
    Xcp_DumpMessageObject(cmoIn);

    if (Xcp_ConnectionState == XCP_CONNECTED) {
        switch (XCP_COMMAND) {
        case GET_XCP_VERSION:
                Xcp_AcknowledgedCRM(COUNTER_IN, XCP_VERSION_MAJOR, XCP_VERSION_RELEASE,0, 0, 0);
                Xcp_SendCmo(&cmoOut);
                break;
            case EXCHANGE_ID:

                Xcp_AcknowledgedCRM(COUNTER_IN,
                                    Xcp_StationID.len,
                                    0 ,                 /* data type qualifier of slave device ID (optional and implementation specific). */
                                    PGM | DAQ | CAL,    /* TODO: config. */
                                    0,                  /* No protection. */
                                    0
                );
                Xcp_SendCmo(&cmoOut);
                Xcp_Mta0 = (uint32_t)&Xcp_StationID.name;
                break;
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
            case DNLOAD:
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

void Xcp_SetSendCallout(Xcp_SendCalloutType * callout)
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

uint32_t Xcp_GetMta0(void)
{
    return Xcp_Mta0;
}

uint32_t Xcp_GetMta1(void)
{
    return Xcp_Mta1;
}

void Xcp_DumpMessageObject(Xcp_MessageObjectType const * cmo)
{
    printf("%08X  %u  [%02X %02X %02X %02X %02X %02X %02X %02X]\n", cmo->canID, cmo->dlc,
           cmo->data[0], cmo->data[1], cmo->data[2], cmo->data[3], cmo->data[4], cmo->data[5], cmo->data[6], cmo->data[7]
    );
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

