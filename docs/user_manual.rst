User guide
==========

BlueParrot XCP is embedded into an application rather than installed as a
single binary library. A target selects one transport, supplies
``xcp_config.h``, and links the protocol core with matching transport and
hardware adapters.

Start here
----------

1. Build one maintained example using :doc:`install`.
2. Use :doc:`tutorial` to create or adapt an application integration.
3. Configure the protocol and resource limits through :doc:`options`.
4. Implement the application policy hooks described in
   :doc:`hook_functions` only when their switches are enabled.

Operational model
-----------------

An XCP master connects through the selected transport and issues protocol
commands. The core processes those commands and serves enabled upload,
calibration, DAQ, paging, checksum, or programming services. The application
continues to own its hardware and scheduling model.

After ``Xcp_Init()``, a cooperative target must call
``XcpTl_MainFunction()`` before ``Xcp_MainFunction()`` regularly. The
application triggers configured DAQ events using
``XcpDaq_TriggerEvent(eventNumber)``. The event number and cadence must agree
with the DAQ event declaration and A2L description used by the XCP master.

Configuration principles
------------------------

Feature switches control the commands compiled into the target. A master
cannot use a command omitted from ``xcp_config.h``. The public header validates
key feature dependencies and transport constraints while compiling the target.

DAQ resources are bounded at build time. In dynamic mode, the single entity
pool contains DAQ lists, ODTs, and ODT entries. Allocate those in the protocol
sequence and configure sufficient total capacity. The current implementation
supports one DAQ list per event.

Security and memory access
--------------------------

For production targets, restrict measurement, calibration, and programming
access according to the product's memory map and security policy. The optional
memory-access, address-mapping, and seed/key hooks are application
responsibilities. Simulator hooks are illustrative only and are not a
production security design.
