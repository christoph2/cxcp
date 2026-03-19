#!/usr/bin/env python

import sys
import time

from pyxcp.cmdline import ArgumentParser
from pyxcp.daq_stim import DaqList, DaqRecorder, DaqToCsv  # noqa: F401
from pyxcp.types import XcpTimeoutError

ap = ArgumentParser(description="DAQ test")

DAQ_LISTS = [
    DaqList(
        name="part_1",
        event_num=0,
        stim=False,
        enable_timestamps=False,
        measurements=[
            ("byteCounter", 0x00023648, 0, "U8"),
            ("wordCounter", 0x0002364C, 0, "U16"),
            ("dwordCounter", 0x00023650, 0, "U32"),
            ("sbyteCounter", 0x00023649, 0, "I8"),
        ],
        priority=0,
        prescaler=1,
    ),
    DaqList(
        name="part_2",
        event_num=7,
        stim=False,
        enable_timestamps=False,
        measurements=[
            ("swordCounter", 0x00023654, 0, "I16"),
            ("sdwordCounter", 0x00023658, 0, "I32"),
            ("channel1", 0x00023630, 0, "F64"),
            ("channel2", 0x00023638, 0, "F64"),
            ("channel3", 0x00023640, 0, "F64"),
        ],
        priority=0,
        prescaler=1,
    ),
]

daq_parser = DaqRecorder(DAQ_LISTS, "run_daq_21092025_01", 8)  # Record to ".xmraw" file.

with ap.run(policy=daq_parser) as x:
    try:
        x.connect()
    except XcpTimeoutError:
        print("TO")
        sys.exit(2)

    if x.slaveProperties.optionalCommMode:
        x.getCommModeInfo()

    x.cond_unlock("DAQ")  # DAQ resource is locked in many cases.

    print("setup DAQ lists.")
    daq_parser.setup()  # Execute setup procedures.
    print("start DAQ lists.")
    daq_parser.start()  # Start DAQ lists.

    time.sleep(0.25 * 60.0 * 60.0)  # Run for 15 minutes.

    print("Stop DAQ....")
    daq_parser.stop()  # Stop DAQ lists.
    print("finalize DAQ lists.\n")
    x.disconnect()

if hasattr(daq_parser, "files"):  # `files` attribute is specific to `DaqToCsv`.
    print("Data written to:")
    print("================")
    for fl in daq_parser.files.values():
        print(fl.name)
