#!/bin/sh
clang-format -i --style=file:../.clang-format arduino/hello_xcp/*.ino
clang-format -i --style=file:../.clang-format arduino/hello_xcp/*.h
clang-format -i --style=file:../.clang-format *.h
