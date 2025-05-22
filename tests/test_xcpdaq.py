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
    MemoryInfo, Xcp_ReturnType, Xcp_GetConnectionState, Xcp_CalculateChecksum,
    xcp_init, xcpdaq_init, xcpdaq_free, xcpdaq_alloc, xcpdaq_alloc_odt,
    xcpdaq_alloc_odt_entry, xcpdaq_set_pointer,
    get_daq_counts, xcpdaq_total_dynamic_entity_count, xcpdaq_queue_init, xcpdaq_write_entry,
    get_dynamic_daq_entities, get_dynamic_daq_entity,
    daq_enqueue, daq_dequeue, get_daq_queue_var, xcpdaq_set_list_mode, xcpdaq_get_first_pid,
    xcpdaq_start_stop_single_list, xcpdaq_start_stop_synch,  xcpdaq_trigger_event,
)

mem = MemoryInfo()
# print("MEM @0", hex(mem.absolute_address(0)))


XCP_DAQ_MAX_DYNAMIC_ENTITIES = 16

daq_entity_int = integers(min_value = 0, max_value = XCP_DAQ_MAX_DYNAMIC_ENTITIES * 2)

def choose(size):
    return random.choice(range(size - 1)) if size > 1 else 0

@pytest.fixture(scope = "function")
def xcp():
    xcp_init()
    xcpdaq_init()
    yield
    xcpdaq_free()

def xcpdaq_get_dynamic_entity_content(num):
    entry = get_dynamic_daq_entity(num)
    return entry

def xcpdaq_check_entries_are_unused(start_idx: int = 0):
    for idx in range(start_idx, xcpdaq_total_dynamic_entity_count()):
        ent = xcpdaq_get_dynamic_entity_content(idx)
        assert ent is None
        #assert ent == (XcpDaq_EntityKindType.XCP_ENTITY_UNUSED, None)

##
## Test xcpdaq_allocTransitionTable
##
@pytest.mark.skip
def test_alloc_seq_err(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_seq_err(xcp):
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_entry_seq_err(xcp):
    assert xcpdaq_alloc_odt_entry(0, 1, 2) == Xcp_ReturnType.ERR_SEQUENCE

def test_alloc_ok(xcp):
    xcpdaq_check_entries_are_unused()
    assert xcpdaq_alloc(5) == Xcp_ReturnType.ERR_SUCCESS
    #ec, _, _ = xcp.get_daq_counts()
    #for idx in range(0, ec):
    #    state, ent = xcp.xcpdaq_total_dynamic_entity_count()
    #    print("ENT", state, ent.numOdts, ent.firstOdt, ent.mode)
    xcpdaq_check_entries_are_unused(5)

def test_allocodt_ok(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_entry_ok(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt_entry(0, 1, 1) == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok1(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok2(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok3(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_free() == Xcp_ReturnType.ERR_SUCCESS

def test_free_ok4(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt_entry(0, 1, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_free() == Xcp_ReturnType.ERR_SUCCESS

def test_alloc_aftera_alloc():
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc(2) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_after_allocodt(xcp):
    assert xcpdaq_alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(1, 3) == Xcp_ReturnType.ERR_SUCCESS

def test_allocodt_entry_after_allocodt_entry(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt_entry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt_entry(0, 1, 2) == Xcp_ReturnType.ERR_SUCCESS

def test_alloc_after_allocodt(xcp):
    assert xcpdaq_alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc(3) == Xcp_ReturnType.ERR_SEQUENCE

def test_alloc_after_allocodt_entry(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt_entry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc(3) == Xcp_ReturnType.ERR_SEQUENCE

def test_allocodt_after_allocodt_entry(xcp):
    assert xcpdaq_alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt_entry(0, 0, 1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(1, 3) == Xcp_ReturnType.ERR_SEQUENCE


##
##  Memory Overflows.
##
def test_alloc_out_of_mem(xcp):
    assert xcpdaq_alloc(101) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

def test_allocodt_out_of_mem(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, XCP_DAQ_MAX_DYNAMIC_ENTITIES) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

def test_allocodt_entry_out_of_mem(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt_entry(0, 1, XCP_DAQ_MAX_DYNAMIC_ENTITIES) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW

##
##
##
def test_basic_alloc1(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert get_daq_counts() == (1, 1, 0)

def test_basic_alloc2(xcp):
    assert xcpdaq_alloc(1) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert get_daq_counts() == (3, 1, 2)

def test_basic_alloc3(xcp):
    assert xcpdaq_alloc(2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt(0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt_entry(0, 0, 2) == Xcp_ReturnType.ERR_SUCCESS
    assert xcpdaq_alloc_odt_entry(0, 1, 3) == Xcp_ReturnType.ERR_SUCCESS
    assert get_daq_counts() == (9, 2, 2)

##
## DAQ queue tests.
##
def test_daq_queue_init(xcp):
    xcpdaq_queue_init()
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
        (5, 5),
        (6, 6),
        (7, 7),
        (8, 8),
        (9, 9),
        (10, 10),
        (11, 11),
        (12, 12),
        (13, 13),
        (14, 14),
        (15, 15),
        (16, 16),
        (0, 0),
        (1, 1),
    )
    xcpdaq_queue_init()
    for idx in range(16 + 1):
        #daq_enqueue(b"test data")
        daq_enqueue(b"test")
        daq_dequeue()
        var = get_daq_queue_var()
        head, tail = RESULTS[idx]
        assert var.head == head
        assert var.tail == tail

def test_daq_queue_empty(xcp):
    xcpdaq_queue_init()
    assert daq_dequeue() == (False, '', )

def test_daq_queue_full(xcp):
    xcpdaq_queue_init()
    for _ in range(16):
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


def test_complex_sequence_1(xcp):
    mem = MemoryInfo()
    BASE_ADDR = mem.absolute_address(0)
    print("MEM @0", hex(BASE_ADDR))
    xcpdaq_free()
    xcpdaq_alloc(1)
    xcpdaq_alloc_odt(0, 5)
    xcpdaq_alloc_odt_entry(0, 0, 1)
    xcpdaq_alloc_odt_entry(0, 1, 1)
    xcpdaq_alloc_odt_entry(0, 2, 1)
    xcpdaq_alloc_odt_entry(0, 3, 1)
    xcpdaq_alloc_odt_entry(0, 4, 1)

    xcpdaq_set_pointer(0, 0, 0)
    xcpdaq_write_entry(255, 4, 0, BASE_ADDR + 0)
    xcpdaq_set_pointer(0, 1, 0)
    xcpdaq_write_entry(255, 4, 0, BASE_ADDR + 4)
    xcpdaq_set_pointer(0, 2, 0)
    xcpdaq_write_entry(255, 4, 0, BASE_ADDR + 8)
    xcpdaq_set_pointer(0, 3, 0)
    xcpdaq_write_entry(255, 4, 0, BASE_ADDR + 12)
    xcpdaq_set_pointer(0, 4, 0)
    xcpdaq_write_entry(255, 4, 0, BASE_ADDR + 16)
    xcpdaq_set_list_mode(0, 0, 0, 1, 0)
    xcpdaq_start_stop_single_list(0, 2)
    xcpdaq_start_stop_synch(1)
    for i in range(10):
        xcpdaq_trigger_event(0)
        print("="*80)
    print("DYN-Entries:", get_dynamic_daq_entities())

##
## Hypothesis tests.
##
@given(num = daq_entity_int)
@settings(max_examples = 1000)
def test_hy_alloc(num):
    if num > XCP_DAQ_MAX_DYNAMIC_ENTITIES:
        assert xcpdaq_alloc(num) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
        count = 0
    else:
        assert xcpdaq_alloc(num) == Xcp_ReturnType.ERR_SUCCESS
        count = num
    ec, lc, _ = get_daq_counts()
    assert ec == count
    assert lc == count
    xcpdaq_free()

#@given(daq_entity_int, daq_entity_int, daq_entity_int, suppress_health_check = True)
#@settings(max_examples = 1000)
#@example(0, 0, 0)
##@settings(verbosity = Verbosity.verbose)
#def test_hy_alloc_odt_entities(xcp, n0, n1, n2):
#    if n0 > XCP_DAQ_MAX_DYNAMIC_ENTITIES:
#        assert xcp.xcpdaq_alloc(n0) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
#    else:
#        assert xcp.xcpdaq_alloc(n0) == Xcp_ReturnType.ERR_SUCCESS
#        if n0 == 0:
#            xcp.xcpdaq_free()
#            return
#        else:
#            daq_list = choose(n0)
#        if (n0 + n1) > XCP_DAQ_MAX_DYNAMIC_ENTITIES:
#            assert xcp.xcpdaq_alloc_odt(daq_list, n1) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
#        else:
#            assert xcp.xcpdaq_alloc_odt(daq_list, n1) == Xcp_ReturnType.ERR_SUCCESS
#            if n1 == 0:
#                xcp.xcpdaq_free()
#                return
#            else:
#                daq_list = choose(n0)
#                odt = choose(n1)
#                if (n0 + n1 + n2) > XCP_DAQ_MAX_DYNAMIC_ENTITIES:
#                    assert xcp.xcpdaq_alloc_odt_entry(n0, n1, n2) == Xcp_ReturnType.ERR_MEMORY_OVERFLOW
#                else:
#                    assert xcp.xcpdaq_alloc_odt_entry(n0, n1, n2) == Xcp_ReturnType.ERR_SUCCESS
#    xcp.xcpdaq_free()
