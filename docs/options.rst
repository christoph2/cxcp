Configuration Options
=====================


Configuration options need to be supplied in a file named **xcp_config.h**. Typically one uses a file from the examples
directory as a starting point.

.. Note::
   - **bool** is either **XCP_ON** or **XCP_OFF** .
   - Options starting with **XCP_ENABLE_** are of type **bool** to enable/disable some functionality.


General Options
---------------

    .. c:macro:: XCP_GET_ID_0

        A short description of your node.
        This is the return value of **GET_ID(0)** .

    .. c:macro:: XCP_GET_ID_1


        A2L filename without path and extension.
        This is the return value of **GET_ID(1)** .

    .. c:macro:: XCP_BUILD_TYPE

        Build type is either **XCP_DEBUG_BUILD** or **XCP_RELEASE_BUILD**.
        Most useful if you need to debug your application or your're a contributor to **BlueParrotXCP**.

    .. c:macro:: XCP_ENABLE_EXTERN_C_GUARDS

        If enabled, include-files are frame with:

        .. code-block:: c

            #if defined(__cplusplus)
            extern "C"
            {
            #endif  /* __cplusplus *

            /*
            ...
            ...
            */

            #if defined(__cplusplus)
            }
            #endif  /* __cplusplus *

        Required for C++ linkage.

    .. c:macro:: XCP_ENABLE_SLAVE_BLOCKMODE

            If enabled, slave may use block transfer mode.


    .. c:macro:: XCP_ENABLE_MASTER_BLOCKMODE

            If enabled, master may use block transfer mode. In this case, options :c:macro:`XCP_MAX_BS` and :c:macro:`XCP_MIN_ST` apply.

    .. c:macro:: XCP_ENABLE_STIM                             **bool**
            STIM is currently not implemented.

    .. c:macro:: XCP_CHECKSUM_METHOD

            Choose one of:

           * XCP_CHECKSUM_METHOD_XCP_ADD_11
           * XCP_CHECKSUM_METHOD_XCP_ADD_12
           * XCP_CHECKSUM_METHOD_XCP_ADD_14
           * XCP_CHECKSUM_METHOD_XCP_ADD_22
           * XCP_CHECKSUM_METHOD_XCP_ADD_24
           * XCP_CHECKSUM_METHOD_XCP_ADD_44
           * XCP_CHECKSUM_METHOD_XCP_CRC_16
           * XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT
           * XCP_CHECKSUM_METHOD_XCP_CRC_32

           **XCP_USER_DEFINED** not supported yet.


    .. c:macro:: XCP_CHECKSUM_CHUNKED_CALCULATION            **bool**

            If **XCP_FALSE** checksums are completely calculated for requested memory blocks,
            otherwise chunked in a periodically called MainFunction.

    .. c:macro:: XCP_CHECKSUM_CHUNK_SIZE

            Chunk size in bytes.
            s. :c:macro:`XCP_CHECKSUM_CHUNKED_CALCULATION`

    .. c:macro:: XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE

            You may want to limit maximum checksum block size (in bytes), **0** means unlimited (4294967295 to be exact).

    .. c:macro:: XCP_BYTE_ORDER

            Byteorder / endianess of your platform, choose either **XCP_BYTE_ORDER_INTEL** or **XCP_BYTE_ORDER_MOTOROLA**

    .. c:macro:: XCP_ADDRESS_GRANULARITY


            Choose **XCP_ADDRESS_GRANULARITY_BYTE**, **XCP_ADDRESS_GRANULARITY_WORD** and **XCP_ADDRESS_GRANULARITY_DWORD**
            are not supported yet.

    .. c:macro:: XCP_ENABLE_STATISTICS


            If enabled collect some statistics like traffic and so on.

    .. c:macro:: XCP_MAX_BS

            Indicates the maximum allowed block size as the number of consecutive command packets (**DOWNLOAD_NEXT**) in a block sequence.

    .. c:macro:: XCP_MIN_ST

            Indicates the required minimum separation time between the packets of a block transfer from the master
            device to the slave device in units of 100 microseconds.

    .. c:macro:: XCP_QUEUE_SIZE

            Applies to **INTERLEAVED_MODE**, which is currently not supported.

Resource Protection Options
---------------------------

These option determine the initial per-session resource protection state.

   .. c:macro:: XCP_PROTECT_CAL         **bool**
   .. c:macro:: XCP_PROTECT_PAG         **bool**
   .. c:macro:: XCP_PROTECT_DAQ         **bool**
   .. c:macro:: XCP_PROTECT_STIM        **bool**
   .. c:macro:: XCP_PROTECT_PGM         **bool**


DAQ Options
-----------

   .. c:macro:: XCP_DAQ_CONFIG_TYPE

       - XCP_DAQ_CONFIG_TYPE_NONE
             No DAQ lists at all.

       - XCP_DAQ_CONFIG_TYPE_STATIC
             Only static DAQ lists.

       - XCP_DAQ_CONFIG_TYPE_DYNAMIC
             Only dynamic DAQ lists.

   .. c:macro:: XCP_DAQ_DTO_BUFFER_SIZE

        Size of DTO message buffer (in bytes).

   .. c:macro:: XCP_DAQ_ENABLE_PREDEFINED_LISTS

        Enable support for predefined DAQ lists.

   .. c:macro:: XCP_DAQ_TIMESTAMP_UNIT

       Choose:

          *  XCP_DAQ_TIMESTAMP_UNIT_1NS
          *  XCP_DAQ_TIMESTAMP_UNIT_10NS
          *  XCP_DAQ_TIMESTAMP_UNIT_100NS
          *  XCP_DAQ_TIMESTAMP_UNIT_1US
          *  XCP_DAQ_TIMESTAMP_UNIT_10US
          *  XCP_DAQ_TIMESTAMP_UNIT_100US
          *  XCP_DAQ_TIMESTAMP_UNIT_1MS
          *  XCP_DAQ_TIMESTAMP_UNIT_10MS
          *  XCP_DAQ_TIMESTAMP_UNIT_100MS
          *  XCP_DAQ_TIMESTAMP_UNIT_1S
          *  XCP_DAQ_TIMESTAMP_UNIT_1PS
          *  XCP_DAQ_TIMESTAMP_UNIT_10PS
          *  XCP_DAQ_TIMESTAMP_UNIT_100PS

   .. c:macro:: XCP_DAQ_TIMESTAMP_SIZE

       Timestamps could be either 1, 2, or 4 bytes in size:

           * XCP_DAQ_TIMESTAMP_SIZE_1
           * XCP_DAQ_TIMESTAMP_SIZE_2
           * XCP_DAQ_TIMESTAMP_SIZE_4

   .. c:macro:: XCP_DAQ_ENABLE_PRESCALER            **bool**

           DAQ list prescaling is currently not supported.

   .. c:macro:: XCP_DAQ_ENABLE_ADDR_EXT             **bool**

           Measurement quantities can't have an address extension yet.

   .. c:macro:: XCP_DAQ_ENABLE_BIT_OFFSET           **bool**

           Bit offsets are currently not supported.

   .. c:macro:: XCP_DAQ_ENABLE_PRIORITIZATION       **bool**

           DAQ list prioritization not supported yet.

   .. c:macro:: XCP_DAQ_ENABLE_ALTERNATING          **bool**

           Alternating display mode not supported yet.

   .. c:macro:: XCP_DAQ_ENABLE_WRITE_THROUGH        **bool**

           **XCP_OFF**: Disable internal buffering of **DTO** messages, in this case buffering must be handled by your network/socket stack.

   .. c:macro:: XCP_DAQ_MAX_DYNAMIC_ENTITIES

       The maximum number of allocatable DAQ entities -- DAQ lists, ODTs, and ODT entries.
       Multiply by sizeof(`XcpDaq_EntityType`) on your platform to get memory usage.

   .. c:macro:: XCP_DAQ_MAX_EVENT_CHANNEL

       Number of available event channels.

   .. c:macro:: XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT **bool**

       Enable/disable support for multiple DAQ list per event.

   .. c:macro:: XCP_DAQ_ENABLE_TIMESTAMPING **bool**

       If enabled, each DTO can carry a timestamp field according to
       :c:macro:`XCP_DAQ_TIMESTAMP_UNIT` and :c:macro:`XCP_DAQ_TIMESTAMP_SIZE`.

   .. c:macro:: XCP_DAQ_ENABLE_CLOCK_ACCESS_ALWAYS **bool**

       If enabled, DAQ timestamp clock information is accessible even when DAQ is not running
       (affects responses to clock-related queries).

   .. c:macro:: XCP_DAQ_QUEUE_SIZE

       Size of the internal DAQ DTO queue (number of queued frames) when queuing is enabled internally.
       Only used when :c:macro:`XCP_DAQ_ENABLE_WRITE_THROUGH` is **XCP_OFF** and internal buffering is active.

   .. c:macro:: XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR   **bool**

       Expert option:   If **XCP_ON**, re-initialize dynamic DAQ structures after an **ERR_SEQUENCE**.

   .. c:macro:: XCP_DAQ_LIST_TYPE                           uint8_t

           Choose: **uint8_t**, **uint16_t**, or **uint32_t**.

   .. c:macro:: XCP_DAQ_ODT_TYPE                            uint8_t

           Choose: **uint8_t**, **uint16_t**, or **uint32_t**.

   .. c:macro:: XCP_DAQ_ODT_ENTRY_TYPE                      uint8_t

           Choose: **uint8_t**, **uint16_t**, or **uint32_t**.



Optional Services
-----------------

Enable/disable optional XCP service categories/services. These options are rather self-explanatory.

Optional Standard Services
^^^^^^^^^^^^^^^^^^^^^^^^^^^

    .. c:macro:: XCP_ENABLE_GET_COMM_MODE_INFO
    .. c:macro:: XCP_ENABLE_GET_ID
    .. c:macro:: XCP_ENABLE_SET_REQUEST
    .. c:macro:: XCP_ENABLE_GET_SEED
    .. c:macro:: XCP_ENABLE_UNLOCK
    .. c:macro:: XCP_ENABLE_SET_MTA
    .. c:macro:: XCP_ENABLE_UPLOAD
    .. c:macro:: XCP_ENABLE_SHORT_UPLOAD
    .. c:macro:: XCP_ENABLE_BUILD_CHECKSUM
    .. c:macro:: XCP_ENABLE_TRANSPORT_LAYER_CMD
    .. c:macro:: XCP_ENABLE_USER_CMD


.. c:macro:: XCP_ENABLE_CAL_COMMANDS

Optional Calibration Services
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   .. c:macro:: XCP_ENABLE_DOWNLOAD_NEXT
   .. c:macro:: XCP_ENABLE_DOWNLOAD_MAX
   .. c:macro:: XCP_ENABLE_SHORT_DOWNLOAD
   .. c:macro:: XCP_ENABLE_MODIFY_BITS

.. c:macro:: XCP_ENABLE_PAG_COMMANDS

Optional Paging Services
^^^^^^^^^^^^^^^^^^^^^^^^

    .. c:macro:: XCP_ENABLE_GET_PAG_PROCESSOR_INFO
    .. c:macro:: XCP_ENABLE_GET_SEGMENT_INFO
    .. c:macro:: XCP_ENABLE_GET_PAGE_INFO
    .. c:macro:: XCP_ENABLE_SET_SEGMENT_MODE
    .. c:macro:: XCP_ENABLE_GET_SEGMENT_MODE
    .. c:macro:: XCP_ENABLE_COPY_CAL_PAGE


.. c:macro:: XCP_ENABLE_DAQ_COMMANDS

Optional DAQ Services
^^^^^^^^^^^^^^^^^^^^^

    .. c:macro:: XCP_ENABLE_GET_DAQ_CLOCK
    .. c:macro:: XCP_ENABLE_READ_DAQ
    .. c:macro:: XCP_ENABLE_GET_DAQ_PROCESSOR_INFO
    .. c:macro:: XCP_ENABLE_GET_DAQ_RESOLUTION_INFO
    .. c:macro:: XCP_ENABLE_GET_DAQ_LIST_INFO
    .. c:macro:: XCP_ENABLE_GET_DAQ_EVENT_INFO
    .. c:macro:: XCP_ENABLE_FREE_DAQ
    .. c:macro:: XCP_ENABLE_ALLOC_DAQ
    .. c:macro:: XCP_ENABLE_ALLOC_ODT
    .. c:macro:: XCP_ENABLE_ALLOC_ODT_ENTRY
    .. c:macro:: XCP_ENABLE_WRITE_DAQ_MULTIPLE


.. c:macro:: XCP_ENABLE_PGM_COMMANDS

Optional Programming Services
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    .. c:macro:: XCP_ENABLE_GET_PGM_PROCESSOR_INFO
    .. c:macro:: XCP_ENABLE_GET_SECTOR_INFO
    .. c:macro:: XCP_ENABLE_PROGRAM_PREPARE
    .. c:macro:: XCP_ENABLE_PROGRAM_FORMAT
    .. c:macro:: XCP_ENABLE_PROGRAM_NEXT
    .. c:macro:: XCP_ENABLE_PROGRAM_MAX
    .. c:macro:: XCP_ENABLE_PROGRAM_VERIFY

Transport-Layer specific options
--------------------------------

This section documents configuration macros that apply to specific transport layers.
Select the transport with :c:macro:`XCP_TRANSPORT_LAYER` (values: ``XCP_ON_CAN``, ``XCP_ON_ETHERNET``, ``XCP_ON_SXI``, ``XCP_ON_BTH``).

Common transport-layer sizes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   .. c:macro:: XCP_MAX_CTO

      Maximum size (in bytes) of a Command Transfer Object (CTO).
      - On CAN Classic, this is forced to 8. On CAN FD it may be 8 .. 64.
      - On Ethernet/SXI typical values are 32 .. 64.

   .. c:macro:: XCP_MAX_DTO

      Maximum size (in bytes) of a Data Transfer Object (DTO). See transport constraints above.

   .. note::
      The implementation derives internal header sizes per transport via
      :c:macro:`XCP_TRANSPORT_LAYER_LENGTH_SIZE`, :c:macro:`XCP_TRANSPORT_LAYER_COUNTER_SIZE`, and
      :c:macro:`XCP_TRANSPORT_LAYER_CHECKSUM_SIZE`. These are computed and typically not configured directly.

XCP on CAN (XCP_ON_CAN)
^^^^^^^^^^^^^^^^^^^^^^^

   .. c:macro:: XCP_ON_CAN_INBOUND_IDENTIFIER

      CAN Identifier used by the slave for receiving CTOs (master → slave).
      Use bit 31 to mark Extended IDs (29-bit): set ``XCP_ON_CAN_EXT_IDENTIFIER``.

   .. c:macro:: XCP_ON_CAN_OUTBOUND_IDENTIFIER

      CAN Identifier used by the slave for transmitting CRO/DTO frames (slave → master).

   .. c:macro:: XCP_ON_CAN_BROADCAST_IDENTIFIER

      Optional broadcast identifier to react to broadcast requests.

   .. c:macro:: XCP_ON_CAN_MAX_DLC_REQUIRED **bool**

      If **XCP_ON**, require every received frame to use DLC == :c:macro:`XCP_MAX_CTO`.
      If **XCP_OFF**, accept shorter CTO frames.

   Timing / bit-rate
   """""""""""""""""""

   .. c:macro:: XCP_ON_CAN_FREQ
   .. c:macro:: XCP_ON_CAN_BTQ
   .. c:macro:: XCP_ON_CAN_TSEG1
   .. c:macro:: XCP_ON_CAN_TSEG2
   .. c:macro:: XCP_ON_CAN_SJW
   .. c:macro:: XCP_ON_CAN_NOSAMP

      Configure the CAN bit timing (implementation-specific mapping to the underlying driver).

   Optional transport-layer commands (XCPonCAN)
   """""""""""""""""""""""""""""""""""""""""

   .. c:macro:: XCP_ENABLE_CAN_GET_SLAVE_ID **bool**
   .. c:macro:: XCP_ENABLE_CAN_GET_DAQ_ID   **bool**
   .. c:macro:: XCP_ENABLE_CAN_SET_DAQ_ID   **bool**

      Enabling any of these will automatically enable :c:macro:`XCP_ENABLE_TRANSPORT_LAYER_CMD`.

   Interface selection and pins (MCU shields)
   """"""""""""""""""""""""""""""""""""""""""""

   .. c:macro:: XCP_CAN_INTERFACE

      Select a supported CAN shield/driver:
        - ``XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD``
        - ``XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD``
        - ``XCP_CAN_IF_MKR_ZERO_CAN_SHIELD``
        - ``XCP_CAN_IF_SPARKFUN_CAN_SHIELD``

   .. c:macro:: XCP_CAN_IF_MCP25XX_PIN_CS
   .. c:macro:: XCP_CAN_IF_MCP25XX_PIN_INT

      Chip-select and interrupt pin numbers for MCP25xx-based shields. Defaults depend on platform.

XCP on SXI (serial) (XCP_ON_SXI)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   .. c:macro:: XCP_ON_SXI_HEADER_FORMAT

      Select serial header layout. Choose one of:
        - ``XCP_ON_SXI_HEADER_LEN_BYTE``
        - ``XCP_ON_SXI_HEADER_LEN_WORD``
        - ``XCP_ON_SXI_HEADER_LEN_CTR_BYTE``
        - ``XCP_ON_SXI_HEADER_LEN_CTR_WORD``
        - ``XCP_ON_SXI_HEADER_LEN_FILL_BYTE``
        - ``XCP_ON_SXI_HEADER_LEN_FILL_WORD``

   .. c:macro:: XCP_ON_SXI_BITRATE

      UART bit-rate (baud).

   .. c:macro:: XCP_ON_SXI_CONFIG

      UART configuration (e.g., ``SERIAL_8N1`` on Arduino cores).

   .. c:macro:: XCP_ON_SXI_TAIL_CHECKSUM

      Select checksum for trailer: ``XCP_ON_SXI_NO_CHECKSUM``, ``XCP_ON_SXI_CHECKSUM_BYTE``, or ``XCP_ON_SXI_CHECKSUM_WORD``.

   Framing and escaping (optional)
   """""""""""""""""""""""""""""

   .. c:macro:: XCP_ON_SXI_ENABLE_FRAMING **bool**
   .. c:macro:: XCP_ON_SXI_SYNC_CHAR
   .. c:macro:: XCP_ON_SXI_ESC_CHAR
   .. c:macro:: XCP_ON_SXI_ESC_SYNC_CHAR
   .. c:macro:: XCP_ON_SXI_ESC_ESC_CHAR

      Enable XCP-style framing with sync/escape characters on the serial link.

   .. note::
      On SXI the effective CTO buffer includes the header and optional counter and checksum. Keep :c:macro:`XCP_MAX_CTO`
      and :c:macro:`XCP_MAX_DTO` in sync with your UART performance.

XCP on Ethernet (XCP_ON_ETHERNET)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   .. c:macro:: XCP_ON_ETHERNET_PORT

      UDP/TCP port the slave listens on. Default is 5555.

   .. c:macro:: XCP_ON_ETHERNET_IP_OCTETS

      Provide a static IPv4 address as four octets for Arduino Ethernet (e.g., ``192, 168, 1, 100``).
      Alternatively, define :c:macro:`XCP_ON_ETHERNET_IP` with a string ("192.168.1.100"). If neither is set, DHCP is used.

   .. c:macro:: XCP_ON_ETHERNET_ARDUINO_DRIVER

      Select Arduino networking backend: ``XCP_ON_ETHERNET_DRIVER_ETHERNET`` or ``XCP_ON_ETHERNET_DRIVER_WIFI``.

   .. c:macro:: XCP_ON_ETHERNET_WIFI_SSID
   .. c:macro:: XCP_ON_ETHERNET_WIFI_PASSWORD

      WiFi credentials (ESP32 and compatible cores) when using the WiFi backend.

   .. c:macro:: XCP_ON_ETHERNET_MAC_ADDRESS

      MAC address used for Arduino Ethernet (array of 6 bytes).

   .. note::
      XCP on Ethernet prepends a 4-byte header (LEN, CTR). The implementation handles this automatically; ensure
      :c:macro:`XCP_MAX_CTO`/:c:macro:`XCP_MAX_DTO` are sized to accommodate your application throughput.

Customization options
---------------------

Platform specific options
-------------------------


.. _my-reference-label:

Section to cross-reference
--------------------------
This is the text of the section.
It refers to the section itself, see :ref:`my-reference-label`.
