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

.. c:macro:: XCP_ENABLE_CAL_COMMANDS

   x

.. c:macro:: XCP_ENABLE_PAG_COMMANDS

   y

.. c:macro:: XCP_ENABLE_DAQ_COMMANDS

   z

.. c:macro:: XCP_ENABLE_PGM_COMMANDS

   j

Transport-Layer specific options
--------------------------------

Customization options
---------------------

Platform specific options
-------------------------


