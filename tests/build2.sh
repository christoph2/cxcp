#!/bin/sh
gcc -Wall -O3 -fPIC -shared -c -I../inc -I. checksum_mocks.c
gcc -Wall -O3 -fPIC -shared -c -I../inc -I. ../src/xcp_checksum.c
gcc -shared checksum_mocks.o xcp_checksum.o -o test_cs.so
