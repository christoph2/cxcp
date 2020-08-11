#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ctypes
import enum
from pprint import pprint
import pytest

FUNCS = """XcpDaq_Alloc
XcpDaq_AllocOdt
XcpDaq_AllocOdtEntry
XcpDaq_CopyMemory
XcpDaq_DequeueMessage
XcpDaq_EnqueueMessage
XcpDaq_Free
XcpDaq_GetEventConfiguration
XcpDaq_GetFirstPid
XcpDaq_GetListConfiguration
XcpDaq_GetListState
XcpDaq_GetOdtEntry
XcpDaq_GetProperties
XcpDaq_Init
XcpDaq_MainFunction
XcpDaq_SetProcessorState
XcpDaq_TriggerEvent
XcpDaq_ValidateList
XcpDaq_ValidateOdtEntry
"""

class Xcp_ReturnType(enum.IntEnum):
    ERR_CMD_SYNCH           = 0x00

    ERR_CMD_BUSY            = 0x10
    ERR_DAQ_ACTIVE          = 0x11
    ERR_PGM_ACTIVE          = 0x12

    ERR_CMD_UNKNOWN         = 0x20
    ERR_CMD_SYNTAX          = 0x21
    ERR_OUT_OF_RANGE        = 0x22
    ERR_WRITE_PROTECTED     = 0x23
    ERR_ACCESS_DENIED       = 0x24
    ERR_ACCESS_LOCKED       = 0x25
    ERR_PAGE_NOT_VALID      = 0x26
    ERR_MODE_NOT_VALID      = 0x27
    ERR_SEGMENT_NOT_VALID   = 0x28
    ERR_SEQUENCE            = 0x29
    ERR_DAQ_CONFIG          = 0x2A

    ERR_MEMORY_OVERFLOW     = 0x30
    ERR_GENERIC             = 0x31
    ERR_VERIFY              = 0x32
    ERR_RESOURCE_TEMPORARY_NOT_ACCESSIBLE = 0x33

    ERR_SUCCESS             = 0xff

DLL_NAME = "test_daq.so"

dll = ctypes.CDLL(DLL_NAME)

FUNCTIONS = {}
for func in FUNCS.splitlines():
    func = func.strip()
    FUNCTIONS[func] = getattr(dll, func)

#pprint(FUNCTIONS, indent = 4)

@pytest.fixture
def daq():
    xcpdaq_init()
    xcpdaq_free()

def xcpdaq_init():
    FUNCTIONS["XcpDaq_Init"]()

def xcpdaq_free():
    return FUNCTIONS["XcpDaq_Free"]()

def xcpdaq_alloc(daq_count: int) -> int:
    return FUNCTIONS["XcpDaq_Alloc"](daq_count)

def xcpdaq_allocodt(daq_list_num: int, odt_count: int) -> int:
    return FUNCTIONS["XcpDaq_AllocOdt"](daq_list_num, odt_count)
##
##
##


##
## Test XcpDaq_AllocTransitionTable
##
def test_alloc_seq_err():
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SEQUENCE

def test_alloc_ok(daq):
    assert xcpdaq_alloc(5) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_seq_err(daq):
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_ok(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS

##
##
##
def test_alloc_out_of_mem(daq):
    assert xcpdaq_alloc(101) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

def test_allocodt_out_of_mem(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 100) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

def main():
    xcpdaq_init()
    print(xcpdaq_free())
    res = xcpdaq_alloc(5)
    print(res)
    res = xcpdaq_alloc(50000)
    print(res)

if __name__ == '__main__':
    main()

