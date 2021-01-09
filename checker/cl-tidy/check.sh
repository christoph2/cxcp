#!/bin/sh
clang-tidy --config='' ../../flsemu/posix/flsemu.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../flsemu/common.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../src/xcp_checksum.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../src/hw/linux/tui.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../src/hw/linux/hw.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../src/xcp.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../src/xcp_util.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../src/xcp_daq.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../src/tl/can/linux_socket_can.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../src/tl/can/socket_can.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../src/tl/sxi/xcp_tl.c -- -I../../inc -I../../flsemu -I. -DETHER
clang-tidy --config='' ../../src/tl/eth/linuxeth.c -- -I../../inc -I../../flsemu -I. -DETHER
