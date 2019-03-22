Configuration Options
=====================

Configuration options need to be supplied in a user edited file
named `xcp_config.h`. Typically one uses a file from the examples
directory as a starting point.

.. Note::

    Options starting with `XCP_ENABLE_` categorically use `XCP_ON` or `XCP_OFF` to enable resp. disable some functionality.


Basic Options
-------------


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


