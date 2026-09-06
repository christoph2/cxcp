Application hook functions
==========================

Hooks keep target-specific policy out of the protocol core. Implement a hook
only when its associated configuration option is enabled. The maintained
``examples/xcpsim/app_config.c`` contains complete, buildable examples of the
memory, seed/key, and GET_ID hooks.

Address mapping
---------------

Enable ``XCP_ENABLE_ADDRESS_MAPPER`` when XCP addresses must be translated
before the core accesses them. Implement:

.. code-block:: c

   Xcp_MemoryMappingResultType Xcp_HookFunction_AddressMapper(
       Xcp_MtaType *dst,
       Xcp_MtaType const *src
   );

``src`` is the requested memory-transfer address and ``dst`` receives the
mapped address. Return the mapping result expected by the XCP types in
``xcp.h``. The simulator forwards this to its flash-emulation mapper.

Memory-access policy
--------------------

Enable ``XCP_ENABLE_CHECK_MEMORY_ACCESS`` to authorize each memory access:

.. code-block:: c

   bool Xcp_HookFunction_CheckMemoryAccess(
       Xcp_MtaType mta,
       uint32_t length,
       Xcp_MemoryAccessType access,
       bool programming
   );

Return true only for an allowed range and operation. A production hook should
check the complete range, prevent integer-overflow mistakes when calculating
its end, distinguish read and write requests, and apply programming policy
when ``programming`` is true. Do not copy the simulator's permissive example
into production firmware.

Seed and key
------------

Seed/key protection needs both ``XCP_ENABLE_GET_SEED`` and
``XCP_ENABLE_UNLOCK``. The seed provider uses:

.. code-block:: c

   bool Xcp_HookFunction_GetSeed(
       uint8_t resource,
       Xcp_1DArrayType *result
   );

Set ``result->data`` and ``result->length`` to a valid seed buffer for the
requested resource, then return true when a seed is available. The unlock
implementation receives the resource and supplied key:

.. code-block:: c

   bool Xcp_HookFunction_Unlock(
       uint8_t resource,
       Xcp_1DArrayType const *key
   );

Return true only after validating the complete key. The sample algorithm is
for demonstration and must be replaced with a security design appropriate for
the deployed product.

Custom GET_ID responses
-----------------------

Enable ``XCP_ENABLE_GET_ID_HOOK`` to provide user-defined GET_ID types:

.. code-block:: c

   bool Xcp_HookFunction_GetId(
       uint8_t id_type,
       uint8_t **result,
       uint32_t *result_length
   );

On success, set the byte buffer and its exact length and return true. Return
false for ID types that the hook does not own so the core can use its normal
handling. The returned storage must remain valid until the response has been
sent; static or application-owned storage is suitable, while a stack buffer
is not.

Replacing memory copy
---------------------

``XCP_REPLACE_STD_COPY_MEMORY`` is the extension point for targets that
cannot use the standard copy path. Consult the declarations and conditional
implementation in ``inc/xcp.h`` and ``src/xcp.c`` for the required target
contract before enabling it.
