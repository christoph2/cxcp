Integration examples
====================

This page provides practical deployment patterns based on maintained examples.
Each example highlights the transport choice, build command, and runtime
integration shape.

Example 1: Desktop Ethernet simulator
-------------------------------------

Source path: ``examples/xcpsim``

Use this path to validate host-side tooling, protocol command flow, and DAQ
behavior without embedded hardware.

Build:

.. code-block:: console

   cmake -S examples/xcpsim -B build/xcpsim
   cmake --build build/xcpsim --config Debug

Runtime entry point: ``examples/xcpsim/main.c``

Relevant lifecycle in this example:

.. code-block:: c

   FlsEmu_Init(&FlsEmu_Config);
   Xcp_Init();
   Xcp_DisplayInfo();
   XcpThrd_RunThreads();

Example 2: ESP32 with PlatformIO (SXI or Bluetooth)
----------------------------------------------------

Source path: ``examples/pio``

The ``platformio.ini`` file defines two target environments:

* ``esp32doit-devkit-v1`` (SXI serial)
* ``esp32doit-devkit-v1-bth`` (Bluetooth)

Build both variants:

.. code-block:: console

   pio run -d examples/pio
   pio run -d examples/pio -e esp32doit-devkit-v1-bth

Runtime loop in ``examples/pio/src/main.cpp`` follows the expected cooperative
order:

.. code-block:: c

   XcpTl_MainFunction();
   Xcp_MainFunction();

DAQ trigger in the same example:

.. code-block:: c

   XcpDaq_TriggerEvent(0);

Example 3: Raspberry Pi Pico target
-----------------------------------

Source path: ``examples/debug_on_pico``

This target demonstrates Pico SDK integration and USB CDC / SXI-oriented setup.

Build:

.. code-block:: console

   cmake -S examples/debug_on_pico -B build/pico
   cmake --build build/pico --config Release

Main loop in ``examples/debug_on_pico/main.c``:

.. code-block:: c

   while (1) {
       XcpTl_MainFunction();
       Xcp_MainFunction();
       XcpDaq_TriggerEvent(0);
   }

Example 4: Core behavior validation via Python tests
----------------------------------------------------

Source path: ``tests``

This is the recommended regression path for protocol core and DAQ logic.

Build and test:

.. code-block:: console

   python -m pip install -r tests/requirements.txt pybind11
   Push-Location tests
   python build.py
   python -m pytest -svv --hypothesis-show-statistics
   Pop-Location

Run one focused DAQ allocation test:

.. code-block:: console

   Push-Location tests
   python -m pytest test_xcpdaq.py -k test_allocodt_ok -vv
   Pop-Location

Selecting the right example
---------------------------

* Choose ``examples/xcpsim`` when you need the shortest local feedback loop.
* Choose ``examples/pio`` for Arduino/ESP32 serial or Bluetooth integration.
* Choose ``examples/debug_on_pico`` for Pico SDK-based embedded targets.
* Choose ``tests`` when validating protocol behavior before deployment.
