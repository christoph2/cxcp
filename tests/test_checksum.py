
import pytest
import ctypes

DLL_NAME = "./test_cs.so"

#dll = ctypes.windll.LoadLibrary(DLL_NAME)
dll = ctypes.CDLL(DLL_NAME)
calc = dll.Xcp_CalculateChecksum

TEST = [
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
    0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0x00,
]


DATA = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
#DATA = (ctypes.c_uint8 * 3)(1,2,3)
arr = ctypes.c_char_p(bytes(DATA))
print(arr)

#res = calc(ctypes.byref(arr), 10, 0, 1)
res = calc(arr, 10, 0, 1)
print(hex(res))

def calculate_checksum(data, initial = 0, first = 1):
    arr = ctypes.c_char_p(bytes(data))
    return calc(arr, len(data), initial, first)

print(hex(calculate_checksum(DATA)))
print(hex(calculate_checksum(TEST)))

"""
#if (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT)
typedef uint16_t Xcp_ChecksumType;
#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_32
typedef uint32_t Xcp_ChecksumType;

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_11

typedef uint8_t Xcp_ChecksumType;

#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_12) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_22)

typedef uint16_t Xcp_ChecksumType;

#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_14) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_24) || \

(XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_44)

typedef uint32_t Xcp_ChecksumType;
#endif // XCP_CHECKSUM_METHOD


Xcp_ChecksumType Xcp_CalculateChecksum(uint8_t const * ptr, uint32_t length, Xcp_ChecksumType startValue, bool isFirstCall);

"""


