Configuration reference
=======================

Every CXCP target must provide ``xcp_config.h`` on its include path before
``xcp.h`` is included. The header selects protocol capabilities and establishes
the resource limits for one firmware image. Copy the configuration nearest to
your target and modify it incrementally.

Use ``XCP_ON`` and ``XCP_OFF`` for Boolean switches. The complete definitions,
defaults, and compile-time compatibility checks live in ``inc/xcp.h``; that
header is authoritative when this reference and a diagnostic differ.

Core identity and behavior
--------------------------

``XCP_GET_ID_0`` and ``XCP_GET_ID_1`` provide the standard GET_ID responses.
``XCP_BUILD_TYPE`` selects debug or release behavior, while
``XCP_ENABLE_EXTERN_C_GUARDS`` enables C linkage when the header is consumed
from C++. Set ``XCP_BYTE_ORDER`` and ``XCP_ADDRESS_GRANULARITY`` to describe
the target representation.

``XCP_CHECKSUM_METHOD`` selects the checksum algorithm. When
``XCP_CHECKSUM_CHUNKED_CALCULATION`` is enabled, the periodic core processing
continues a checksum in chunks of ``XCP_CHECKSUM_CHUNK_SIZE``; use
``XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE`` to reject excessive requests (zero means
unlimited). ``XCP_MAIN_FUNCTION_PERIOD`` documents the intended call period of
the application main function.

Resource protection starts with the per-resource state selected by
``XCP_PROTECT_CAL``, ``XCP_PROTECT_PAG``, ``XCP_PROTECT_DAQ``,
``XCP_PROTECT_STIM``, and ``XCP_PROTECT_PGM``. Seed/key support requires both
``XCP_ENABLE_GET_SEED`` and ``XCP_ENABLE_UNLOCK``.

Optional command groups
-----------------------

Enable only commands that the application needs:

* general services: ``XCP_ENABLE_GET_VERSION``,
  ``XCP_ENABLE_GET_COMM_MODE_INFO``, ``XCP_ENABLE_GET_ID``,
  ``XCP_ENABLE_SET_MTA``, upload, checksum, seed/key, and user-command
  switches;
* calibration: ``XCP_ENABLE_CAL_COMMANDS`` plus DOWNLOAD, DOWNLOAD_MAX,
  SHORT_DOWNLOAD, DOWNLOAD_NEXT, and MODIFY_BITS switches;
* paging: ``XCP_ENABLE_PAG_COMMANDS`` plus its page and segment query/control
  switches;
* DAQ: ``XCP_ENABLE_DAQ_COMMANDS`` plus DAQ query and allocation switches;
* programming: ``XCP_ENABLE_PGM_COMMANDS`` plus its processor, sector, and
  program-transfer switches.

Several options have dependencies. For example, master block mode requires
DOWNLOAD_NEXT. Unsupported features such as STIM, DAQ bit offsets,
prioritization, alternation, address extension, and multiple DAQ lists per
event are rejected by the current header when enabled. Do not use an A2L file
to advertise commands excluded by the firmware configuration.

Transport selection
-------------------

Set ``XCP_TRANSPORT_LAYER`` to exactly one of the following values:

====================  ==============================================
Value                 Transport
====================  ==============================================
``XCP_ON_CAN``        XCP on Classic CAN
``XCP_ON_ETHERNET``   XCP on Ethernet
``XCP_ON_SXI``        XCP on SXI serial framing
``XCP_ON_BTH``        Bluetooth adapter (non-standard extension)
====================  ==============================================

Some existing example configurations also define a ``TP_*`` convenience macro
and ``XCP_TRANSPORT_LAYER_TYPE`` for their build selection. Follow the local
example convention; do not select conflicting transports.

``XCP_MAX_CTO`` and ``XCP_MAX_DTO`` define maximum command and data payload
sizes. Classic CAN requires values in the range enforced by ``xcp.h``; the
Ethernet, SXI, and Bluetooth adapters have their own practical transport
limits. CXCP undefines and derives
``XCP_TRANSPORT_LAYER_LENGTH_SIZE``,
``XCP_TRANSPORT_LAYER_COUNTER_SIZE``, and
``XCP_TRANSPORT_LAYER_CHECKSUM_SIZE`` from the selected transport. Application
configuration must not override those derived values.

CAN
^^^

Set ``XCP_ON_CAN_INBOUND_IDENTIFIER`` and
``XCP_ON_CAN_OUTBOUND_IDENTIFIER`` for master-to-slave and slave-to-master
traffic. ``XCP_ON_CAN_BROADCAST_IDENTIFIER`` optionally enables broadcast
handling. Use ``XCP_ON_CAN_EXT_IDENTIFIER`` to mark an extended identifier,
and configure the adapter's timing and interface parameters where applicable.
``XCP_ON_CAN_MAX_DLC_REQUIRED`` controls whether received frames must use the
configured maximum DLC.

Ethernet
^^^^^^^^

``XCP_ON_ETHERNET_PORT`` selects the listening port. The protocol is selected
with ``XCP_ON_ETHERNET_PROTOCOL`` as UDP or TCP; UDP is the default in
``xcp.h``. The Arduino adapter additionally uses its networking driver,
addressing, and Wi-Fi configuration macros. Ethernet discovery is controlled
by ``XCP_ENABLE_ETH_DISCOVERY`` and the related discovery MAC/status macros.

SXI and Bluetooth
^^^^^^^^^^^^^^^^^

SXI requires ``XCP_ON_SXI_HEADER_FORMAT``,
``XCP_ON_SXI_BITRATE``, ``XCP_ON_SXI_CONFIG``, and
``XCP_ON_SXI_TAIL_CHECKSUM``. Optional SXI framing adds the sync and escape
settings. Bluetooth configuration selects an Arduino backend with
``XCP_ON_BTH_ARDUINO_DRIVER`` and may specify the device name, pairing PIN, or
BLE service and characteristic UUIDs.

DAQ sizing and mode
-------------------

``XCP_DAQ_CONFIG_TYPE`` selects no DAQ, static DAQ, or dynamic DAQ.
Predefined lists require application declarations such as the macros used in
``examples/pio/src/main.cpp``. Dynamic lists use one shared entity pool:
``XCP_DAQ_MAX_DYNAMIC_ENTITIES`` covers DAQ lists, ODTs, and ODT entries
together, not independently.

``XCP_DAQ_DTO_BUFFER_SIZE`` and ``XCP_DAQ_QUEUE_SIZE`` determine DTO storage.
Timestamp DTO behavior is controlled by ``XCP_DAQ_ENABLE_TIMESTAMPING``,
``XCP_DAQ_TIMESTAMP_UNIT``, and ``XCP_DAQ_TIMESTAMP_SIZE``.
``XCP_DAQ_ENABLE_WRITE_THROUGH`` controls whether internal DTO buffering is
used. Packed DAQ support is optional through ``XCP_DAQ_ENABLE_PACKED_MODE``;
the header validates its required settings when enabled.

Set ``XCP_DAQ_MAX_EVENT_CHANNEL`` high enough for every declared event. Each
event must be triggered by the application. The current implementation
supports one DAQ list per event; leave
``XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT`` disabled.

Application hooks and locking
-----------------------------

``XCP_ENABLE_ADDRESS_MAPPER``, ``XCP_ENABLE_CHECK_MEMORY_ACCESS``, and
``XCP_ENABLE_GET_ID_HOOK`` enable application-owned behavior described in
:doc:`hook_functions`. The target also supplies the critical-section macros
used by the core and adapters, for example ``XCP_ENTER_CRITICAL()`` and
``XCP_DAQ_ENTER_CRITICAL()``. Map these macros to synchronization appropriate
for the target's foreground, interrupt, and transport execution model.

Time correlation and event APIs
-------------------------------

``XCP_ENABLE_TIME_CORRELATION`` includes the optional time-correlation module
and its configuration. Event and service-request packet APIs are separately
gated by ``XCP_ENABLE_EVENT_PACKET_API`` and
``XCP_ENABLE_SERVICE_REQUEST_API``. Enable them only when the application
uses their public functions.
