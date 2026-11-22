#!/usr/bin/env python

import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import sysconfig


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path, default=Path(__file__).parent / "build")
    parser.add_argument("--output-dir", type=Path, default=Path(__file__).parent)
    parser.add_argument("--sanitizers", action="store_true")
    args = parser.parse_args()

    tests_dir = Path(__file__).resolve().parent
    build_dir = args.build_dir.resolve()
    output_dir = args.output_dir.resolve()
    pybind11_dir = subprocess.check_output(
        [sys.executable, "-m", "pybind11", "--cmakedir"], text=True
    ).strip()

    configure = [
        "cmake",
        "-S",
        str(tests_dir),
        "-B",
        str(build_dir),
        "-D",
        f"Python3_EXECUTABLE={sys.executable}",
        "-D",
        "CMAKE_BUILD_TYPE=Debug",
        "-D",
        f"pybind11_DIR={pybind11_dir}",
        "-D",
        f"CXCP_ENABLE_SANITIZERS={'ON' if args.sanitizers else 'OFF'}",
    ]

    subprocess.run(configure, check=True)
    subprocess.run(
        ["cmake", "--build", str(build_dir), "--config", "Debug", "--parallel"],
        check=True,
    )

    extension_suffix = sysconfig.get_config_var("EXT_SUFFIX")
    extensions = [path for path in build_dir.rglob(f"cxcp*{extension_suffix}") if path.is_file()]
    if not extensions:
        raise RuntimeError(f"No Python extension modules found under {build_dir}")

    output_dir.mkdir(parents=True, exist_ok=True)
    for extension in extensions:
        print(f"copying {extension}")
        shutil.copy2(extension, output_dir / extension.name)


if __name__ == "__main__":
    main()
