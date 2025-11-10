Tutorial
========

This tutorial gives you a practical, step-by-step path to get a BlueParrot XCP slave running on
- Arduino boards (serial SXI, Ethernet/WiFi or CAN shields)
- Raspberry Pi Pico (RP2040)

It focuses on minimal configuration in your `xcp_config.h`, building/flashing, and connecting from an XCP client.
For a full description of all configuration options, see :doc:`options`.

Quick overview
--------------
- Transport selection: set :c:macro:`XCP_TRANSPORT_LAYER` to one of ``XCP_ON_ETHERNET``, ``XCP_ON_CAN``, or ``XCP_ON_SXI``.
- Sizes: pick sensible :c:macro:`XCP_MAX_CTO` and :c:macro:`XCP_MAX_DTO` (see Transport-Layer section in :doc:`options`).
- DAQ: start with dynamic DAQ lists if you plan to measure variables: ``XCP_DAQ_CONFIG_TYPE_DYNAMIC``.
- Identification: set :c:macro:`XCP_GET_ID_0` and :c:macro:`XCP_GET_ID_1` to something meaningful for your node/A2L.

Arduino quick start
-------------------
The repository contains ready-to-use sketches:
- ``tools/arduino/hello_xcp``: minimal “hello” XCP slave
- ``tools/arduino/predefined_daq/predefined_daq.ino``: shows how-to use predefined DAQ lists

Prerequisites
^^^^^^^^^^^^^
- Arduino IDE or Arduino CLI installed
- For Ethernet: an Ethernet-capable board/shield (e.g., W5100/W5500) or WiFi board (ESP32)
- For CAN: a CAN shield (MKR Zero CAN, Seeed Studio CAN, and SparkFun CAN currently supported).
- Serial SXI over the board’s UART (recommended starting point).


1) Choose a transport
^^^^^^^^^^^^^^^^^^^^^
Open the sketch folder (e.g., ``tools/arduino/hello_xcp``) and edit its ``xcp_config.h``,
un-comment the transport you want to use:

- `#define TP_SXI` for Serial SXI.

- `#define TP_CAN` for CAN bus.

- `#define TP_ETHER` for Ethernet.

Now fill the minimal settings:

Ethernet (UDP)
""""""""""""
- Select Ethernet or WiFi backend via :c:macro:`XCP_ON_ETHERNET_ARDUINO_DRIVER`.
- Keep default port 5555, or change :c:macro:`XCP_ON_ETHERNET_PORT`.
- Provide a MAC address (required by many Ethernet shields) via :c:macro:`XCP_ON_ETHERNET_MAC_ADDRESS`.
- IP address options:
  - Use DHCP by omitting IP defines, or
  - Set :c:macro:`XCP_ON_ETHERNET_IP_OCTETS` (e.g., ``192, 168, 1, 100``) or :c:macro:`XCP_ON_ETHERNET_IP` (string).
- Typical sizes: ``#define XCP_MAX_CTO (32)`` and ``#define XCP_MAX_DTO (32)``.

Example (excerpt):

.. code-block:: c

   #define XCP_TRANSPORT_LAYER               XCP_ON_ETHERNET
   #define XCP_ON_ETHERNET_ARDUINO_DRIVER    (XCP_ON_ETHERNET_DRIVER_ETHERNET)
   #define XCP_ON_ETHERNET_PORT              (5555)
   #define XCP_ON_ETHERNET_MAC_ADDRESS       { 0xBE, 0xEF, 0xCA, 0xAA, 0xFF, 0xFE }
   // Static IP (optional, otherwise DHCP)
   // #define XCP_ON_ETHERNET_IP_OCTETS 192, 168, 1, 100
   #define XCP_MAX_CTO (32)
   #define XCP_MAX_DTO (32)

WiFi (ESP32 and compatibles)
"""""""""""""""""""""""
- Use the WiFi backend and set credentials:

.. code-block:: c

   #define XCP_TRANSPORT_LAYER               XCP_ON_ETHERNET
   #define XCP_ON_ETHERNET_ARDUINO_DRIVER    (XCP_ON_ETHERNET_DRIVER_WIFI)
   #define XCP_ON_ETHERNET_WIFI_SSID         ("MySSID")
   #define XCP_ON_ETHERNET_WIFI_PASSWORD     ("MyPass")
   #define XCP_ON_ETHERNET_PORT              (5555)
   #define XCP_MAX_CTO (32)
   #define XCP_MAX_DTO (32)

CAN (Classic or FD)
""""""""""""""""""
- Select XCP on CAN and set the CAN identifiers (note: bit 31 marks extended ID):

.. code-block:: c

   #define XCP_TRANSPORT_LAYER               XCP_ON_CAN
   #define XCP_ON_CAN_INBOUND_IDENTIFIER     (0x300)
   #define XCP_ON_CAN_OUTBOUND_IDENTIFIER    (0x301)
   // Optional broadcast ID
   // #define XCP_ON_CAN_BROADCAST_IDENTIFIER  (0x222)

- Choose your shield with :c:macro:`XCP_CAN_INTERFACE` (one of `XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD`, `XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD`, `XCP_CAN_IF_MKR_ZERO_CAN_SHIELD`, `XCP_CAN_IF_SPARKFUN_CAN_SHIELD`) and, if applicable, the MCP25xx pins.
- Classic CAN forces :c:macro:`XCP_MAX_CTO`/``XCP_MAX_DTO`` to 8. With CAN FD, 8..64 is allowed (see :doc:`options`).
- Optionally require max. DLC (i.e. padding) via :c:macro:`XCP_ON_CAN_MAX_DLC_REQUIRED`.

Serial (SXI)
"""""""""""
- Pick a header format and baud rate. The simple default is length+counter (16-bit each) and 38400 baud:

.. code-block:: c

   #define XCP_TRANSPORT_LAYER              XCP_ON_SXI
   #define XCP_ON_SXI_HEADER_FORMAT         (XCP_ON_SXI_HEADER_LEN_CTR_WORD)
   #define XCP_ON_SXI_BITRATE               (38400)
   #define XCP_ON_SXI_CONFIG                (SERIAL_8N1)
   #define XCP_ON_SXI_TAIL_CHECKSUM         (XCP_ON_SXI_NO_CHECKSUM)
   #define XCP_MAX_CTO                      (64)
   #define XCP_MAX_DTO                      (64)

.. note::
   The parameters `XCP_ON_SXI_BITRATE` and `XCP_ON_SXI_CONFIG` are not relevant for USB-to-USB connections, but there are boards whose RxD/TxD pins are connected
   to USB-to-serial converters; here the parameters are crucial for successful communication (not to mention real RS232 ports)!

1) Build and upload
^^^^^^^^^^^^^^^^^^^
Using Arduino IDE: open the sketch, select your board/port, then Compile/Upload.
Using Arduino CLI (example):

.. code-block:: console

   arduino-cli compile --fqbn arduino:avr:uno tools/arduino/hello_xcp
   arduino-cli upload  --fqbn arduino:avr:uno -p COM5 tools/arduino/hello_xcp

3) Connect from the client
^^^^^^^^^^^^^^^^^^^^^^^^^^
- Ethernet/WiFi: the slave listens on UDP port 5555 by default.
  - Use your preferred XCP master to connect (e.g., commercial tools or open-source clients). Ensure the IP is reachable.
- CAN: configure your PC CAN interface with matching bit timing/IDs.
- SXI: open the corresponding serial port at the configured baud and use an XCP master that supports XCPonSXI.

Tips and troubleshooting
^^^^^^^^^^^^^^^^^^^^^^^^
- On Classic CAN DLC must be 8; enable :c:macro:`XCP_ON_CAN_MAX_DLC_REQUIRED` to enforce it on RX.
- If Ethernet via DHCP does not yield an address, set a static IP using :c:macro:`XCP_ON_ETHERNET_IP_OCTETS`.
- For DAQ timestamping, enable :c:macro:`XCP_DAQ_ENABLE_TIMESTAMPING` and set :c:macro:`XCP_DAQ_TIMESTAMP_UNIT`/:c:macro:`XCP_DAQ_TIMESTAMP_SIZE`.

Raspberry Pi Pico (RP2040) quick start
--------------------------------------
The Pico is well-suited as a compact XCP slave. The repository includes a sample under ``examples/debug_on_pico``.

Prerequisites
^^^^^^^^^^^^^
- Pico SDK installed and environment set up (PICO_SDK_PATH)
- A recent CMake and an ARM GCC toolchain
- USB cable for flashing/debugging

1) Select a transport
^^^^^^^^^^^^^^^^^^^^^
Common choices are SXI over UART or Ethernet via an external module (user-specific). The simplest path is SXI.

Minimal SXI configuration (``xcp_config.h``)
"""""""""""""""""""""""""""""""""""""

.. code-block:: c

   #define XCP_TRANSPORT_LAYER              XCP_ON_SXI
   #define XCP_ON_SXI_HEADER_FORMAT         (XCP_ON_SXI_HEADER_LEN_CTR_WORD)
   #define XCP_ON_SXI_BITRATE               (115200)   /* Pico UART can go higher; pick what your host supports */
   #define XCP_ON_SXI_CONFIG                (SERIAL_8N1)
   #define XCP_ON_SXI_TAIL_CHECKSUM         (XCP_ON_SXI_NO_CHECKSUM)
   #define XCP_MAX_CTO                      (64)
   #define XCP_MAX_DTO                      (64)

Notes
"""""
- When building with ``PICO_RP2040`` debug output can be routed to USB CDC (see conditional code in :c:macro:`XCP_ENABLE_DEBUG_OUTPUT`).
- If you use DAQ, consider :c:macro:`XCP_DAQ_QUEUE_SIZE` and :c:macro:`XCP_DAQ_ENABLE_WRITE_THROUGH` for buffering.

2) Build the example
^^^^^^^^^^^^^^^^^^^^
From the repository root:

.. code-block:: console

   cd examples/debug_on_pico
   cmake -B build -S .
   cmake --build build --config Release

Then flash the generated UF2 or use your preferred uploader.

Further reading
---------------
- Configuration reference: :doc:`options`
- Core API overview and service categories: see ``inc/xcp.h`` and comments therein
- Transport implementation details: ``src/tl/eth/arduino_networking.cpp``, ``src/tl/eth/common.c``, ``src/tl/can/kvaser.c``
