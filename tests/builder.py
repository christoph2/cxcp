#!/usr/bin/env python
# -*- coding: utf-8 -*-

from glob import glob
import os
import subprocess

CFLAGS_OBJ = "-Wall -g -std=c11 -O3 -fPIC -shared -c -I../inc -I."
CFLAGS_SO = "-shared"

class Builder:

    def clean(self):
        p0 = glob("*.o")
        p1 = glob("*.so")
        philes = p0 + p1
        for ph in philes:
            res = os.unlink(ph)

    def run(self, cmd, *args):
        line = cmd + " " + " ".join(args)
        print("CMD:", line)
        return subprocess.check_call(line, shell = True)

    def build_objs(self, *objs):
        for obj in objs:
            print(obj)
            self.run("gcc", CFLAGS_OBJ, obj, "-ftest-coverage -fprofile-arcs")


    def build_so(self, so, *objs):
        self.run("gcc", CFLAGS_SO, " ".join(objs), "-o {}".format(so), "-ftest-coverage -fprofile-arcs")


def main():
    builder = Builder()
    builder.clean()
    #print(builder.run("ls", "-l", "-S", "-R", "*py"))
    builder.build_objs("checksum_mocks.c", "xcp_init.c", "../src/xcp_checksum.c", "../src/xcp_daq.c", "../src/xcp_util.c")
    builder.build_so("test_cs.so", "checksum_mocks.o", "xcp_checksum.o")
    builder.build_so("test_daq.so", "xcp_daq.o", "xcp_util.o", "xcp_init.o")

if __name__ == '__main__':
    main()

