#!/usr/bin/env python

from pathlib import Path
from glob import glob
import shutil
from subprocess import call
import sys

WIN = sys.platform == "win32"

if WIN:
    EXT = "pyd"
else:
    EXT = "so"

CACHE = Path("CMakeCache.txt")

if not CACHE.exists():
    call(["cmake", "."])

call(["cmake", "--build", "."])

philez = glob(f"./Debug/*{EXT}")  # TODO: Debug may only be valid on Windows.

for phile in philez:
    print(f"copying {phile}")
    shutil.copy(phile, ".")
