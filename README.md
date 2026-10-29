# BlueParrot XCP

![BlueParrot logo](blueparrot.png)

BlueParrot XCP (CXCP) is a configurable C implementation of an XCP slave for
embedded targets and host-side simulation. It implements the protocol core,
data acquisition (DAQ), checksums, optional programming support, and
transport adapters. Applications supply a configuration header, platform
services, and a transport-specific source set.

The project is intended to be integrated into firmware; it is not a
pre-built, general-purpose library with one universal build target.

## Supported integration paths

| Path | Purpose | Build entry point |
| --- | --- | --- |
| `examples/xcpsim` | Desktop simulator with Ethernet; CAN where the required platform support is available | CMake |
| `examples/pio` | ESP32 Arduino example using PlatformIO | PlatformIO |
| `examples/debug_on_pico` | Raspberry Pi Pico USB CDC/SXI example | Pico SDK + CMake |
| `tests` | Python/pybind11 test extension for core and DAQ behavior | CMake + pytest |
| `tools/app_gen` | Independent Arduino-oriented application generator | Python |

Transport adapters are provided for CAN, Ethernet, SXI (serial), and
Bluetooth. Availability depends on the selected platform adapter and target
configuration. XCP on Bluetooth is explicitly marked non-standard in the
public header.

## Repository layout

| Directory | Contents |
| --- | --- |
| `inc` | Public headers; `xcp.h` is the integration and compile-time validation boundary |
| `src` | Protocol core, DAQ, checksum, time-correlation, transport, and hardware adapters |
| `flsemu` | Flash-emulation support used by the simulator and tests |
| `examples` | Supported example applications and build definitions |
| `tests` | pybind11 bindings, mocks, and pytest-based core tests |
| `tools/app_gen` | Separate Python package for generating Arduino-oriented applications |
| `docs` | Sphinx source for the project documentation |

## Quick start

Build the desktop Ethernet simulator from the repository root:

```powershell
cmake -S examples/xcpsim -B build/xcpsim
cmake --build build/xcpsim --config Debug
```

Build the ESP32 PlatformIO example:

```powershell
pio run -d examples/pio
```

Build the Python test extension and run the core tests:

```powershell
python -m pip install -r tests/requirements.txt pybind11
Push-Location tests
python build.py
python -m pytest -svv --hypothesis-show-statistics
Pop-Location
```

## Integrating CXCP

1. Start from the `xcp_config.h` nearest to your target, such as
   `examples/pio/include/xcp_config.h`.
2. Add `inc` and the directory containing your `xcp_config.h` to the include
   path. Select one transport with `XCP_TRANSPORT_LAYER` and use `XCP_ON` or
   `XCP_OFF` for feature switches.
3. Compile the protocol sources and exactly one compatible transport and
   hardware adapter. Do not compile every implementation in `src/tl` or
   `src/hw` into the same target.
4. Call `Xcp_Init()` at startup. In the application loop, call
   `XcpTl_MainFunction()` before `Xcp_MainFunction()`. Trigger configured DAQ
   event channels with `XcpDaq_TriggerEvent()` at the application's intended
   cadence.
5. Implement hooks only when their corresponding configuration option enables
   them. The configuration reference and hook contracts are documented in
   `docs/`.

`inc/xcp.h` validates unsupported combinations at compile time. Treat those
diagnostics as the authoritative compatibility rules for a configured target.

## Documentation

The Sphinx documentation covers installation, integration, configuration,
DAQ, application hooks, and API reference:

```powershell
Push-Location docs
.\make.bat html
Pop-Location
```

See `docs/index.rst` for the rendered documentation structure.

## License

CXCP is licensed under the GNU General Public License, version 2 or later.
See [LICENSE](LICENSE).
