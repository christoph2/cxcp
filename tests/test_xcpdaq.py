#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ctypes
import enum
import os
from pprint import pprint
import random
import pytest

from hypothesis import (
    assume, event, example, given, infer, note, reject, register_random, reproduce_failure, settings,
    Verbosity
)
from hypothesis.strategies import (
    booleans, integers, lists, text, characters, builds, composite, tuples, randoms
)

from xcp_if import XCP

from xcp_types import (
    Xcp_ReturnType, XcpDaq_ListIntegerType, XcpDaq_ODTIntegerType, XcpDaq_ODTEntryIntegerType,
    XcpDaq_ListConfigurationType, XcpDaq_ListStateType, XcpDaq_ODTEntryType, XcpDaq_EventType,
    XcpDaq_ProcessorStateType, XcpDaq_ProcessorType, XcpDaq_MessageType, XcpDaq_EntityType,
    XcpDaq_EntityKindType
)

def libname(name):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), name))


DLL_NAME = "./test_daq.so"

dll = ctypes.CDLL(DLL_NAME)

MAX_DAQ_ENTITIES = 100

daq_entity_int = integers(min_value = 0, max_value = MAX_DAQ_ENTITIES * 2)

def choose(size):
    return random.choice(range(size - 1)) if size > 1 else 0

@pytest.fixture
def xcp():
    xcp =XCP(dll)
    xcp.Xcp_Init()
    xcp.XcpDaq_Init()
    yield xcp
    del xcp


def xcpdaq_get_dynamic_entity_content(xcp, num):
    addr = xcp.XcpDaq_GetDynamicEntity(num)
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

def xcpdaq_check_entries_are_unused(xcp, start_idx: int = 0):
    for idx in range(start_idx, xcp.XcpDaq_TotalDynamicEntityCount()):
        ent = xcpdaq_get_dynamic_entity_content(xcp, idx)
        assert ent == (XcpDaq_EntityKindType.XCP_ENTITY_UNUSED, None)

##
## Test XcpDaq_AllocTransitionTable
##
def test_alloc_seq_err():
    xcp =XCP(dll)
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_seq_err(xcp):
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_entry_seq_err(xcp):
    assert xcp.XcpDaq_AllocOdtEntry(0, 1, 2) == Xcp_ReturnType.ERR_SEQUENCE

def test_alloc_ok(xcp):
    xcpdaq_check_entries_are_unused(xcp)
    assert xcp.XcpDaq_Alloc(5) == Xcp_ReturnType.ERR_SUCCESS
    #ec, _, _ = xcp.get_daq_counts()
    #for idx in range(0, ec):
    #    state, ent = xcp.XcpDaq_TotalDynamicEntityCount()
    #    print("ENT", state, ent.numOdts, ent.firstOdt, ent.mode)

    xcpdaq_check_entries_are_unused(xcp, 5)

def test_allocodt_ok(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_entry_ok(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdtEntry(0, 1, 1) == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok1(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_Free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok2(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_Free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok3(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_Free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok4(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdtEntry(0, 1, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_Free() == Xcp_ReturnType.ERR_SUCCESS

def test_alloc_aftera_alloc(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_Alloc(2) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_after_allocodt(xcp):
    assert xcp.XcpDaq_Alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(1, 3) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_entry_after_allocodt_entry(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdtEntry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdtEntry(0, 1, 2) == Xcp_ReturnType.ERR_SUCCESS

def test_alloc_after_allocodt(xcp):
    assert xcp.XcpDaq_Alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_Alloc(3) == Xcp_ReturnType.ERR_SEQUENCE

def test_alloc_after_allocodt_entry(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdtEntry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_Alloc(3) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_after_allocodt_entry(xcp):
    assert xcp.XcpDaq_Alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdtEntry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(1, 3) == Xcp_ReturnType.ERR_SEQUENCE


##
##  Memory Overflows.
##
def test_alloc_out_of_mem(xcp):
    assert xcp.XcpDaq_Alloc(101) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

def test_allocodt_out_of_mem(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, MAX_DAQ_ENTITIES) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

def test_allocodt_entry_out_of_mem(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdtEntry(0, 1, MAX_DAQ_ENTITIES) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

##
##
##
def test_basic_alloc1(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.get_daq_counts() == (1, 1, 0)

def test_basic_alloc2(xcp):
    assert xcp.XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.get_daq_counts() == (3, 1, 2)

def test_basic_alloc3(xcp):
    assert xcp.XcpDaq_Alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdtEntry(0, 0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.XcpDaq_AllocOdtEntry(0, 1, 3) == Xcp_ReturnType.ERR_SUCCESS
    assert xcp.get_daq_counts() == (9, 2, 2)


##
## Hypothesis tests.
##
@given(num = daq_entity_int)
@settings(max_examples = 1000)
def test_hy_alloc(xcp, num):
    if num > MAX_DAQ_ENTITIES:
        assert xcp.XcpDaq_Alloc(num) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
        count = 0
    else:
        assert xcp.XcpDaq_Alloc(num) == Xcp_ReturnType.ERR_SUCCESS
        count = num
    ec, lc, _ = xcp.get_daq_counts()
    assert ec == count
    assert lc == count
    xcp.XcpDaq_Free()

@given(daq_entity_int, daq_entity_int, daq_entity_int)
@settings(max_examples = 1000)
@example(0, 0, 0)
#@settings(verbosity = Verbosity.verbose)
def test_hy_alloc_odt_entities(xcp, n0, n1, n2):
    if n0 > MAX_DAQ_ENTITIES:
        assert xcp.XcpDaq_Alloc(n0) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
    else:
        assert xcp.XcpDaq_Alloc(n0) == Xcp_ReturnType.ERR_SUCCESS
        if n0 == 0:
            xcp.XcpDaq_Free()
            return
        else:
            daq_list = choose(n0)
        if (n0 + n1) > MAX_DAQ_ENTITIES:
            assert xcp.XcpDaq_AllocOdt(daq_list, n1) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
        else:
            assert xcp.XcpDaq_AllocOdt(daq_list, n1) == Xcp_ReturnType.ERR_SUCCESS
            if n1 == 0:
                xcp.XcpDaq_Free()
                return
            else:
                daq_list = choose(n0)
                odt = choose(n1)
                if (n0 + n1 + n2) > MAX_DAQ_ENTITIES:
                    assert xcp.XcpDaq_AllocOdtEntry(n0, n1, n2) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
                else:
                    assert xcp.XcpDaq_AllocOdtEntry(n0, n1, n2) == Xcp_ReturnType.ERR_SUCCESS
    xcp.XcpDaq_Free()
