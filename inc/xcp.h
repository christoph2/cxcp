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

//#include <stdlib.h>

/* check for C99-Compiler */
#if defined(__STDC_VERSION__)
    #if __STDC_VERSION__ >= 199901L
        #define C99_COMPILER
    #endif
#endif

/* check for C1x-Compiler */
#if defined(__STDC_VERSION__)
    #if __STDC_VERSION__>= 201112L
        #define C11_COMPILER
    #endif
#endif

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

#if XCP_DAQ_MAX_EVENT_CHANNEL < 1
    #error XCP_DAQ_MAX_EVENT_CHANNEL must be at least 1
#endif // XCP_DAQ_MAX_EVENT_CHANNEL


#if defined(__CSMC__)  || !defined(C99_COMPILER) || !defined(C11_COMPILER)
typedef unsigned char       bool;

typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
typedef signed long         int32_t;
typedef unsigned long       uint32_t;

typedef signed long long    int64_t;
typedef unsigned long long  uint64_t;
#else


#include <stdbool.h>
#include <stdint.h>

#endif // defined

#define UINT8(x)    ((uint8_t)(x))
#define INT8(x)     ((int8_t)(x))

#define UINT16(x)   ((uint16_t)(x))
#define INT16(x)    ((int16_t)(x))

#define UINT32(x)   ((uint32_t)(x))
#define INT32(x)    ((int32_t)(x))

#define UINT64(x)   ((uint64_t)(x))
#define INT64(x)    ((int64_t)(x))

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


#if defined(_MSC_VER)
#define INLINE __inline
#define DBG_PRINT1(a)               printf(a)
#define DBG_PRINT2(a, b)            printf(a, b)
#define DBG_PRINT3(a, b, c)         printf(a, b, c)
#define DBG_PRINT4(a, b, c, d)      printf(a, b, c, d)
#define DBG_PRINT5(a, b, c, d, e)   printf(a, b, c, d, e)
#elif defined(__CSMC__) || defined(__IAR_SYSTEMS_ICC__)
#define INLINE
#define DBG_PRINT1(a)
#define DBG_PRINT2(a, b)
#define DBG_PRINT3(a, b, c)
#define DBG_PRINT4(a, b, c, d)
#define DBG_PRINT5(a, b, c, d, e)
#else
#define INLINE inline
#define DBG_PRINT(...)
#endif // defined(_MSC_VER)


/*
** Global Types.
*/

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

typedef struct tagXcp_DaqMtaType {
#if XCP_DAQ_ADDR_EXT_SUPPORTED == XCP_ON
    uint8_t ext;
#endif // XCP_DAQ_ADDR_EXT_SUPPORTED
    uint32_t address;
} Xcp_DaqMtaType;


typedef enum tagXcp_DaqProcessorStateType {
    XCP_DAQ_STATE_UNINIT            = 0,
    XCP_DAQ_STATE_CONFIG_INVALID    = 1,
    XCP_DAQ_STATE_CONFIG_VALID      = 2,
    XCP_DAQ_STATE_STOPPED           = 3,
    XCP_DAQ_STATE_RUNNING           = 4
} Xcp_DaqProcessorStateType;

typedef struct tagXcp_DaqProcessorType {
    bool running;   // TODO: StateEnum; i.e. DAQ_CONFIG_{INVALID, VALID}
    Xcp_DaqProcessorStateType state;
} Xcp_DaqProcessorType;


typedef struct tagXcp_DaqPointerType {
    uint16_t daqList;
    uint8_t odt;
    uint8_t odtEntry;
    uint16_t daqEntityNumber;
} Xcp_DaqPointerType;


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


typedef struct tagXcp_ODTEntryType {
    Xcp_DaqMtaType mta;
#if XCP_DAQ_BIT_OFFSET_SUPPORTED == XCP_ON
    uint8_t bitOffset;
#endif // XCP_DAQ_BIT_OFFSET_SUPPORTED
    uint32_t length;
} Xcp_ODTEntryType;

typedef struct tagXcp_ODTType {
    uint8_t numOdtEntries;
    uint16_t firstOdtEntry;
} Xcp_ODTType;

typedef enum tagXcp_DaqDirectionType {
    XCP_DIRECTION_NONE,
    XCP_DIRECTION_DAQ,
    XCP_DIRECTION_STIM,
    XCP_DIRECTION_DAQ_STIM
} Xcp_DaqDirectionType;


typedef struct tagXcp_DaqListType {
    uint8_t numOdts;
    uint16_t firstOdt;
    uint8_t mode;
#if XCP_DAQ_PRESCALER_SUPPORTED == XCP_ON
    uint8_t prescaler;
    uint8_t  counter;
#endif // XCP_DAQ_PRESCALER_SUPPORTED
} Xcp_DaqListType;


typedef enum tagXcp_DaqEntityKindType {
    XCP_ENTITY_UNUSED,
    XCP_ENTITY_DAQ_LIST,
    XCP_ENTITY_ODT,
    XCP_ENTITY_ODT_ENTRY
} Xcp_DaqEntityKindType;


typedef struct tagXcp_DaqEntityType {
    /*Xcp_DaqEntityKindType*/uint8_t kind;
    union {
        Xcp_ODTEntryType odtEntry;
        Xcp_ODTType odt;
        Xcp_DaqListType daqList;
    } entity;
} Xcp_DaqEntityType;


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
void Xcp_InitDaq(void);
Xcp_ReturnType Xcp_FreeDaq(void);
Xcp_ReturnType Xcp_AllocDaq(uint16_t daqCount);
Xcp_ReturnType Xcp_AllocOdt(uint16_t daqListNumber, uint8_t odtCount);
Xcp_ReturnType Xcp_AllocOdtEntry(uint16_t daqListNumber, uint8_t odtNumber, uint8_t odtEntriesCount);
Xcp_DaqListType * Daq_GetList(uint8_t daqListNumber);
Xcp_ODTEntryType * Daq_GetOdtEntry(uint8_t daqListNumber, uint8_t odtNumber, uint8_t odtEntryNumber);
bool Xcp_DaqConfigurationValid(void);
bool Xcp_DaqListValid(uint8_t daqListNumber);
void Xcp_DaqMainfunction(void);
void Xcp_DaqAddEventChannel(uint16_t daqListNumber, uint16_t eventChannelNumber);
void Xcp_TriggerDaqEvent(uint8_t eventNumber);

#if !defined(LOBYTE)
#define LOBYTE(w)   ((uint8_t)((w) & (uint8_t)0xff))
#endif

#if !defined(HIBYTE)
#define HIBYTE(w)   ((uint8_t)(((w)  & (uint16_t)0xff00) >> 8))
#endif

#if !defined(LOWORD)
#define LOWORD(w)   ((uint16_t)((w) & (uint16_t)0xffff))
#endif

#if !defined(HIWORD)
#define HIWORD(w)   ((uint16_t)(((w)  & (uint32_t)0xffff0000) >> 16))
#endif

#if !defined(MAX)
#define MAX(l, r) ((l) > (r) ? (l) : (r))
#endif

#if !defined(MIN)
#define MIN(l, r) ((l) < (r) ? (l) : (r))
#endif

#if !defined(TRUE)
#define TRUE    (1)
#endif

#if !defined(FALSE)
#define FALSE   (0)
#endif

#if !defined(NULL)
#if defined(__cplusplus)
#define NULL (0)
#else
#define NULL    ((void*)0)
#endif
#endif


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

/* Locals!!! */
void Xcp_MemCopy(void * dst, void * src, uint32_t len);
void Xcp_MemSet(void * dest, uint8_t fill_char, uint32_t len);

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
int XcpTl_FrameAvailable(long sec, long usec);
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


#endif /* __CXCP_H */
