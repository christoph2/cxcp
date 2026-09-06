Integrating an XCP slave
========================

This guide describes the integration contract shared by the maintained
examples. For a ready-to-build target, first follow :doc:`install`.

1. Create the configuration
---------------------------

Place an application-provided ``xcp_config.h`` on the include path. Start with
the closest example configuration rather than writing one from scratch:

* ``examples/pio/include/xcp_config.h`` for Arduino/ESP32;
* ``examples/xcpsim/xcp_config.h`` for a desktop simulator;
* ``examples/debug_on_pico/xcp_config.h`` for the Pico example.

Select exactly one transport with ``XCP_TRANSPORT_LAYER``. Use ``XCP_ON`` and
``XCP_OFF`` for feature switches. Set ``XCP_MAX_CTO`` and ``XCP_MAX_DTO`` to
values valid for that transport, configure the services your master requires,
and size the DAQ entity pool and DTO queue for the application. See
:doc:`options` for the relationships between these settings.

``xcp.h`` includes the configuration header, establishes derived transport
sizes, and rejects unsupported combinations at compile time. Do not define
derived header, counter, or checksum-size macros independently unless the
selected transport contract explicitly requires it.

2. Select the source set
------------------------

Build the protocol core with the subsystem sources it uses:

.. code-block:: text

   src/xcp.c
   src/xcp_checksum.c
   src/xcp_daq.c
   src/xcp_util.c
   src/xcp_timecorr.c                 (when time correlation is configured)
   src/tl/xcp_tl_timeout.c            (when used by the selected adapter)

Then add one matching transport implementation from ``src/tl`` and the
corresponding platform implementation from ``src/hw``. Their implementations
define the same transport entry points, so adapters for different platforms or
transports must not be linked together. Use a maintained example's CMake or
PlatformIO source list as the model for a new target.

3. Initialize and poll
----------------------

Initialize the XCP stack once after the platform services required by the
selected adapter are ready:

.. code-block:: c

   Xcp_Init();

In a cooperative application, service the transport before the core:

.. code-block:: c

   for (;;) {
       XcpTl_MainFunction();
       Xcp_MainFunction();
       /* Run the rest of the application. */
   }

``Xcp_Init()`` initializes the selected transport through ``XcpTl_Init()``.
An application that owns its own receive interrupt or callback should route
received frames through that adapter's established receive path instead of
calling core dispatch functions directly.

4. Define and trigger DAQ events
--------------------------------

DAQ event definitions are provided with the DAQ declaration macros before
application code, as shown in ``examples/pio/src/main.cpp``. Trigger each
event from the cadence represented by its configuration:

.. code-block:: c

   XcpDaq_TriggerEvent(0);

The event number must reference an entry declared in ``XcpDaq_Events``. If
multiple DAQ lists per event are disabled, an event can own only one list.
Dynamic DAQ allocation follows a strict sequence: free DAQ state, allocate
lists, allocate ODTs, then allocate ODT entries. Configure
``XCP_DAQ_MAX_DYNAMIC_ENTITIES`` for the combined number of these objects.

5. Connect a master
-------------------

Configure the XCP master with the transport settings from ``xcp_config.h``:
CAN identifiers and bit timing for CAN, host/port and protocol for Ethernet,
or the selected framing and baud rate for SXI. An A2L file is normally supplied
by the application or its build tooling; CXCP does not generate an A2L file
for arbitrary firmware.

Start with CONNECT, GET_STATUS, GET_ID, and a small upload. Then verify one
DAQ event before enabling a larger measurement configuration. When a command
is not compiled in, the slave cannot provide it regardless of the master's
A2L description.

Common integration failures
---------------------------

* **Linker collisions:** More than one transport or platform adapter was
  compiled. Build one compatible source set only.
* **Configuration errors:** Read the ``#error`` diagnostic emitted by
  ``inc/xcp.h``; it identifies unsupported feature combinations and invalid
  transport sizes.
* **No DAQ DTOs:** Confirm that the event is defined, the master started the
  list, and the application calls ``XcpDaq_TriggerEvent()`` at that event's
  cadence.
* **Unresponsive connection:** Call ``XcpTl_MainFunction()`` and
  ``Xcp_MainFunction()`` regularly, and verify the configured transport
  endpoint and framing.
