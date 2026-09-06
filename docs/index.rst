BlueParrot XCP documentation
============================

BlueParrot XCP (CXCP) is a configurable XCP slave implementation for
embedded applications and host-side simulation. The stack is integrated into
an application at build time: the application chooses a transport, supplies
``xcp_config.h``, and builds the matching core, transport, and hardware
sources.

Start with :doc:`install` to build a supplied target, then use
:doc:`tutorial` to integrate the stack into an application. The source header
``inc/xcp.h`` remains the authoritative definition of compile-time validation
and public C declarations.

.. toctree::
   :maxdepth: 2
   :caption: Contents

   install
   user_manual
   tutorial
   options
   hook_functions
   api

Useful source locations
-----------------------

* ``inc/xcp.h`` — public integration boundary, protocol types, defaults, and
  compile-time checks.
* ``src/xcp.c`` — protocol state and command dispatch.
* ``src/xcp_daq.c`` — DAQ allocation, configuration, and DTO generation.
* ``src/tl`` — transport adapters.
* ``src/hw`` — platform and synchronization adapters.
* ``examples`` — maintained buildable integrations.

Indices and search
------------------

* :ref:`genindex`
* :ref:`search`
