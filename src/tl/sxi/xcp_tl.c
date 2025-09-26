
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

#include "xcp_config.h"

#if XCP_TRANSPORT_LAYER == XCP_ON_SXI

    #if defined(ARDUINO)
        #include "Arduino.h"
    #else
        #include <stdio.h>
    #endif

    /*!!! START-INCLUDE-SECTION !!!*/
    #include "xcp.h"
    #include "xcp_tl_timeout.h"
    #include "xcp_util.h"
/*!!! END-INCLUDE-SECTION !!!*/

    #define XCP_SXI_MAKEWORD(buf, offs) ((uint16_t)(*(((buf)) + (offs))) | ((uint16_t)(*(((buf)) + (offs) + 1)) << 8))

    #define TIMEOUT_VALUE (100)

typedef enum tagXcpTl_ReceiverStateType {
    XCP_RCV_IDLE,
    XCP_RCV_UNTIL_LENGTH,
    XCP_RCV_REMAINING
} XcpTl_ReceiverStateType;

typedef struct tagXcpTl_ReceiverType {
    uint8_t                 Buffer[XCP_COMM_BUFLEN];
    XcpTl_ReceiverStateType State;
    uint16_t                Index;
    uint16_t                Remaining;
    uint16_t                Dlc;
    uint16_t                Ctr;
    #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
    XcpSxiChecksumType ReceivedChecksum;
    uint8_t            ChecksumBytesRemaining;
    #endif
} XcpTl_ReceiverType;

    #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
typedef enum {
    FRM_WAIT_FOR_SYNC,
    FRM_RECEIVING,
    FRM_WAIT_FOR_ESC_BYTE
} XcpTl_FramingStateType;

static XcpTl_FramingStateType s_FramingState = FRM_WAIT_FOR_SYNC;

    #endif

static XcpTl_ReceiverType XcpTl_Receiver;

static void XcpTl_ResetSM(void);

static void XcpTl_SignalTimeout(void);

static void XcpTl_ProcessOctet(uint8_t octet);

void XcpTl_Init(void) {
    #if defined(ARDUINO)
    Serial.begin(XCP_ON_SXI_BITRATE, XCP_ON_SXI_CONFIG);
    #endif

    XcpTl_ResetSM();
    XcpTl_TimeoutInit(TIMEOUT_VALUE, XcpTl_ResetSM);
}

void XcpTl_DeInit(void) {
}

void XcpTl_MainFunction(void) {
    uint8_t octet = 0;

    #if defined(ARDUINO)
    if (Serial.available()) {
        digitalWrite(LED_BUILTIN, HIGH);
        octet = Serial.read();
        XcpTl_FeedReceiver(octet);

        digitalWrite(LED_BUILTIN, LOW);
    }
    #endif
    XcpTl_TimeoutCheck();
}

/**
 * \brief Initialize, i.e. reset
 *receiver state Machine
 *
 * \param void
 * \return void
 *
 **/
static void XcpTl_ResetSM(void) {
    XcpTl_Receiver.State     = XCP_RCV_IDLE;
    XcpTl_Receiver.Index     = 0u;
    XcpTl_Receiver.Dlc       = 0u;
    XcpTl_Receiver.Ctr       = 0u;
    XcpTl_Receiver.Remaining = 0u;
    #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
    XcpTl_Receiver.ReceivedChecksum       = 0;
    XcpTl_Receiver.ChecksumBytesRemaining = 0;
    #endif
    #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
    s_FramingState = FRM_WAIT_FOR_SYNC;
    #endif
}

void XcpTl_RxHandler(void) {
}

void XcpTl_FeedReceiver(uint8_t octet) {
    #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
    switch (s_FramingState) {
        case FRM_WAIT_FOR_SYNC:
            if (octet == XCP_ON_SXI_SYNC_CHAR) {
                s_FramingState = FRM_RECEIVING;
                XcpTl_ResetSM();
            }
            break;

        case FRM_RECEIVING:
            if (octet == XCP_ON_SXI_ESC_CHAR) {
                s_FramingState = FRM_WAIT_FOR_ESC_BYTE;
            } else if (octet == XCP_ON_SXI_SYNC_CHAR) {
                /* New frame started without proper termination of the old one. */
                XcpTl_ResetSM();
                s_FramingState = FRM_RECEIVING;
            } else {
                XcpTl_ProcessOctet(octet);
            }
            break;

        case FRM_WAIT_FOR_ESC_BYTE:
            if (octet == XCP_ON_SXI_ESC_SYNC_CHAR) {
                XcpTl_ProcessOctet(XCP_ON_SXI_SYNC_CHAR);
            } else if (octet == XCP_ON_SXI_ESC_ESC_CHAR) {
                XcpTl_ProcessOctet(XCP_ON_SXI_ESC_CHAR);
            } else {
                /* Protocol error, invalid escape sequence. Reset. */
                XcpTl_ResetSM();
                s_FramingState = FRM_WAIT_FOR_SYNC;
            }
            s_FramingState = FRM_RECEIVING;
            break;
    }
    #else
    XcpTl_ProcessOctet(octet);
    #endif
}

static void XcpTl_ProcessOctet(uint8_t octet) {
    XCP_TL_ENTER_CRITICAL();

    /* Bounds check to protect Buffer[] */
    if (XcpTl_Receiver.Index >= (uint16_t)sizeof(XcpTl_Receiver.Buffer)) {
        /* Overflow/malformed frame: reset receiver */
        XcpTl_ResetSM();
        XcpTl_TimeoutStop();
        XCP_TL_LEAVE_CRITICAL();
        return;
    }
    XcpTl_Receiver.Buffer[XcpTl_Receiver.Index] = octet;

    if (XcpTl_Receiver.State == XCP_RCV_IDLE) {
        XcpTl_Receiver.State = XCP_RCV_UNTIL_LENGTH;
        XcpTl_TimeoutStart();
    #if (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_BYTE)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x00) {
            XcpTl_Receiver.Dlc       = XcpTl_Receiver.Buffer[0];
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc;
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((XcpTl_Receiver.Dlc & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u; /* fill byte before checksum */
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 1;
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            uint16_t payload_off = 1u;
            uint16_t dlc         = XcpTl_Receiver.Dlc;
            uint16_t fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                uint32_t calc = 0u;
                for (uint16_t i = 0u; i < dlc; ++i)
                    calc += XcpTl_Receiver.Buffer[payload_off + i];
                uint8_t recv = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 1;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            {
                uint32_t calc = 0u;
                uint16_t i    = 0u;
                while (i < dlc) {
                    uint16_t w  = XcpTl_Receiver.Buffer[payload_off + i];
                    uint16_t w2 = (i + 1u < dlc) ? XcpTl_Receiver.Buffer[payload_off + i + 1u] : 0u;
                    calc += (uint16_t)(w | (uint16_t)(w2 << 8));
                    i += 2u;
                }
                uint16_t off  = payload_off + dlc + fill;
                uint16_t recv = (uint16_t)(((uint16_t)XcpTl_Receiver.Buffer[off] << 8) | XcpTl_Receiver.Buffer[off + 1u]);
                if (((uint16_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 1;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_BYTE)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x00) {
            XcpTl_Receiver.Dlc       = XcpTl_Receiver.Buffer[0];
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc + 1; /* CTR */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((XcpTl_Receiver.Dlc & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u; /* fill byte before checksum */
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        if (XcpTl_Receiver.Index == 0x01) {
            XcpTl_Receiver.Ctr = XcpTl_Receiver.Buffer[1];
        }
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            uint16_t payload_off = 2u;
            uint16_t dlc         = XcpTl_Receiver.Dlc;
            uint16_t fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                uint32_t calc = 0u;
                for (uint16_t i = 0u; i < dlc; ++i)
                    calc += XcpTl_Receiver.Buffer[payload_off + i];
                uint8_t recv = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            {
                uint32_t calc = 0u;
                uint16_t i    = 0u;
                while (i < dlc) {
                    uint16_t w  = XcpTl_Receiver.Buffer[payload_off + i];
                    uint16_t w2 = (i + 1u < dlc) ? XcpTl_Receiver.Buffer[payload_off + i + 1u] : 0u;
                    calc += (uint16_t)(w | (uint16_t)(w2 << 8));
                    i += 2u;
                }
                uint16_t off  = payload_off + dlc + fill;
                uint16_t recv = (uint16_t)(((uint16_t)XcpTl_Receiver.Buffer[off] << 8) | XcpTl_Receiver.Buffer[off + 1u]);
                if (((uint16_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_FILL_BYTE)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x00) {
            XcpTl_Receiver.Dlc       = XcpTl_Receiver.Buffer[0];
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc + 1; /* FILL */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((XcpTl_Receiver.Dlc & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u; /* fill byte before checksum */
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            uint16_t payload_off = 2u;
            uint16_t dlc         = XcpTl_Receiver.Dlc;
            uint16_t fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                uint32_t calc = 0u;
                for (uint16_t i = 0u; i < dlc; ++i)
                    calc += XcpTl_Receiver.Buffer[payload_off + i];
                uint8_t recv = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            {
                uint32_t calc = 0u;
                uint16_t i    = 0u;
                while (i < dlc) {
                    uint16_t w  = XcpTl_Receiver.Buffer[payload_off + i];
                    uint16_t w2 = (i + 1u < dlc) ? XcpTl_Receiver.Buffer[payload_off + i + 1u] : 0u;
                    calc += (uint16_t)(w | (uint16_t)(w2 << 8));
                    i += 2u;
                }
                uint16_t off  = payload_off + dlc + fill;
                uint16_t recv = (uint16_t)(((uint16_t)XcpTl_Receiver.Buffer[off] << 8) | XcpTl_Receiver.Buffer[off + 1u]);
                if (((uint16_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_WORD)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x01) {
            XcpTl_Receiver.Dlc = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x00);
            /* Validate DLC against buffer capacity (account for 2-byte LEN + 2-byte CTR) */
            if (XcpTl_Receiver.Dlc > ((uint16_t)sizeof(XcpTl_Receiver.Buffer) - 4u)) {
                /* Length too large: reset */
                XcpTl_ResetSM();
                XcpTl_TimeoutStop();
                XCP_TL_LEAVE_CRITICAL();
                return;
            }
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc;
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((XcpTl_Receiver.Dlc & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u; /* fill byte before checksum */
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            uint16_t payload_off = 2u;
            uint16_t dlc         = XcpTl_Receiver.Dlc;
            uint16_t fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                uint32_t calc = 0u;
                for (uint16_t i = 0u; i < dlc; ++i)
                    calc += XcpTl_Receiver.Buffer[payload_off + i];
                uint8_t recv = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            {
                uint32_t calc = 0u;
                uint16_t i    = 0u;
                while (i < dlc) {
                    uint16_t w  = XcpTl_Receiver.Buffer[payload_off + i];
                    uint16_t w2 = (i + 1u < dlc) ? XcpTl_Receiver.Buffer[payload_off + i + 1u] : 0u;
                    calc += (uint16_t)(w | (uint16_t)(w2 << 8));
                    i += 2u;
                }
                uint16_t off  = payload_off + dlc + fill;
                uint16_t recv = (uint16_t)(((uint16_t)XcpTl_Receiver.Buffer[off] << 8) | XcpTl_Receiver.Buffer[off + 1u]);
                if (((uint16_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_WORD)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x01) {
            XcpTl_Receiver.Dlc       = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x00);
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc + 2; /* CTR */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((XcpTl_Receiver.Dlc & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u; /* fill byte before checksum */
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        if (XcpTl_Receiver.Index == 0x03) {
            XcpTl_Receiver.Ctr = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x02);
        }
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4;
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            uint16_t payload_off = 4u;
            uint16_t dlc         = XcpTl_Receiver.Dlc;
            uint16_t fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                uint32_t calc = 0u;
                for (uint16_t i = 0u; i < dlc; ++i)
                    calc += XcpTl_Receiver.Buffer[payload_off + i];
                uint8_t recv = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            {
                uint32_t calc = 0u;
                uint16_t i    = 0u;
                while (i < dlc) {
                    uint16_t w  = XcpTl_Receiver.Buffer[payload_off + i];
                    uint16_t w2 = (i + 1u < dlc) ? XcpTl_Receiver.Buffer[payload_off + i + 1u] : 0u;
                    calc += (uint16_t)(w | (uint16_t)(w2 << 8));
                    i += 2u;
                }
                uint16_t off  = payload_off + dlc + fill;
                uint16_t recv = (uint16_t)(((uint16_t)XcpTl_Receiver.Buffer[off] << 8) | XcpTl_Receiver.Buffer[off + 1u]);
                if (((uint16_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_FILL_WORD)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x01) {
            XcpTl_Receiver.Dlc       = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x00);
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc + 2; /* FILL */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((XcpTl_Receiver.Dlc & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u; /* fill byte before checksum */
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4;
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            uint16_t payload_off = 4u;
            uint16_t dlc         = XcpTl_Receiver.Dlc;
            uint16_t fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                uint32_t calc = 0u;
                for (uint16_t i = 0u; i < dlc; ++i)
                    calc += XcpTl_Receiver.Buffer[payload_off + i];
                uint8_t recv = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            {
                uint32_t calc = 0u;
                uint16_t i    = 0u;
                while (i < dlc) {
                    uint16_t w  = XcpTl_Receiver.Buffer[payload_off + i];
                    uint16_t w2 = (i + 1u < dlc) ? XcpTl_Receiver.Buffer[payload_off + i + 1u] : 0u;
                    calc += (uint16_t)(w | (uint16_t)(w2 << 8));
                    i += 2u;
                }
                uint16_t off  = payload_off + dlc + fill;
                uint16_t recv = (uint16_t)(((uint16_t)XcpTl_Receiver.Buffer[off] << 8) | XcpTl_Receiver.Buffer[off + 1u]);
                if (((uint16_t)calc) == recv) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #else
        #error "Please define XCP_ON_SXI_HEADER_FORMAT to a valid value"
    #endif
    }
    if (XcpTl_Receiver.State != XCP_RCV_IDLE) {
        XcpTl_Receiver.Index++;
    }
    XCP_TL_LEAVE_CRITICAL();
}

    #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
typedef uint8_t XcpSxiChecksumType;
    #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
typedef uint16_t XcpSxiChecksumType;
    #endif

static void XcpTl_TransformAndSend(uint8_t const *buf, uint16_t len) {
    uint16_t i;

    (void)Serial.write((uint8_t)XCP_ON_SXI_SYNC_CHAR);

    for (i = 0; i < len; i++) {
        if (buf[i] == XCP_ON_SXI_SYNC_CHAR) {
            (void)Serial.write((uint8_t)XCP_ON_SXI_ESC_CHAR);
            (void)Serial.write((uint8_t)XCP_ON_SXI_ESC_SYNC_CHAR);
        } else if (buf[i] == XCP_ON_SXI_ESC_CHAR) {
            (void)Serial.write((uint8_t)XCP_ON_SXI_ESC_CHAR);
            (void)Serial.write((uint8_t)XCP_ON_SXI_ESC_ESC_CHAR);
        } else {
            (void)Serial.write(buf[i]);
        }
    }
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    XCP_TL_ENTER_CRITICAL();

#if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
    {
        XcpSxiChecksumType checksum = 0;
        uint16_t           i;

        /* Calculate checksum over original data */
        for (i = 0; i < len; i++) {
            checksum += buf[i];
        }

#if defined(ARDUINO)
#if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
        XcpTl_TransformAndSend(buf, len);
#else
        Serial.write(buf, len);
#endif
#endif

#if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
        /* Add fill byte if length is odd for word checksum */
        if ((len % 2) != 0) {
            uint8_t fill_byte = 0x00;
            checksum += fill_byte;
#if defined(ARDUINO)
#if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
            XcpTl_TransformAndSend(&fill_byte, 1);
#else
            Serial.write(&fill_byte, 1);
#endif
#endif
        }
#endif

        /* Append checksum to buffer */
#if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
        {
            uint8_t checksum_bytes[2];
            checksum_bytes[0] = (uint8_t)(checksum >> 8);
            checksum_bytes[1] = (uint8_t)(checksum);
#if defined(ARDUINO)
#if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
            XcpTl_TransformAndSend(checksum_bytes, 2);
#else
            Serial.write(checksum_bytes, 2);
#endif
#endif
        }
#else /* XCP_ON_SXI_CHECKSUM_BYTE */
        {
            uint8_t checksum_byte = (uint8_t)checksum;
#if defined(ARDUINO)
#if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
            XcpTl_TransformAndSend(&checksum_byte, 1);
#else
            Serial.write(&checksum_byte, 1);
#endif
#endif
        }
#endif
    }
#else
#if defined(ARDUINO)
#if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
    XcpTl_TransformAndSend(buf, len);
#else
    Serial.write(buf, len);
#endif
#endif
#endif /* XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM */

    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_SaveConnection(void) {
}

void XcpTl_ReleaseConnection(void) {
}

void XcpTl_PrintConnectionInformation(void) {
    #if defined(ARDUINO)
    Serial.println("XCPonSxi");
    #else
    printf("\nXCPonSxi\n");
    #endif
}

static void XcpTl_SignalTimeout(void) {
    XCP_TL_ENTER_CRITICAL();
    XcpTl_ResetSM();
    XCP_TL_LEAVE_CRITICAL();
}

    #if 0
void serialEventRun(void)
{
    if (Serial.available()) {
        serialEvent();
    }
}

void serialEvent()
{
    digitalWrite(LED_BUILTIN, HIGH);
    XcpTl_RxHandler();
    digitalWrite(LED_BUILTIN, LOW);
}
    #endif

#endif /* XCP_TRANSPORT_LAYER == XCP_ON_SXI */
