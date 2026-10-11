BlueParrot XCP documentation
============================

BlueParrot XCP (CXCP) is a configurable C implementation of an XCP slave for
embedded targets and host-side simulation. The project is integrated into an
application at build time: each target selects one transport, provides
``xcp_config.h``, and compiles a compatible source set from core, transport,
and platform adapters.

Use this documentation in the following order:

#. :doc:`getting_started` for a fast first build and protocol smoke test.
#. :doc:`install` for supported build paths and target-specific requirements.
#. :doc:`tutorial` for full integration into a custom application.
#. :doc:`integration_examples` for end-to-end deployment examples.
#. :doc:`options` and :doc:`hook_functions` for configuration and policy hooks.
#. :doc:`api` for the public interface reference.

.. toctree::
   :maxdepth: 2
   :caption: Contents

   getting_started
   install
   architecture
   user_manual
   tutorial
   integration_examples
   options
   hook_functions
   troubleshooting
   api

Repository map
--------------

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
