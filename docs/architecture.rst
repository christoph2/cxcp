Architecture overview
=====================

CXCP is designed as a configurable protocol core with explicit transport and
platform abstraction boundaries. A target selects one compatible source set and
provides one application configuration header.

Layer model
-----------

.. code-block:: text

   Application code
      |
      +-- xcp_config.h (feature switches, limits, transport selection)
      |
   CXCP protocol core (src/xcp.c, src/xcp_daq.c, src/xcp_checksum.c, ...)
      |
      +-- Transport adapter (src/tl/<transport>/...)
      |
      +-- Platform adapter  (src/hw/<platform>/...)

Integration boundaries
----------------------

* ``inc/xcp.h`` is the public integration boundary.
* The application must provide ``xcp_config.h`` on the include path.
* Exactly one transport contract must be selected via
  ``XCP_TRANSPORT_LAYER``.
* Compile-time checks in ``xcp.h`` reject unsupported combinations early.

Runtime flow
------------

The protocol is driven cooperatively:

1. ``Xcp_Init()`` initializes core state and transport glue.
2. ``XcpTl_MainFunction()`` processes transport-level traffic.
3. ``Xcp_MainFunction()`` dispatches protocol commands and responses.
4. ``XcpDaq_TriggerEvent(event)`` emits DAQ data for configured events.

This ordering is required for predictable command and DAQ behavior.

DAQ model
---------

CXCP DAQ allocation is sequence-sensitive and uses a shared entity pool.
Dynamic allocation follows this state progression:

.. code-block:: text

   FreeDaq -> Allocate DAQ lists -> Allocate ODTs -> Allocate ODT entries

When one-list-per-event mode is configured, each event can own only one DAQ
list. Configure entity limits accordingly.

Build strategy
--------------

Do not compile all sources in ``src/tl`` or ``src/hw`` into one target.
Transport and platform adapters implement overlapping symbols and are meant to
be selected per target.

Use the maintained source-set examples as build templates:

* ``examples/xcpsim`` for desktop simulation.
* ``examples/pio`` for ESP32 Arduino targets.
* ``examples/debug_on_pico`` for Pico SDK targets.
