Build and test targets
======================

CXCP is integrated per target. There is no single monolithic build that links
all transport and hardware implementations together. Build commands below are
executed from the repository root.

Prerequisites
-------------

.. list-table::
   :header-rows: 1

   * - Component
     - Used by
     - Notes
   * - CMake + C/C++ toolchain
     - ``examples/xcpsim``, ``examples/debug_on_pico``, ``tests``
     - Required for simulator, Pico build, and pybind extension build.
   * - Python 3 + pip
     - ``tests``, ``tools/app_gen``
     - Install test dependencies from ``tests/requirements.txt``.
   * - pybind11
     - ``tests``
     - Used by the test extension wrapped around core sources.
   * - PlatformIO
     - ``examples/pio``
     - Required only for ESP32 Arduino builds.
   * - Pico SDK
     - ``examples/debug_on_pico``
     - Set ``PICO_SDK_PATH`` before configuration.

Desktop simulator (Ethernet)
----------------------------

Build the maintained desktop simulator target from ``examples/xcpsim``:

.. code-block:: console

   cmake -S examples/xcpsim -B build/xcpsim
   cmake --build build/xcpsim --config Debug

Result: ``xcp_sim_eth`` in the selected build configuration.

Notes:

* On Linux, ``xcp_sim_can`` is usually enabled via SocketCAN.
* On Windows, CAN support is available only when ``KVASER_CANLIB`` points to a
  Kvaser CANlib installation.

ESP32 PlatformIO example
------------------------

Build the ESP32 Arduino integration in ``examples/pio``:

.. code-block:: console

   pio run -d examples/pio
   pio run -d examples/pio -e esp32doit-devkit-v1-bth

The default environment uses SXI serial transport. The ``-bth`` environment
switches to the Bluetooth transport source set.

Raspberry Pi Pico example
-------------------------

Configure and build the Pico target:

.. code-block:: console

   cmake -S examples/debug_on_pico -B build/pico
   cmake --build build/pico --config Release

The project fetches ``pico-mcp2515`` at configure time and therefore needs
network access on first configuration.

Core tests (pybind11 extension)
-------------------------------

Build the extension and run the core test suite:

.. code-block:: console

   python -m pip install -r tests/requirements.txt pybind11
   Push-Location tests
   python build.py
   python -m pytest -svv --hypothesis-show-statistics
   Pop-Location

Run a single DAQ allocation test after the extension is built:

.. code-block:: console

   Push-Location tests
   python -m pytest test_xcpdaq.py -k test_allocodt_ok -vv
   Pop-Location

Application generator package
-----------------------------

``tools/app_gen`` is a separate Python package with its own tests:

.. code-block:: console

   python -m pip install -e tools/app_gen
   Push-Location tools/app_gen
   python -m pytest tests/test_utils.py -q
   Pop-Location

Build this documentation
------------------------

Build the Sphinx HTML site:

.. code-block:: console

   Push-Location docs
   .\make.bat html
   Pop-Location

Output is written to ``docs/build/html``. The optional Doxygen configuration in
``docs/doxyfile`` is not part of the Sphinx build pipeline.
