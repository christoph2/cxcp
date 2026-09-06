Public API and source reference
===============================

``inc/xcp.h`` is the public C interface. It includes the application
configuration, defines protocol and transport types, exposes public functions,
and performs the configuration checks that protect supported combinations.

Lifecycle
---------

.. code-block:: c

   void Xcp_Init(void);
   void Xcp_MainFunction(void);
   void Xcp_Disconnect(void);

Call ``Xcp_Init()`` once after target services are ready. Call
``XcpTl_MainFunction()`` and then ``Xcp_MainFunction()`` regularly in a
cooperative application. ``Xcp_Disconnect()`` ends the current XCP session.

Transport API
-------------

The selected transport adapter implements the transport functions declared in
``xcp.h``, including ``XcpTl_Init()``, ``XcpTl_DeInit()``,
``XcpTl_RxHandler()``, ``XcpTl_Send()``, and ``XcpTl_MainFunction()``.
Applications normally use initialization and periodic processing only; source
and hardware integration should use the adapter's established receive path.

DAQ API
-------

``XcpDaq_TriggerEvent(uint8_t eventChannelNumber)`` publishes a configured
DAQ event. It is the primary application-facing DAQ function and must be
called from the represented cyclic or sporadic activity. DAQ declaration
macros, list types, and the remaining DAQ functions are declared in
``inc/xcp.h`` and implemented by ``src/xcp_daq.c``.

Output and utility functions
----------------------------

The core exposes CTO/DTO output helpers such as ``Xcp_GetCtoOutPtr()``,
``Xcp_SetCtoOutLen()``, ``Xcp_SendCto()``, ``Xcp_GetDtoOutPtr()``,
``Xcp_SetDtoOutLen()``, and ``Xcp_SendDto()``. They are intended for core,
transport, and tightly coupled integration code. Their availability follows
the relevant service configuration guards.

``Xcp_SendEventPacket()`` and ``Xcp_SendServiceRequestPacket()`` are available
only when their API switches are enabled. Checksum helpers, memory-transfer
types, byte-order helpers, and hardware/transport declarations are also in
``xcp.h``.

Doxygen
-------

To generate a local API site from the configured Doxygen input:

.. code-block:: console

   Push-Location docs
   doxygen doxyfile
   Pop-Location

This is optional. The maintained Sphinx documentation does not invoke Doxygen
automatically.
