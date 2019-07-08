Configuration Options
=====================


Configuration options need to be supplied in a file named `xcp_config.h`. Typically one uses a file from the examples
directory as a starting point.

.. Note::
   - `bool` is either `XCP_ON` or `XCP_OFF`.
   - Options starting with `XCP_ENABLE_` are of type `bool` to enable resp. disable some functionality.


General Options
---------------

    .. c:macro:: XCP_STATION_ID

        A short description of your node (this is usually the name of an A2L file).

    .. c:macro:: XCP_BUILD_TYPE

        Build type is either `XCP_DEBUG_BUILD` or `XCP_RELEASE_BUILD`.

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

            if enabled, slave may use block transfer mode.


    .. c:macro:: XCP_ENABLE_MASTER_BLOCKMODE

            If enabled, master may use block transfer mode. In this case, options :c:macro:`XCP_MAX_BS` and :c:macro:`XCP_MIN_ST` apply.

    .. c:macro:: XCP_ENABLE_STIM                             `bool`

    .. c:macro:: XCP_CHECKSUM_METHOD                         XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT
    .. c:macro:: XCP_CHECKSUM_CHUNKED_CALCULATION            `bool`
    .. c:macro:: XCP_CHECKSUM_CHUNK_SIZE                     (64)
    .. c:macro:: XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE             (0)     /* 0 ==> unlimited */

    .. c:macro:: XCP_BYTE_ORDER                              XCP_BYTE_ORDER_INTEL
    .. c:macro:: XCP_ADDRESS_GRANULARITY                     XCP_ADDRESS_GRANULARITY_BYTE


    .. c:macro:: XCP_MAX_BS                                  (0)
    .. c:macro:: XCP_MIN_ST                                  (0)
    .. c:macro:: XCP_QUEUE_SIZE                              (0)

    .. c:macro:: XCP_DAQ_MAX_DYNAMIC_ENTITIES

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

 :abbr:`LIFO (last-in, first-out)`.

 :menuselection:`Start --> Programs`

.. _my-reference-label:

Section to cross-reference
--------------------------
This is the text of the section.
It refers to the section itself, see :ref:`my-reference-label`.
