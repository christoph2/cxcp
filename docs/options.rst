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

Customization options
---------------------

Platform specific options
-------------------------


.. _my-reference-label:

Section to cross-reference
--------------------------
This is the text of the section.
It refers to the section itself, see :ref:`my-reference-label`.
