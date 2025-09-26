#!/usr/bin/env python
# -*- coding: utf-8 -*-

from collections import defaultdict
import enum
import graphlib     # requires Python >= 3.9
import pathlib
from pprint import pprint
import re




START = re.compile(r"/\*!!!\s+START-INCLUDE-SECTION\s+!!!\*/")
END = re.compile(r"/\*!!!\s+END-INCLUDE-SECTION\s+!!!\*/")

INCLUDE = re.compile(r'#include\s+"([^"]*)"')

class StateType(enum.IntEnum):
    IDLE = 0
    COLLECTING = 1

def builder(files, stoplist = None):
    source_includes = defaultdict(list)
    source_files = defaultdict(list)
    state = StateType.IDLE
    stoplist = stoplist or []
    for fn in files:
        state = StateType.IDLE
        if str(fn) in stoplist:
            continue
        with open(fn) as source:
            #source_includes[str(fn)] = []
            source_includes[fn] = []
            for idx, line in enumerate(source, 1):
                if state == StateType.IDLE:
                    match = START.match(line.strip())
                    if match:
                        state = StateType.COLLECTING
                    else:
                        source_files[fn].append(line)
                if state == StateType.COLLECTING:
                    match = END.match(line.strip())
                    if match:
                        state = StateType.IDLE
                    else:
                        line = line.strip()
                        if not line:
                            continue
                        match = INCLUDE.match(line.strip())
                        if match:
                            incf = match.group(1)
                            source_includes[fn].append(incf)
    for k,v in source_files.items():
        source_files[k] = "".join(v)
    return source_includes, source_files

def convert_to_paths(file_name_dict: dict):
    result = {}
    for k, v in file_name_dict.items():
        items = []
        for item in v:
            item = pathlib.Path(r"../inc") / pathlib.Path(item)
            items.append(item)
        result[k] = items
    return result

def print_file_name(out_file, item):
    out_file.write("\n")
    out_file.write("/" * 80)
    out_file.write("\n")
    fname = "//" + f"{item.name}".center(76) + "//"
    out_file.write(fname)
    out_file.write("\n")
    out_file.write("/" * 80)
    out_file.write("\n\n")


STOP_LIST = [   # don't include these files.
    r"..\inc\queue.h",
    r'..\src\hw\options.c',
    r'..\src\hw\terminal.c',
    r'..\src\hw\threads.c',
    r'..\src\hw\linux\hw.c',
    r'..\src\hw\macos\hw.c',
    r'..\src\hw\posix\hw.c',
    r'..\src\hw\win\hw.c',
    r'..\src\tl\bt\winbt.c',
    r'..\src\tl\can\kvaser.c',
    r'..\src\tl\can\linux_socket_can.c',
    r'..\src\tl\eth\common.c',
    r'..\src\tl\eth\linuxeth.c',
    r'..\src\tl\eth\wineth.c',
    r'..\src\tl\eth\wineth_iocp.c',
]

pth = pathlib.Path("../src")
SOURCES = list(pth.glob('**/*.c')) + list(pth.glob('**/*.cpp'))
INCS = list(pathlib.Path("../inc").glob("**/*.h"))
source_includes, source_files = builder(INCS, stoplist = [r"..\inc\xcp_ow.h", r'..\inc\xcp_terminal.h', r'..\inc\xcp_threads.h', r'..\inc\xcp_tui.h'])

ts = graphlib.TopologicalSorter(convert_to_paths(source_includes))
static_ordered_files = tuple(ts.static_order())
with open("xcp.h", "wt") as sf:
    for item in static_ordered_files:
        print("ITEM", item)
        print_file_name(sf, item)
        if item in source_files:
            sf.write(source_files[item])
        else:
            # raise RuntimeError("Something went wrong", item, source_files.keys())
            print("Something went wrong", item, source_files.keys())
    source_includes, source_files = builder(SOURCES, STOP_LIST)
    for k, v in source_files.items():
        print_file_name(sf, k)
        sf.write(v)

print("\n", "=" * 80, "\n")
