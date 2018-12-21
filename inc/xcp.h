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

#if !defined(__CXCP_H)
#define __CXCP_H

#include "xcp_types.h"
#include "xcp_macros.h"

#define XCP_PROTOCOL_VERSION_MAJOR       (1)
#define XCP_PROTOCOL_VERSION_RELEASE     (0)

#define XCP_TRANSPORT_LAYER_VERSION_MAJOR       (1)
#define XCP_TRANSPORT_LAYER_VERSION_RELEASE     (1)

#define XCP_DEBUG_BUILD         (1)
#define XCP_RELEASE_BUILD       (2)

#define XCP_ON                  (1)
#define XCP_OFF                 (0)

#define XCP_ON_CAN              (0)
#define XCP_ON_CANFD            (1)
#define XCP_ON_FLEXRAY          (2)
#define XCP_ON_USB              (3)
#define XCP_ON_ETHERNET         (4)
#define XCP_ON_SXI              (5)

#include "xcp_config.h"

#if XCP_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
extern "C"
{
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */


#if XCP_DAQ_MAX_EVENT_CHANNEL < 1
    #error XCP_DAQ_MAX_EVENT_CHANNEL must be at least 1
#endif // XCP_DAQ_MAX_EVENT_CHANNEL


/*
**  Available Resources.
*/
#define XCP_RESOURCE_PGM        ((uint8_t)16)
#define XCP_RESOURCE_STIM       ((uint8_t)8)
#define XCP_RESOURCE_DAQ        ((uint8_t)4)
#define XCP_RESOURCE_CAL_PAG    ((uint8_t)1)


#define XCP_BYTE_ORDER_INTEL    ((uint8_t)0)
#define XCP_BYTE_ORDER_MOTOROLA ((uint8_t)1)

#define XCP_SLAVE_BLOCK_MODE    ((uint8_t)0x40)

#define XCP_ADDRESS_GRANULARITY_0   ((uint8_t)2)
#define XCP_ADDRESS_GRANULARITY_1   ((uint8_t)4)

#define XCP_ADDRESS_GRANULARITY_BYTE    ((uint8_t)0)
#define XCP_ADDRESS_GRANULARITY_WORD    (XCP_ADDRESS_GRANULARITY_0)
#define XCP_ADDRESS_GRANULARITY_DWORD   (XCP_ADDRESS_GRANULARITY_1)

#define XCP_OPTIONAL_COMM_MODE          ((uint8_t)0x80)

#define XCP_MASTER_BLOCK_MODE           ((uint8_t)1)
#define XCP_INTERLEAVED_MODE            ((uint8_t)2)

/* DAQ List Modes. */
#define XCP_DAQ_LIST_MODE_ALTERNATING   ((uint8_t)0x01)
#define XCP_DAQ_LIST_MODE_DIRECTION     ((uint8_t)0x02)
#define XCP_DAQ_LIST_MODE_TIMESTAMP     ((uint8_t)0x10)
#define XCP_DAQ_LIST_MODE_PID_OFF       ((uint8_t)0x20)
#define XCP_DAQ_LIST_MODE_SELECTED      ((uint8_t)0x40)
#define XCP_DAQ_LIST_MODE_STARTED       ((uint8_t)0x80)

/* DAQ Properties */
#define XCP_DAQ_PROP_OVERLOAD_EVENT          ((uint8_t)0x80)
#define XCP_DAQ_PROP_OVERLOAD_MSB            ((uint8_t)0x40)
#define XCP_DAQ_PROP_PID_OFF_SUPPORTED       ((uint8_t)0x20)
#define XCP_DAQ_PROP_TIMESTAMP_SUPPORTED     ((uint8_t)0x10)
#define XCP_DAQ_PROP_BIT_STIM_SUPPORTED      ((uint8_t)0x08)
#define XCP_DAQ_PROP_RESUME_SUPPORTED        ((uint8_t)0x04)
#define XCP_DAQ_PROP_PRESCALER_SUPPORTED     ((uint8_t)0x02)
#define XCP_DAQ_PROP_DAQ_CONFIG_TYPE         ((uint8_t)0x01)


/*
** Global Types.
*/

typedef XCP_DAQ_LIST_TYPE XcpDaq_ListIntegerType;
typedef XCP_DAQ_ODT_TYPE XcpDaq_ODTIntegerType;
typedef XCP_DAQ_ODT_ENTRY_TYPE XcpDaq_ODTEntryIntegerType;

typedef enum tagXcp_CommandType {
//
// STD
//
    //
    // Mandatory Commands.
    //
    XCP_CONNECT                 = UINT8(0xFF),
    XCP_DISCONNECT              = UINT8(0xFE),
    XCP_GET_STATUS              = UINT8(0xFD),
    XCP_SYNCH                   = UINT8(0xFC),
    //
    // Optional Commands.
    //
    XCP_GET_COMM_MODE_INFO      = UINT8(0xFB),
    XCP_GET_ID                  = UINT8(0xFA),
    XCP_SET_REQUEST             = UINT8(0xF9),
    XCP_GET_SEED                = UINT8(0xF8),
    XCP_UNLOCK                  = UINT8(0xF7),
    XCP_SET_MTA                 = UINT8(0xF6),
    XCP_UPLOAD                  = UINT8(0xF5),
    XCP_SHORT_UPLOAD            = UINT8(0xF4),
    XCP_BUILD_CHECKSUM          = UINT8(0xF3),

    XCP_TRANSPORT_LAYER_CMD     = UINT8(0xF2),
    XCP_USER_CMD                = UINT8(0xF1),
//
// CAL
//
    //
    // Mandatory Commands.
    //
    XCP_DOWNLOAD                = UINT8(0xF0),
    //
    // Optional Commands.
    //
    XCP_DOWNLOAD_NEXT           = UINT8(0xEF),
    XCP_DOWNLOAD_MAX            = UINT8(0xEE),
    XCP_SHORT_DOWNLOAD          = UINT8(0xED),
    XCP_MODIFY_BITS             = UINT8(0xEC),
//
// PAG
//
    //
    // Mandatory Commands.
    //
    XCP_SET_CAL_PAGE            = UINT8(0xEB),
    XCP_GET_CAL_PAGE            = UINT8(0xEA),
    //
    // Optional Commands.
    //
    XCP_GET_PAG_PROCESSOR_INFO  = UINT8(0xE9),
    XCP_GET_SEGMENT_INFO        = UINT8(0xE8),
    XCP_GET_PAGE_INFO           = UINT8(0xE7),
    XCP_SET_SEGMENT_MODE        = UINT8(0xE6),
    XCP_GET_SEGMENT_MODE        = UINT8(0xE5),
    XCP_COPY_CAL_PAGE           = UINT8(0xE4),
//
// DAQ
//
    //
    // Mandatory Commands.
    //
    XCP_CLEAR_DAQ_LIST          = UINT8(0xE3),
    XCP_SET_DAQ_PTR             = UINT8(0xE2),
    XCP_WRITE_DAQ               = UINT8(0xE1),
    WRITE_DAQ_MULTIPLE          = UINT8(0xC7), // NEW IN 1.1
    XCP_SET_DAQ_LIST_MODE       = UINT8(0xE0),
    XCP_GET_DAQ_LIST_MODE       = UINT8(0xDF),
    XCP_START_STOP_DAQ_LIST     = UINT8(0xDE),
    XCP_START_STOP_SYNCH        = UINT8(0xDD),
    //
    // Optional Commands.
    //
    XCP_GET_DAQ_CLOCK           = UINT8(0xDC),
    XCP_READ_DAQ                = UINT8(0xDB),
    XCP_GET_DAQ_PROCESSOR_INFO  = UINT8(0xDA),
    XCP_GET_DAQ_RESOLUTION_INFO = UINT8(0xD9),
    XCP_GET_DAQ_LIST_INFO       = UINT8(0xD8),
    XCP_GET_DAQ_EVENT_INFO      = UINT8(0xD7),
    XCP_FREE_DAQ                = UINT8(0xD6),
    XCP_ALLOC_DAQ               = UINT8(0xD5),
    XCP_ALLOC_ODT               = UINT8(0xD4),
    XCP_ALLOC_ODT_ENTRY         = UINT8(0xD3),
//
// PGM
//
    //
    // Mandatory Commands.
    //
    XCP_PROGRAM_START           = UINT8(0xD2),
    XCP_PROGRAM_CLEAR           = UINT8(0xD1),
    XCP_PROGRAM                 = UINT8(0xD0),
    XCP_PROGRAM_RESET           = UINT8(0xCF),
    //
    // Optional Commands.
    //
    XCP_GET_PGM_PROCESSOR_INFO  = UINT8(0xCE),
    XCP_GET_SECTOR_INFO         = UINT8(0xCD),
    XCP_PROGRAM_PREPARE         = UINT8(0xCC),
    XCP_PROGRAM_FORMAT          = UINT8(0xCB),
    XCP_PROGRAM_NEXT            = UINT8(0xCA),
    XCP_PROGRAM_MAX             = UINT8(0xC9),
    XCP_PROGRAM_VERIFY          = UINT8(0xC8)

} Xcp_CommandType;


typedef enum tagXcp_ReturnType {
    ERR_CMD_SYNCH           = UINT8(0x00), // Command processor synchronization.                            S0
                                           //
    ERR_CMD_BUSY            = UINT8(0x10), // Command was not executed.                                     S2
    ERR_DAQ_ACTIVE          = UINT8(0x11), // Command rejected because DAQ is running.                      S2
    ERR_PGM_ACTIVE          = UINT8(0x12), // Command rejected because PGM is running.                      S2
                                           //
    ERR_CMD_UNKNOWN         = UINT8(0x20), // Unknown command or not implemented optional command.          S2
    ERR_CMD_SYNTAX          = UINT8(0x21), // Command syntax invalid                                        S2
    ERR_OUT_OF_RANGE        = UINT8(0x22), // Command syntax valid but command parameter(s) out of range.   S2
    ERR_WRITE_PROTECTED     = UINT8(0x23), // The memory location is write protected.                       S2
    ERR_ACCESS_DENIED       = UINT8(0x24), // The memory location is not accessible.                        S2
    ERR_ACCESS_LOCKED       = UINT8(0x25), // Access denied, Seed & Key is required                         S2
    ERR_PAGE_NOT_VALID      = UINT8(0x26), // Selected page not available                                   S2
    ERR_MODE_NOT_VALID      = UINT8(0x27), // Selected page mode not available                              S2
    ERR_SEGMENT_NOT_VALID   = UINT8(0x28), // Selected segment not valid                                    S2
    ERR_SEQUENCE            = UINT8(0x29), // Sequence error                                                S2
    ERR_DAQ_CONFIG          = UINT8(0x2A), // DAQ configuration not valid                                   S2
                                           //
    ERR_MEMORY_OVERFLOW     = UINT8(0x30), // Memory overflow error                                         S2
    ERR_GENERIC             = UINT8(0x31), // Generic error.                                                S2
    ERR_VERIFY              = UINT8(0x32), // The slave internal program verify routine detects an error.   S3

    // NEW IN 1.1
    ERR_RESOURCE_TEMPORARY_NOT_ACCESSIBLE = UINT8(0x33),    // Access to the requested resource is temporary not possible.   S3

    // Internal Success Code - not related to XCP spec.
    ERR_SUCCESS             = UINT8(0xff)
} Xcp_ReturnType;


typedef struct tagXcp_MtaType {
    uint8_t ext;
    uint32_t address;
} Xcp_MtaType;

typedef struct tagXcpDaq_MtaType {
#if XCP_DAQ_ADDR_EXT_SUPPORTED == XCP_ON
    uint8_t ext;
#endif // XCP_DAQ_ADDR_EXT_SUPPORTED
    uint32_t address;
} XcpDaq_MtaType;


typedef enum tagXcpDaq_ProcessorStateType {
    XCP_DAQ_STATE_UNINIT            = 0,
    XCP_DAQ_STATE_CONFIG_INVALID    = 1,
    XCP_DAQ_STATE_CONFIG_VALID      = 2,
    XCP_DAQ_STATE_STOPPED           = 3,
    XCP_DAQ_STATE_RUNNING           = 4
} XcpDaq_ProcessorStateType;

typedef struct tagXcpDaq_ProcessorType {
    XcpDaq_ProcessorStateType state;
} XcpDaq_ProcessorType;


typedef struct tagXcpDaq_PointerType {
    XcpDaq_ListIntegerType daqList;
    XcpDaq_ODTIntegerType odt;
    XcpDaq_ODTEntryIntegerType odtEntry;
} XcpDaq_PointerType;


typedef enum tagXcp_DTOType {
    EVENT_MESSAGE           = 254,
    COMMAND_RETURN_MESSAGE  = 255
} Xcp_DTOType;

typedef enum tagXcp_ConnectionStateType {
    XCP_DISCONNECTED = 0,
    XCP_CONNECTED = 1
} Xcp_ConnectionStateType;


typedef enum tagXcp_SlaveAccessType {
    XCP_ACC_PGM = 0x40,
    XCP_ACC_DAQ = 0x02,
    XCP_ACC_CAL = 0x01
} Xcp_SlaveAccessType;


typedef struct tagXcp_PDUType {
    uint16_t len;
    uint8_t * data;
} Xcp_PDUType;


typedef struct tagXcp_StationIDType {
    uint16_t len;
    uint8_t const * name;
} Xcp_StationIDType;


typedef struct tagXcpDaq_ODTEntryType {
    XcpDaq_MtaType mta;
#if XCP_DAQ_BIT_OFFSET_SUPPORTED == XCP_ON
    uint8_t bitOffset;
#endif // XCP_DAQ_BIT_OFFSET_SUPPORTED
    uint32_t length;
} XcpDaq_ODTEntryType;

typedef struct tagXcpDaq_ODTType {
    XcpDaq_ODTEntryIntegerType numOdtEntries;
    uint16_t firstOdtEntry;
} XcpDaq_ODTType;

typedef enum tagXcpDaq_DirectionType {
    XCP_DIRECTION_NONE,
    XCP_DIRECTION_DAQ,
    XCP_DIRECTION_STIM,
    XCP_DIRECTION_DAQ_STIM
} XcpDaq_DirectionType;


typedef struct tagXcpDaq_ListType {
    XcpDaq_ODTIntegerType numOdts;
    uint16_t firstOdt;
    uint8_t mode;
#if XCP_DAQ_PRESCALER_SUPPORTED == XCP_ON
    uint8_t prescaler;
    uint8_t  counter;
#endif // XCP_DAQ_PRESCALER_SUPPORTED
} XcpDaq_ListType;


typedef enum tagXcpDaq_EntityKindType {
    XCP_ENTITY_UNUSED,
    XCP_ENTITY_DAQ_LIST,
    XCP_ENTITY_ODT,
    XCP_ENTITY_ODT_ENTRY
} XcpDaq_EntityKindType;


typedef struct tagXcpDaq_EntityType {
    /*Xcp_DaqEntityKindType*/uint8_t kind;
    union {
        XcpDaq_ODTEntryType odtEntry;
        XcpDaq_ODTType odt;
        XcpDaq_ListType daqList;
    } entity;
} XcpDaq_EntityType;


typedef struct tagXcpDaq_EventType {
    uint8_t const * name;
/*
1  BYTE  DAQ_EVENT_PROPERTIES           Specific properties for this event channel
2  BYTE  MAX_DAQ_LIST [0,1,2,..255]     maximum number of DAQ lists in this event channel
3  BYTE  EVENT_CHANNEL_NAME_LENGTH      in bytes 0 – If not available
4  BYTE  EVENT_CHANNEL_TIME_CYCLE       0 – Not cyclic
5  BYTE  EVENT_CHANNEL_TIME_UNIT        don’t care if Event channel time cycle = 0
6  BYTE  EVENT_CHANNEL_PRIORITY         (FF highest)
*/
} XcpDaq_EventType;


typedef void(*Xcp_SendCalloutType)(Xcp_PDUType const * pdu);
typedef void (*Xcp_ServerCommandType)(Xcp_PDUType const * const pdu);

/*
** Global User Functions.
*/
void Xcp_Init(void);
void Xcp_MainFunction(void);


/*
** Global Helper Functions.
*/
void Xcp_DispatchCommand(Xcp_PDUType const * const pdu);

Xcp_ConnectionStateType Xcp_GetConnectionState(void);
void Xcp_SetSendCallout(Xcp_SendCalloutType callout);
void Xcp_DumpMessageObject(Xcp_PDUType const * pdu);
Xcp_MtaType Xcp_GetNonPagedAddress(void const * const ptr);
void Xcp_SetMta(Xcp_MtaType mta);

/*
** DAQ Implementation Functions.
*/
void XcpDaq_Init(void);
Xcp_ReturnType XcpDaq_Free(void);
Xcp_ReturnType XcpDaq_Alloc(XcpDaq_ListIntegerType daqCount);
Xcp_ReturnType XcpDaq_AllocOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtCount);
Xcp_ReturnType XcpDaq_AllocOdtEntry(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntriesCount);
XcpDaq_ListType * XcpDaq_GetList(XcpDaq_ListIntegerType daqListNumber);
XcpDaq_ODTEntryType * XcpDaq_GetOdtEntry(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber);
bool XcpDaq_ValidateConfiguration(void);
bool XcpDaq_ValidateList(XcpDaq_ListIntegerType daqListNumber);
bool XcpDaq_ValidateOdtEntry(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntry);
void XcpDaq_Mainfunction(void);
void XcpDaq_AddEventChannel(XcpDaq_ListIntegerType daqListNumber, uint16_t eventChannelNumber);
void XcpDaq_TriggerEvent(uint8_t eventChannelNumber);
void XcpDaq_GetProperties(uint8_t * properties);
XcpDaq_ListIntegerType XcpDaq_GetListCount(void);


#define XCP_CHECKSUM_METHOD_XCP_ADD_11      (1)
#define XCP_CHECKSUM_METHOD_XCP_ADD_12      (2)
#define XCP_CHECKSUM_METHOD_XCP_ADD_14      (3)
#define XCP_CHECKSUM_METHOD_XCP_ADD_22      (4)
#define XCP_CHECKSUM_METHOD_XCP_ADD_24      (5)
#define XCP_CHECKSUM_METHOD_XCP_ADD_44      (6)
#define XCP_CHECKSUM_METHOD_XCP_CRC_16      (7)
#define XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT (8)
#define XCP_CHECKSUM_METHOD_XCP_CRC_32      (9)

#define XCP_DAQ_TIMESTAMP_UNIT_1NS          (0)
#define XCP_DAQ_TIMESTAMP_UNIT_10NS         (1)
#define XCP_DAQ_TIMESTAMP_UNIT_100NS        (2)
#define XCP_DAQ_TIMESTAMP_UNIT_1US          (3)
#define XCP_DAQ_TIMESTAMP_UNIT_10US         (4)
#define XCP_DAQ_TIMESTAMP_UNIT_100US        (5)
#define XCP_DAQ_TIMESTAMP_UNIT_1MS          (6)
#define XCP_DAQ_TIMESTAMP_UNIT_10MS         (7)
#define XCP_DAQ_TIMESTAMP_UNIT_100MS        (8)
#define XCP_DAQ_TIMESTAMP_UNIT_1S           (9)
#define XCP_DAQ_TIMESTAMP_UNIT_1PS          (10)
#define XCP_DAQ_TIMESTAMP_UNIT_10PS         (11)
#define XCP_DAQ_TIMESTAMP_UNIT_100PS        (12)

#define XCP_DAQ_TIMESTAMP_SIZE_1            (1)
#define XCP_DAQ_TIMESTAMP_SIZE_2            (2)
#define XCP_DAQ_TIMESTAMP_SIZE_4            (4)

/*
**
*/
void Xcp_SendPdu(void);
uint8_t * Xcp_GetOutPduPtr(void);
void Xcp_SetPduOutLen(uint16_t len);
void Xcp_Send8(uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7);
void XcpTl_Send(uint8_t const * buf, uint16_t len);

/*
**  Helpers.
*/
void Xcp_CopyMemory(Xcp_MtaType dst, Xcp_MtaType src, uint32_t len);

uint8_t Xcp_GetByte(Xcp_PDUType const * const value, uint8_t offs);
uint16_t Xcp_GetWord(Xcp_PDUType const * const value, uint8_t offs);
uint32_t Xcp_GetDWord(Xcp_PDUType const * const value, uint8_t offs);

void Xcp_SetByte(Xcp_PDUType const * const pdu, uint8_t offs, uint8_t value);
void Xcp_SetWord(Xcp_PDUType const * const pdu, uint8_t offs, uint16_t value);
void Xcp_SetDWord(Xcp_PDUType const * const pdu, uint8_t offs, uint32_t value);

/*
**  Transport Layer Stuff.
*/
void XcpTl_Init(void);
void XcpTl_DeInit(void);
int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec);
void XcpTl_RxHandler(void);

void XcpTl_Task(void);

void XcpTl_SaveConnection(void);
void XcpTl_ReleaseConnection(void);
bool XcpTl_VerifyConnection(void);

void XcpTl_FeedReceiver(uint8_t octet);

void XcpTl_TransportLayerCmd_Res(Xcp_PDUType const * const pdu);

/*
**  Customization Stuff.
*/
bool Xcp_HookFunction_GetId(uint8_t idType);

/*
**  Hardware dependent stuff.
*/
void XcpHw_Init(void);
uint32_t XcpHw_GetTimerCounter(void);

extern Xcp_PDUType Xcp_PduIn;
extern Xcp_PDUType Xcp_PduOut;



#if (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT)
typedef uint16_t Xcp_CrcType;
#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_32
typedef uint32_t Xcp_CrcType;
#endif // XCP_CHECKSUM_METHOD

Xcp_CrcType Xcp_CalculateCRC(uint8_t const * message, uint32_t length, Xcp_CrcType startValue, bool isFirstCall);


#if XCP_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
}
#endif  /* __cplusplus */
#endif /* XCP_EXTERN_C_GUARDS */

#endif /* __CXCP_H */
