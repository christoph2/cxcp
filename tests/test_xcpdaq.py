#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ctypes
import enum
import os
from pprint import pprint
import pytest

def libname(name):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), name))


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
XcpDaq_GetCounts
XcpDaq_GetDynamicEntities
XcpDaq_TotalDynamicEntityCount
Xcp_Init
XcpDaq_GetDynamicEntity
XcpDaq_GetDynamicEntities
XcpDaq_GetDtoBuffer
"""


DLL_NAME = "./test_daq.so"

dll = ctypes.CDLL(DLL_NAME)

FUNCTIONS = {}
for func in FUNCS.splitlines():
    func = func.strip()
    FUNCTIONS[func] = getattr(dll, func)

@pytest.fixture
def daq():
    xcp_init()
    xcpdaq_init()

def xcp_init():
    FUNCTIONS["Xcp_Init"]()

def xcpdaq_init():
    FUNCTIONS["XcpDaq_Init"]()

def xcpdaq_get_counts():
    entityCount = ctypes.c_uint16()
    listCount = ctypes.c_uint16()
    odtCount = ctypes.c_uint16()
    FUNCTIONS["XcpDaq_GetCounts"](ctypes.byref(entityCount), ctypes.byref(listCount), ctypes.byref(odtCount))
    return (entityCount.value, listCount.value, odtCount.value, )

def xcpdaq_free():
    return FUNCTIONS["XcpDaq_Free"]()

def xcpdaq_alloc(daq_count: int) -> int:
    return FUNCTIONS["XcpDaq_Alloc"](daq_count)

def xcpdaq_allocodt(daq_list_num: int, odt_count: int) -> int:
    return FUNCTIONS["XcpDaq_AllocOdt"](daq_list_num, odt_count)

def xcpdaq_allocodt_entry(daq_list_num: int, odt_num: int, entry_count: int) -> int:
    return FUNCTIONS["XcpDaq_AllocOdtEntry"](daq_list_num, odt_num, entry_count)

def xcpdaq_total_dynamic_entity_count():
    return FUNCTIONS["XcpDaq_TotalDynamicEntityCount"]()

def xcpdaq_get_dynamic_entities():
    vp = FUNCTIONS["XcpDaq_GetDynamicEntities"]()
    return vp

def xcpdaq_get_dto_buffer():
    return FUNCTIONS["XcpDaq_GetDtoBuffer"]()

def xcpdaq_get_dynamic_entity(num):
    vp = FUNCTIONS["XcpDaq_GetDynamicEntity"](num)
    return vp

def xcpdaq_get_dynamic_entity_content(num):
    addr = xcpdaq_get_dynamic_entity(num)
    entry = ctypes.cast(addr, ctypes.POINTER(XcpDaq_EntityType))
    kind = entry.contents.kind
    entity = entry.contents.entity

    # Function is polymorphic on return.
    if kind == XcpDaq_EntityKindType.XCP_ENTITY_UNUSED:
        result = None
    elif kind == XcpDaq_EntityKindType.XCP_ENTITY_DAQ_LIST:
        result = entity.daqList
    elif kind == XcpDaq_EntityKindType.XCP_ENTITY_ODT:
        result = entity.odt
    elif kind == XcpDaq_EntityKindType.XCP_ENTITY_ODT_ENTRY:
        result = entity.odtEntry
    return XcpDaq_EntityKindType(kind), result

def xcpdaq_check_entries_are_unused(start_idx: int = 0):
    for idx in range(start_idx, xcpdaq_total_dynamic_entity_count()):
        ent = xcpdaq_get_dynamic_entity_content(idx)
        assert ent == (XcpDaq_EntityKindType.XCP_ENTITY_UNUSED, None)

##
## Test XcpDaq_AllocTransitionTable
##
def test_alloc_seq_err():
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_seq_err(daq):
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_entry_seq_err(daq):
    assert xcpdaq_allocodt_entry(0, 1, 2) == Xcp_ReturnType.ERR_SEQUENCE

def test_alloc_ok(daq):
    xcpdaq_check_entries_are_unused()
    assert xcpdaq_alloc(5) == Xcp_ReturnType.ERR_SUCCESS
    ec, _, _ = xcpdaq_get_counts()
    for idx in range(0, ec):
        state, ent = xcpdaq_get_dynamic_entity_content(idx)

        print("ENT", state, ent.numOdts, ent.firstOdt, ent.mode)

    xcpdaq_check_entries_are_unused(5)

def test_allocodt_ok(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_entry_ok(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt_entry(0, 1, 1) == Xcp_ReturnType.ERR_SUCCESS
    xcpdaq_get_counts()

def test_free_ok1(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok2(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok3(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok4(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt_entry(0, 1, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_free() == Xcp_ReturnType.ERR_SUCCESS

def test_alloc_aftera_alloc(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc(2) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_after_allocodt(daq):
    assert xcpdaq_alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(1, 3) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_entry_after_allocodt_entry(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt_entry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt_entry(0, 1, 2) == Xcp_ReturnType.ERR_SUCCESS

def test_alloc_after_allocodt(daq):
    assert xcpdaq_alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc(3) == Xcp_ReturnType.ERR_SEQUENCE

def test_alloc_after_allocodt_entry(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt_entry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc(3) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_after_allocodt_entry(daq):
    assert xcpdaq_alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt_entry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(1, 3) == Xcp_ReturnType.ERR_SEQUENCE

    #for idx in range(8):
    #    ent = xcpdaq_get_dynamic_entity_content(idx)
    #    print(ent)

##
##  Memory Overflows.
##
def test_alloc_out_of_mem(daq):
    assert xcpdaq_alloc(101) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

def test_allocodt_out_of_mem(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 100) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

def test_allocodt_entry_out_of_mem(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt_entry(0, 1, 100) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

##
##
##
def test_basic_alloc1(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_get_counts() == (1, 1, 0)

def test_basic_alloc2(daq):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_get_counts() == (3, 1, 2)

def test_basic_alloc3(daq):
    assert xcpdaq_alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt_entry(0, 0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_allocodt_entry(0, 1, 3) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_get_counts() == (9, 2, 2)
