
import pytest
import ctypes

DLL_NAME = "test_cs.so"

#dll = ctypes.windll.LoadLibrary(DLL_NAME)
dll = ctypes.CDLL(DLL_NAME)
print(dll)
calc = dll.Xcp_CalculateChecksum
print(calc)

DATA = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
#DATA = (ctypes.c_uint8 * 3)(1,2,3)
arr = ctypes.c_char_p(bytes(DATA))
print(arr)

res = calc(ctypes.byref(arr), 10, 0, 1)
print(hex(res))

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


