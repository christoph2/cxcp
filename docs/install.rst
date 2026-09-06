Build supplied targets
======================

CXCP has no single top-level build that combines every source file. Each
example selects the compatible transport and platform sources for one target.
Run the commands below from the repository root.

Desktop simulator
-----------------

``examples/xcpsim`` builds the ``xcp_sim_eth`` desktop simulator on Windows,
Linux, and macOS. It selects the native Ethernet and hardware adapters.

.. code-block:: console

   cmake -S examples/xcpsim -B build/xcpsim
   cmake --build build/xcpsim --config Debug

On Unix-like systems, the CMake project can also build ``xcp_sim_can`` when
SocketCAN is enabled (the default except on macOS). On Windows, the CAN target
is added only when the ``KVASER_CANLIB`` environment variable points to a
Kvaser CANlib installation.

ESP32 PlatformIO example
------------------------

``examples/pio`` is an Arduino/ESP32 project. The default PlatformIO
environment is ``esp32doit-devkit-v1`` and builds the SXI serial transport.
The Bluetooth environment is named ``esp32doit-devkit-v1-bth``.

.. code-block:: console

   pio run -d examples/pio
   pio run -d examples/pio -e esp32doit-devkit-v1-bth

The project configuration and its source filter deliberately list the core,
transport, and Arduino hardware sources. Keep that selection aligned when
changing the configured transport.

Raspberry Pi Pico example
-------------------------

``examples/debug_on_pico`` requires ``PICO_SDK_PATH`` to reference an
installed Pico SDK, a compatible ARM toolchain, CMake, and network access
during configuration: its CMake project fetches ``pico-mcp2515``.

.. code-block:: console

   cmake -S examples/debug_on_pico -B build/pico
   cmake --build build/pico --config Release

Core test extension
-------------------

The core tests build a pybind11 extension from selected production sources and
test-only mocks. Python development headers, CMake, a C/C++ compiler, and
pybind11 are required.

.. code-block:: console

   python -m pip install -r tests/requirements.txt pybind11
   Push-Location tests
   python build.py
   python -m pytest -svv --hypothesis-show-statistics
   Pop-Location

After the extension has been built, run one DAQ test with:

.. code-block:: console

   Push-Location tests
   python -m pytest test_xcpdaq.py -k test_allocodt_ok -vv
   Pop-Location

Application generator
---------------------

``tools/app_gen`` is an independent Python package. Its tests do not require
the core test extension.

.. code-block:: console

   python -m pip install -e tools/app_gen
   Push-Location tools/app_gen
   python -m pytest tests/test_utils.py -q
   Pop-Location

Building this documentation
---------------------------

Sphinx builds the documentation source in this directory. On Windows:

.. code-block:: console

   Push-Location docs
   .\make.bat html
   Pop-Location

The API page links to declarations in ``inc/xcp.h``. A separately generated
Doxygen site is optional and is not required to build the Sphinx manual.
