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
    Verbosity, HealthCheck
)
from hypothesis.strategies import (
    booleans, integers, lists, text, characters, builds, composite, tuples, randoms
)

# from xcp_if import XCP

from cxcp import (
    Xcp_Init, XcpDaq_Init, XcpDaq_Free, XcpDaq_Alloc, XcpDaq_AllocOdt, Xcp_ReturnType,
    XcpDaq_AllocOdtEntry, Xcp_GetConnectionState, Xcp_CalculateChecksum,
    get_daq_counts, XcpDaq_TotalDynamicEntityCount, XcpDaq_QueueInit,
    XcpDaq_GetDynamicDaqEntityCount, get_dynamic_daq_entities, get_dynamic_daq_entity,
    daq_enqueue, daq_dequeue, get_daq_queue_var
)


XCP_DAQ_MAX_DYNAMIC_ENTITIES = 16

daq_entity_int = integers(min_value = 0, max_value = XCP_DAQ_MAX_DYNAMIC_ENTITIES * 2)

def choose(size):
    return random.choice(range(size - 1)) if size > 1 else 0

@pytest.fixture(scope = "function")
def xcp():
    Xcp_Init()
    XcpDaq_Init()
    yield
    XcpDaq_Free()

def xcpdaq_get_dynamic_entity_content(num):
    entry = get_dynamic_daq_entity(num)
    return entry

def xcpdaq_check_entries_are_unused(start_idx: int = 0):
    for idx in range(start_idx, XcpDaq_TotalDynamicEntityCount()):
        ent = xcpdaq_get_dynamic_entity_content(idx)
        assert ent is None
        #assert ent == (XcpDaq_EntityKindType.XCP_ENTITY_UNUSED, None)

##
## Test XcpDaq_AllocTransitionTable
##
@pytest.mark.skip
def test_alloc_seq_err(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_seq_err(xcp):
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_entry_seq_err(xcp):
    assert XcpDaq_AllocOdtEntry(0, 1, 2) == Xcp_ReturnType.ERR_SEQUENCE

def test_alloc_ok(xcp):
    xcpdaq_check_entries_are_unused()
    assert XcpDaq_Alloc(5) == Xcp_ReturnType.ERR_SUCCESS
    #ec, _, _ = xcp.get_daq_counts()
    #for idx in range(0, ec):
    #    state, ent = xcp.XcpDaq_TotalDynamicEntityCount()
    #    print("ENT", state, ent.numOdts, ent.firstOdt, ent.mode)
    xcpdaq_check_entries_are_unused(5)

def test_allocodt_ok(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_entry_ok(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdtEntry(0, 1, 1) == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok1(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_Free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok2(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_Free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok3(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_Free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok4(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdtEntry(0, 1, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_Free() == Xcp_ReturnType.ERR_SUCCESS

def test_alloc_aftera_alloc():
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_Alloc(2) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_after_allocodt(xcp):
    assert XcpDaq_Alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(1, 3) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_entry_after_allocodt_entry(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdtEntry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdtEntry(0, 1, 2) == Xcp_ReturnType.ERR_SUCCESS

def test_alloc_after_allocodt(xcp):
    assert XcpDaq_Alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_Alloc(3) == Xcp_ReturnType.ERR_SEQUENCE

def test_alloc_after_allocodt_entry(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdtEntry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_Alloc(3) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_after_allocodt_entry(xcp):
    assert XcpDaq_Alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdtEntry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(1, 3) == Xcp_ReturnType.ERR_SEQUENCE


##
##  Memory Overflows.
##
def test_alloc_out_of_mem(xcp):
    assert XcpDaq_Alloc(101) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

def test_allocodt_out_of_mem(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, XCP_DAQ_MAX_DYNAMIC_ENTITIES) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

def test_allocodt_entry_out_of_mem(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdtEntry(0, 1, XCP_DAQ_MAX_DYNAMIC_ENTITIES) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

##
##
##
def test_basic_alloc1(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert get_daq_counts() == (1, 1, 0)

def test_basic_alloc2(xcp):
    assert XcpDaq_Alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert get_daq_counts() == (3, 1, 2)

def test_basic_alloc3(xcp):
    assert XcpDaq_Alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdtEntry(0, 0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert XcpDaq_AllocOdtEntry(0, 1, 3) == Xcp_ReturnType.ERR_SUCCESS
    assert get_daq_counts() == (9, 2, 2)

##
## DAQ queue tests.
##
def test_daq_queue_init(xcp):
    XcpDaq_QueueInit()
    var = get_daq_queue_var()
    print("init", var.head, var.tail)
    assert var.head == 0
    assert var.tail == 0

def test_daq_queue_enq_deq_with_wraparound(xcp):
    RESULTS = (
        (1, 1),
        (2, 2),
        (3, 3),
        (4, 4),
        (0, 0),
        (1, 1),
    )
    XcpDaq_QueueInit()
    for idx in range(6):
        #daq_enqueue(b"test data")
        daq_enqueue(b"test")
        daq_dequeue()
        var = get_daq_queue_var()
        head, tail = RESULTS[idx]
        assert var.head == head
        assert var.tail == tail

def test_daq_queue_empty(xcp):
    XcpDaq_QueueInit()
    assert daq_dequeue() == (False, '', )

def test_daq_queue_full(xcp):
    XcpDaq_QueueInit()
    for _ in range(4):
        assert daq_enqueue(b"test") == True    # b"test data"
    assert daq_enqueue(b"test") == False

def test_daq_queue_push_pop_seq(xcp):
    for idx in range(4):
        data = str(chr(ord('0') + idx) * 8)
        daq_enqueue(data)
    for idx in range(4):
        res, data = daq_dequeue()
        assert res == True
        assert data == str(chr(ord('0') + idx) * 8)


##
## Hypothesis tests.
##
@given(num = daq_entity_int)
@settings(max_examples = 1000)
def test_hy_alloc(num):
    if num > XCP_DAQ_MAX_DYNAMIC_ENTITIES:
        assert XcpDaq_Alloc(num) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
        count = 0
    else:
        assert XcpDaq_Alloc(num) == Xcp_ReturnType.ERR_SUCCESS
        count = num
    ec, lc, _ = get_daq_counts()
    assert ec == count
    assert lc == count
    XcpDaq_Free()

#@given(daq_entity_int, daq_entity_int, daq_entity_int, suppress_health_check = True)
#@settings(max_examples = 1000)
#@example(0, 0, 0)
##@settings(verbosity = Verbosity.verbose)
#def test_hy_alloc_odt_entities(xcp, n0, n1, n2):
#    if n0 > XCP_DAQ_MAX_DYNAMIC_ENTITIES:
#        assert xcp.XcpDaq_Alloc(n0) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
#    else:
#        assert xcp.XcpDaq_Alloc(n0) == Xcp_ReturnType.ERR_SUCCESS
#        if n0 == 0:
#            xcp.XcpDaq_Free()
#            return
#        else:
#            daq_list = choose(n0)
#        if (n0 + n1) > XCP_DAQ_MAX_DYNAMIC_ENTITIES:
#            assert xcp.XcpDaq_AllocOdt(daq_list, n1) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
#        else:
#            assert xcp.XcpDaq_AllocOdt(daq_list, n1) == Xcp_ReturnType.ERR_SUCCESS
#            if n1 == 0:
#                xcp.XcpDaq_Free()
#                return
#            else:
#                daq_list = choose(n0)
#                odt = choose(n1)
#                if (n0 + n1 + n2) > XCP_DAQ_MAX_DYNAMIC_ENTITIES:
#                    assert xcp.XcpDaq_AllocOdtEntry(n0, n1, n2) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
#                else:
#                    assert xcp.XcpDaq_AllocOdtEntry(n0, n1, n2) == Xcp_ReturnType.ERR_SUCCESS
#    xcp.XcpDaq_Free()
