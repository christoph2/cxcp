Getting started in 10 minutes
=============================

This quick start validates a working end-to-end path with the desktop Ethernet
simulator. It is the fastest way to verify your local toolchain and understand
CXCP runtime behavior before integrating into firmware.

What you will do
----------------

1. Build ``examples/xcpsim``.
2. Start the simulator executable.
3. Confirm that the XCP stack is initialized and servicing its runtime loop.

Step 1: Build the desktop simulator
-----------------------------------

From the repository root:

.. code-block:: console

   cmake -S examples/xcpsim -B build/xcpsim
   cmake --build build/xcpsim --config Debug

On Windows with Visual Studio generators, the binary is typically created under
``build/xcpsim/Debug`` (or equivalent configuration output path).

Step 2: Run the simulator
-------------------------

Run the produced executable for your build configuration. Typical examples:

.. code-block:: console

   .\build\xcpsim\Debug\xcp_sim_eth.exe

or, when using a single-config generator:

.. code-block:: console

   .\build\xcpsim\xcp_sim_eth.exe

The simulator initializes flash emulation, the transport adapter, and the XCP
core, then enters the runtime processing path.

Step 3: Validate runtime integration points
-------------------------------------------

Use this checklist to validate your own target integration against the simulator
behavior:

* ``Xcp_Init()`` is called once after platform services are ready.
* The runtime loop services transport before the protocol core:

  .. code-block:: c

     XcpTl_MainFunction();
     Xcp_MainFunction();

* Application events trigger DAQ channels at deterministic cadence:

  .. code-block:: c

     XcpDaq_TriggerEvent(0);

Minimal integration skeleton
----------------------------

Use this pattern when porting to a new target:

.. code-block:: c

   #include "xcp.h"

   int main(void) {
       Xcp_Init();

       for (;;) {
           XcpTl_MainFunction();
           Xcp_MainFunction();
           /* Application logic */
       }
   }

Next steps
----------

* Continue with :doc:`install` for full build and test paths.
* Use :doc:`tutorial` for custom integration details.
* See :doc:`integration_examples` for concrete platform-level deployments.
