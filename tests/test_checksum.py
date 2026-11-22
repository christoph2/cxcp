from binascii import crc_hqx

from cxcp import Xcp_CalculateChecksum


TEST_DATA = bytes(
    [
        *range(1, 17),
        *range(0xF1, 0x100),
        0,
    ]
)


def test_checksum_matches_crc16_citt():
    assert Xcp_CalculateChecksum(TEST_DATA) == crc_hqx(TEST_DATA, 0xFFFF)


def test_checksum_can_be_calculated_in_chunks():
    split = len(TEST_DATA) // 2
    first = Xcp_CalculateChecksum(TEST_DATA[:split])

    assert Xcp_CalculateChecksum(TEST_DATA[split:], first, False) == crc_hqx(TEST_DATA, 0xFFFF)
