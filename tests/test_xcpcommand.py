import pytest

from cxcp import MemoryInfo, dispatch_command, xcp_init, xcp_set_mta


def _connect():
    xcp_init()
    _, _, response = dispatch_command(bytes((0xFF, 0x00)))
    assert response[:1] == b"\xFF"


@pytest.mark.parametrize(
    "command",
    [
        b"",
        b"\xF6",
        b"\xF8\x00",
        b"\xF0\x02\xAA",
        b"\xED\x01\x00\x00\x00\x00\x00\x00",
        b"\xEE",
        b"\xEC\x00\xFF\xFF\xFF",
        b"\xF2\xFA",
        b"\xC7\xFF",
        b"\xC0\x01",
        b"\xFF" + bytes(64),
    ],
)
def test_dispatch_rejects_truncated_or_oversized_ctos(command):
    _connect()

    _, _, response = dispatch_command(command)

    assert response[:2] == b"\xFE\x21"


def test_modify_bits_only_changes_two_authorized_bytes():
    _connect()
    memory = MemoryInfo()
    xcp_set_mta(memory.absolute_address(0))

    _, _, response = dispatch_command(b"\xEC\x00\xFF\xFF\x34\x12")

    assert response[:1] == b"\xFF"
    assert [memory.as_uint8(offset) for offset in range(4)] == [0x34, 0x12, 0, 0]
