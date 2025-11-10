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

// #include "xcp_config.h"

#include <stdio.h>

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_tl_timeout.h"
#include "xcp_util.h"
/*!!! END-INCLUDE-SECTION !!!*/

#if XCP_TRANSPORT_LAYER == XCP_ON_SXI

    #define XCP_SXI_MAKEWORD(buf, offs) ((uint16_t)(*(((buf)) + (offs))) | ((uint16_t)(*(((buf)) + (offs) + 1)) << 8))

    #define TIMEOUT_VALUE (100)

    #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
typedef uint8_t XcpSxiChecksumType;
    #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
typedef uint16_t XcpSxiChecksumType;
    #endif

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
    Serial_Init();

    XcpTl_ResetSM();
    XcpTl_TimeoutInit(TIMEOUT_VALUE, XcpTl_ResetSM);
}

void XcpTl_DeInit(void) {
    Serial_DeInit();
}

void XcpTl_MainFunction(void) {
    uint32_t byte_count = 0UL;
    uint8_t  octet      = 0;

    Serial_MainFunction();
    byte_count = Serial_Available();
    if (byte_count > 0U) {
        while (byte_count--) {
            if (Serial_Read(&octet)) {
                XcpTl_FeedReceiver(octet);
            }
        }
    }
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
    XcpTl_Receiver.ReceivedChecksum = 0;
    #endif
    #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
    s_FramingState = FRM_WAIT_FOR_SYNC;
    #endif
}

    #if defined(XCP_TL_TEST_HOOKS)
/* Test hook to invoke the internal state machine reset from unit-tests. */
void XcpTl_Test_ResetSM(void) {
    XcpTl_ResetSM();
}
    #endif

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
                s_FramingState = FRM_RECEIVING;
            } else if (octet == XCP_ON_SXI_ESC_ESC_CHAR) {
                XcpTl_ProcessOctet(XCP_ON_SXI_ESC_CHAR);
                s_FramingState = FRM_RECEIVING;
            } else {
                /* Protocol error, invalid escape sequence. Reset. */
                XcpTl_ResetSM();
                s_FramingState = FRM_WAIT_FOR_SYNC;
            }
            break;
    }
    #else
    XcpTl_ProcessOctet(octet);
    #endif
}

static void XcpTl_ProcessOctet(uint8_t octet) {
    #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
    XcpSxiChecksumType calc_checksum = 0u;
    uint16_t           idx           = 0u;
    uint16_t           payload_off   = 0u;
    uint16_t           dlc           = 0u;
    uint16_t           fill          = 0u;
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
    uint16_t w   = 0u;
    uint16_t w2  = 0u;
    uint16_t off = 0u;
        #endif
    #endif

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
        if (XcpTl_Receiver.Index == 0x01) {
            XcpTl_Receiver.Dlc       = XcpTl_Receiver.Buffer[0];
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc - 1;
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            /* Even byte-count required. */
            if (((XcpTl_Receiver.Dlc + 1u) & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u;
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        /* Handle minimal-length frames immediately (e.g., LEN=1, no checksum) */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            if (XcpTl_Receiver.Remaining == 0u) {
                Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 1; /* payload follows 1-byte LEN */
                XcpTl_ResetSM();
                XcpTl_TimeoutStop();
                Xcp_DispatchCommand(&Xcp_CtoIn);
                XCP_TL_LEAVE_CRITICAL();
                return;
            }
        #endif
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
            payload_off = 1u;
            dlc         = XcpTl_Receiver.Dlc;
            fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if (((dlc + 1u) & 1u) != 0u) {
                /* Even byte-count required. */
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                calc_checksum = 0u;
                for (idx = 0u; idx < (1u + dlc); ++idx) {
                    calc_checksum += XcpTl_Receiver.Buffer[idx];
                }
                XcpTl_Receiver.ReceivedChecksum = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc_checksum) == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 1;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #else /* XCP_ON_SXI_CHECKSUM_WORD */
            {
                calc_checksum = 0u;
                /* Calculate WORD checksum over header and payload */
                for (idx = 0u; idx < ((payload_off + dlc + fill) / 2u); ++idx) {
                    calc_checksum += (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[2u * idx] |
                                                ((uint16_t)XcpTl_Receiver.Buffer[2u * idx + 1u] << 8));
                }

                XcpTl_Receiver.ReceivedChecksum = (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill] |
                                                             ((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill + 1u] << 8));

                if (calc_checksum == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 1;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_BYTE) ||                                                          \
        (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_FILL_BYTE)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x01) {
            /* DLC and CTR/FILL received */
            XcpTl_Receiver.Dlc       = XcpTl_Receiver.Buffer[0];
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc;
        #if (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_BYTE)
            XcpTl_Receiver.Ctr = XcpTl_Receiver.Buffer[1];
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((XcpTl_Receiver.Dlc & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u; /* fill byte before checksum */
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        /* Handle minimal-length frames immediately (e.g., LEN=0, no checksum) */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            if (XcpTl_Receiver.Remaining == 0u) {
                Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2; /* payload after LEN + CTR/FILL */
                XcpTl_ResetSM();
                XcpTl_TimeoutStop();
                Xcp_DispatchCommand(&Xcp_CtoIn);
                XCP_TL_LEAVE_CRITICAL();
                return;
            }
        #endif
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {

        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2; /* Data starts after LEN and CTR/FILL */
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            payload_off = 2u; /* Data starts after LEN and CTR/FILL */
            dlc         = XcpTl_Receiver.Dlc;
            fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                calc_checksum = 0u;
                for (idx = 0u; idx < (2u + dlc); ++idx) {
                    calc_checksum += XcpTl_Receiver.Buffer[idx];
                }
                XcpTl_Receiver.ReceivedChecksum = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc_checksum) == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #else /* XCP_ON_SXI_CHECKSUM_WORD */
            {
                calc_checksum = 0u;
                /* Calculate WORD checksum over header and payload */
                for (idx = 0u; idx < ((payload_off + dlc + fill) / 2u); ++idx) {
                    calc_checksum += (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[2u * idx] |
                                                ((uint16_t)XcpTl_Receiver.Buffer[2u * idx + 1u] << 8));
                }

                XcpTl_Receiver.ReceivedChecksum = (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill] |
                                                             ((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill + 1u] << 8));

                if (calc_checksum == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
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
        /* Handle minimal-length frames immediately (e.g., LEN=0, no checksum) */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            if (XcpTl_Receiver.Remaining == 0u) {
                Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2; /* payload after 2-byte LEN */
                XcpTl_ResetSM();
                XcpTl_TimeoutStop();
                Xcp_DispatchCommand(&Xcp_CtoIn);
                XCP_TL_LEAVE_CRITICAL();
                return;
            }
        #endif
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
            payload_off = 2u;
            dlc         = XcpTl_Receiver.Dlc;
            fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                calc_checksum = 0u;
                for (idx = 0u; idx < (2u + dlc); ++idx) {
                    calc_checksum += XcpTl_Receiver.Buffer[idx];
                }
                XcpTl_Receiver.ReceivedChecksum = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc_checksum) == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #else /* XCP_ON_SXI_CHECKSUM_WORD */
            {
                calc_checksum = 0u;
                /* Calculate WORD checksum over header and payload */
                for (idx = 0u; idx < ((payload_off + dlc + fill) / 2u); ++idx) {
                    calc_checksum += (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[2u * idx] |
                                                ((uint16_t)XcpTl_Receiver.Buffer[2u * idx + 1u] << 8));
                }

                XcpTl_Receiver.ReceivedChecksum = (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill] |
                                                             ((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill + 1u] << 8));

                if (calc_checksum == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_WORD) ||                                                          \
        (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_FILL_WORD)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x03) {
            /* Wait for 4 header bytes (LEN+CTR/FILL) */
            XcpTl_Receiver.Dlc = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x00);
            /* Validate DLC against buffer capacity (account for 4-byte header) */
            if (XcpTl_Receiver.Dlc > ((uint16_t)sizeof(XcpTl_Receiver.Buffer) - 4u)) {
                /* Length too large: reset */
                XcpTl_ResetSM();
                XcpTl_TimeoutStop();
                XCP_TL_LEAVE_CRITICAL();
                return;
            }
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc;
        #if (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_WORD)
            XcpTl_Receiver.Ctr = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x02);
        #endif
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
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4; /* Data starts after LEN and CTR/FILL */
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            payload_off = 4u; /* Data starts after LEN and CTR/FILL */
            dlc         = XcpTl_Receiver.Dlc;
            fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                calc_checksum = 0u;
                for (idx = 0u; idx < (4u + dlc); ++idx) {
                    calc_checksum += XcpTl_Receiver.Buffer[idx];
                }
                XcpTl_Receiver.ReceivedChecksum = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc_checksum) == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #else /* XCP_ON_SXI_CHECKSUM_WORD */
            {
                calc_checksum = 0u;
                for (idx = 0u; idx < ((payload_off + dlc + fill) / 2u); ++idx) {
                    calc_checksum += (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[2u * idx] |
                                                ((uint16_t)XcpTl_Receiver.Buffer[2u * idx + 1u] << 8));
                }

                XcpTl_Receiver.ReceivedChecksum = (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill] |
                                                             ((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill + 1u] << 8));

                if (calc_checksum == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
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
    uint16_t idx;

    Serial_WriteByte((uint8_t)XCP_ON_SXI_SYNC_CHAR);

    for (idx = 0; idx < len; idx++) {
        if (buf[idx] == XCP_ON_SXI_SYNC_CHAR) {
            Serial_WriteByte((uint8_t)XCP_ON_SXI_ESC_CHAR);
            Serial_WriteByte((uint8_t)XCP_ON_SXI_ESC_SYNC_CHAR);
        } else if (buf[idx] == XCP_ON_SXI_ESC_CHAR) {
            Serial_WriteByte((uint8_t)XCP_ON_SXI_ESC_CHAR);
            Serial_WriteByte((uint8_t)XCP_ON_SXI_ESC_ESC_CHAR);
        } else {
            Serial_WriteByte(buf[idx]);
        }
    }
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    XCP_TL_ENTER_CRITICAL();

    #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
    {
        XcpSxiChecksumType checksum = 0;
        uint16_t           idx;

        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
        /* Calculate WORD checksum over header and payload */
        for (idx = 0; idx < (len / 2u); idx++) {
            checksum += (uint16_t)((uint16_t)buf[2u * idx] | ((uint16_t)buf[2u * idx + 1u] << 8));
        }
        /* If the total length is odd, add the last byte and a fill byte of 0 for checksum calculation */
        if ((len & 1u) != 0u) {
            checksum += (uint16_t)buf[len - 1u]; /* Last byte + 0x00 fill byte */
        }
        #else /* XCP_ON_SXI_CHECKSUM_BYTE */
        /* Calculate BYTE checksum over header and payload */
        for (idx = 0; idx < len; idx++) {
            checksum += buf[idx];
        }
        #endif

        /* Send header + payload */
        #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
        XcpTl_TransformAndSend(buf, len);
        #else
        Serial_WriteBuffer(buf, len);
        #endif

        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
        {
            const uint16_t header_off  = (uint16_t)XCP_TRANSPORT_LAYER_BUFFER_OFFSET;
            const uint16_t payload_len = (len >= header_off) ? (len - header_off) : 0u;

            /* Add fill byte to stream if payload length is odd for word checksum */
            if ((payload_len & 1u) != 0u) {
                uint8_t fill_byte = 0x00;
            #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
                XcpTl_TransformAndSend(&fill_byte, 1);
            #else
                Serial_WriteBuffer(&fill_byte, 1);
            #endif
            }
        }
        #endif

        /* Append checksum to stream */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
        {
            uint8_t checksum_bytes[2];
            checksum_bytes[0] = (uint8_t)(checksum);      /* Little Endian: LSB */
            checksum_bytes[1] = (uint8_t)(checksum >> 8); /* Little Endian: MSB */
            #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
            XcpTl_TransformAndSend(checksum_bytes, 2);
            #else
            Serial_WriteBuffer(checksum_bytes, 2);
            #endif
        }
        #else /* XCP_ON_SXI_CHECKSUM_BYTE */
        {
            uint8_t checksum_byte = (uint8_t)checksum;
            #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
            XcpTl_TransformAndSend(&checksum_byte, 1);
            #else
            Serial_WriteBuffer(&checksum_byte, 1);
            #endif
        }
        #endif
    }
    #else
        #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
    XcpTl_TransformAndSend(buf, len);
        #else
    Serial_WriteBuffer(buf, len);
        #endif
    #endif /* XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM */

    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_SaveConnection(void) {
}

void XcpTl_ReleaseConnection(void) {
}

void XcpTl_PrintConnectionInformation(void) {
    #if 0
        #if defined(ARDUINO)
Serial.println("XCPonSxi");
        #else
printf("\nXCPonSxi\n");
        #endif
    #endif
}

static void XcpTl_SignalTimeout(void) {
    XCP_TL_ENTER_CRITICAL();
    XcpTl_ResetSM();
    XCP_TL_LEAVE_CRITICAL();
}

#endif /* XCP_TRANSPORT_LAYER == XCP_ON_SXI */
