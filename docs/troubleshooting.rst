Troubleshooting
===============

This page collects common integration and runtime issues observed when bringing
up CXCP on new targets.

Build and link errors
---------------------

Multiple definition or duplicate symbol errors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Cause: multiple transport or hardware adapters were compiled into one target.

Action:

* Keep exactly one transport implementation from ``src/tl``.
* Keep exactly one platform implementation from ``src/hw``.
* Use an existing example source list as the baseline for your target.

Compile-time configuration ``#error`` from ``inc/xcp.h``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Cause: invalid or unsupported option combinations in ``xcp_config.h``.

Action:

* Read the specific emitted ``#error`` diagnostic.
* Validate transport-dependent size macros and feature switches.
* Use ``XCP_ON`` and ``XCP_OFF`` consistently for compile-time switches.

No connection or no response
----------------------------

No response to CONNECT or GET_STATUS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Check the runtime sequence first:

.. code-block:: c

   XcpTl_MainFunction();
   Xcp_MainFunction();

Then verify:

* Transport endpoint settings match the master configuration.
* Selected transport in ``xcp_config.h`` matches linked source files.
* Main loop cadence is fast enough for request/response processing.

No DAQ data
-----------

DAQ configuration appears valid but no DTO frames are sent
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Check the full chain:

1. Event is declared in ``XcpDaq_Events``.
2. Master configured and started the DAQ list.
3. Application calls ``XcpDaq_TriggerEvent(eventNumber)`` at the expected rate.
4. DAQ limits (lists, ODTs, entries, queue) are large enough.

For dynamic DAQ allocation, preserve the required order:

.. code-block:: text

   FreeDaq -> AllocDaq -> AllocOdt -> AllocOdtEntry

Test environment issues
-----------------------

Python tests fail to import extension module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Cause: the pybind extension has not been built in ``tests``.

Action:

.. code-block:: console

   python -m pip install -r tests/requirements.txt pybind11
   Push-Location tests
   python build.py
   python -m pytest -svv
   Pop-Location

Documentation build fails
-------------------------

Sphinx build errors in ``docs``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Action:

* Build from repository root.
* Ensure the active Python environment has Sphinx installed.
* Re-run:

.. code-block:: console

   Push-Location docs
   .\make.bat html
   Pop-Location
