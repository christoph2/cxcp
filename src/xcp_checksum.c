/*
 * BlueParrot XCP
 *
 * (C) 2007-2026 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 */

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
#include "xcp_util.h"

/*!!! END-INCLUDE-SECTION !!!*/

/*
** Local Types
*/
typedef enum tagXcp_ChecksumJobStateType {
    XCP_CHECKSUM_STATE_IDLE,
    XCP_CHECKSUM_STATE_RUNNING_INITIAL,
    XCP_CHECKSUM_STATE_RUNNING_REMAINING,
    XCP_CHECKSUM_STATE_RUNNING_FINAL
} Xcp_ChecksumJobStateType;

typedef struct tagXcp_ChecksumJobType {
    Xcp_ChecksumJobStateType state;
    Xcp_MtaType              mta;
    uint32_t                 size;
    Xcp_ChecksumType         interimChecksum;
} Xcp_ChecksumJobType;

#if (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16)
    #define XCP_CRC_POLYNOMIAL      ((Xcp_ChecksumType)0x8005u)
    #define XCP_CRC_INITIAL_VALUE   ((Xcp_ChecksumType)0x0000u)
    #define XCP_CRC_FINAL_XOR_VALUE ((Xcp_ChecksumType)0x0000u)
    #define XCP_REFLECT_DATA        XCP_TRUE
    #define XCP_REFLECT_REMAINDER   XCP_TRUE
#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT)
    #define XCP_CRC_POLYNOMIAL      ((Xcp_ChecksumType)0x1021u)
    #define XCP_CRC_INITIAL_VALUE   ((Xcp_ChecksumType)0xFFFFu)
    #define XCP_CRC_FINAL_XOR_VALUE ((Xcp_ChecksumType)0x0000u)
    #define XCP_REFLECT_DATA        XCP_FALSE
    #define XCP_REFLECT_REMAINDER   XCP_FALSE
#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_32)
    #define XCP_CRC_POLYNOMIAL      ((Xcp_ChecksumType)0x04C11DB7u)
    #define XCP_CRC_INITIAL_VALUE   ((Xcp_ChecksumType)0xFFFFFFFFu)
    #define XCP_CRC_FINAL_XOR_VALUE ((Xcp_ChecksumType)0xFFFFFFFFu)
    #define XCP_REFLECT_DATA        XCP_TRUE
    #define XCP_REFLECT_REMAINDER   XCP_TRUE
#endif

#define XCP_CHECKSUM_WIDTH  ((uint8_t)(8u * sizeof(Xcp_ChecksumType)))
#define XCP_CHECKSUM_TOPBIT ((Xcp_ChecksumType)((Xcp_ChecksumType)1u << (XCP_CHECKSUM_WIDTH - 1u)))

#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON && XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON
static Xcp_ChecksumJobType Xcp_ChecksumJob;
#endif

static uint32_t Xcp_ChecksumReflect(uint32_t data, uint8_t bitCount) {
    uint32_t reflection = 0u;
    uint8_t  bit        = 0u;

    for (bit = 0u; bit < bitCount; ++bit) {
        if ((data & 0x01u) != 0u) {
            reflection |= (uint32_t)(1u << ((bitCount - 1u) - bit));
        }
        data >>= 1u;
    }
    return reflection;
}

#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON && XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON
static void Xcp_ChecksumSendAddressError(void) {
    Xcp_Send8(
        UINT8(2), UINT8(XCP_PACKET_IDENTIFIER_ERR), UINT8(ERR_OUT_OF_RANGE), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0),
        UINT8(0)
    );
}

static bool Xcp_ChecksumReadChunk(uint8_t *dst, uint32_t length) {
    Xcp_MtaType mapped = { 0 };

    if (!Xcp_MapMemory(&mapped, &Xcp_ChecksumJob.mta)) {
        return XCP_FALSE;
    }
    XcpUtl_MemCopy(dst, (void const *)mapped.address, length);
    Xcp_ChecksumJob.mta.address += length;
    return XCP_TRUE;
}
#endif

void Xcp_ChecksumInit(void) {
#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON && XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON
    Xcp_ChecksumJob.mta.address     = UINT32(0ul);
    Xcp_ChecksumJob.mta.ext         = UINT8(0);
    Xcp_ChecksumJob.interimChecksum = (Xcp_ChecksumType)0UL;
    Xcp_ChecksumJob.size            = UINT32(0ul);
    Xcp_ChecksumJob.state           = XCP_CHECKSUM_STATE_IDLE;
#endif
}

Xcp_ChecksumType Xcp_CalculateChecksum(uint8_t const *ptr, uint32_t length, Xcp_ChecksumType startValue, bool isFirstCall) {
    Xcp_ChecksumType result = isFirstCall ? (Xcp_ChecksumType)0 : startValue;
    uint32_t         idx    = 0u;

#if (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT) ||     \
    (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_32)
    result = isFirstCall ? XCP_CRC_INITIAL_VALUE : startValue;
    for (idx = 0u; idx < length; ++idx) {
        Xcp_ChecksumType data = (Xcp_ChecksumType)ptr[idx];
        uint8_t          bit  = 0u;

        if (XCP_REFLECT_DATA == XCP_TRUE) {
            data = (Xcp_ChecksumType)Xcp_ChecksumReflect((uint32_t)data, 8u);
        }
        result ^= (Xcp_ChecksumType)(data << (XCP_CHECKSUM_WIDTH - 8u));
        for (bit = 0u; bit < 8u; ++bit) {
            if ((result & XCP_CHECKSUM_TOPBIT) != 0u) {
                result = (Xcp_ChecksumType)((result << 1u) ^ XCP_CRC_POLYNOMIAL);
            } else {
                result = (Xcp_ChecksumType)(result << 1u);
            }
        }
    }
    if (XCP_REFLECT_REMAINDER == XCP_TRUE) {
        result = (Xcp_ChecksumType)Xcp_ChecksumReflect((uint32_t)result, XCP_CHECKSUM_WIDTH);
    }
    return (Xcp_ChecksumType)(result ^ XCP_CRC_FINAL_XOR_VALUE);
#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_11) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_12) ||        \
    (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_14)
    for (idx = 0u; idx < length; ++idx) {
        result = (Xcp_ChecksumType)(result + ptr[idx]);
    }
    return result;
#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_22) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_24)
    for (idx = 0u; (idx + 1u) < length; idx += 2u) {
        uint16_t value = 0u;
        XcpUtl_MemCopy(&value, &ptr[idx], sizeof(value));
        result = (Xcp_ChecksumType)(result + value);
    }
    return result;
#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_44)
    for (idx = 0u; (idx + 3u) < length; idx += 4u) {
        uint32_t value = 0u;
        XcpUtl_MemCopy(&value, &ptr[idx], sizeof(value));
        result = (Xcp_ChecksumType)(result + value);
    }
    return result;
#else
    XCP_UNREFERENCED_PARAMETER(ptr);
    XCP_UNREFERENCED_PARAMETER(length);
    return result;
#endif
}

void Xcp_StartChecksumCalculation(Xcp_MtaType mta, uint32_t size) {
#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON && XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON
    XCP_ENTER_CRITICAL();
    if ((Xcp_ChecksumJob.state != XCP_CHECKSUM_STATE_IDLE) || Xcp_IsBusy()) {
        XCP_LEAVE_CRITICAL();
        return;
    }
    Xcp_SetBusy(XCP_TRUE);
    Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_RUNNING_INITIAL;
    Xcp_ChecksumJob.mta   = mta;
    Xcp_ChecksumJob.size  = size;
    XCP_LEAVE_CRITICAL();
#else
    XCP_UNREFERENCED_PARAMETER(mta);
    XCP_UNREFERENCED_PARAMETER(size);
#endif
}

void Xcp_ChecksumMainFunction(void) {
#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON && XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON
    uint8_t  chunk[XCP_CHECKSUM_CHUNK_SIZE];
    uint32_t length      = 0u;
    bool     isFirstCall = XCP_FALSE;

    if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_IDLE) {
        return;
    }

    if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_RUNNING_INITIAL) {
        length      = XCP_CHECKSUM_CHUNK_SIZE;
        isFirstCall = XCP_TRUE;
    } else if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_RUNNING_REMAINING) {
        length = XCP_CHECKSUM_CHUNK_SIZE;
    } else {
        length = Xcp_ChecksumJob.size;
    }

    if (!Xcp_ChecksumReadChunk(chunk, length)) {
        Xcp_SetBusy(XCP_FALSE);
        Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_IDLE;
        Xcp_ChecksumSendAddressError();
        return;
    }

    Xcp_ChecksumJob.interimChecksum = Xcp_CalculateChecksum(chunk, length, Xcp_ChecksumJob.interimChecksum, isFirstCall);
    Xcp_ChecksumJob.size -= length;

    if (Xcp_ChecksumJob.size == 0u) {
        Xcp_SetBusy(XCP_FALSE);
        Xcp_SendChecksumPositiveResponse(Xcp_ChecksumJob.interimChecksum);
        Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_IDLE;
    } else if (Xcp_ChecksumJob.size <= XCP_CHECKSUM_CHUNK_SIZE) {
        Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_RUNNING_FINAL;
    } else {
        Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_RUNNING_REMAINING;
    }
#endif
}
