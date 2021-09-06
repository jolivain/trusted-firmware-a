TFA_CTX_INCLUDE_EL2_REGS
========================

.. default-domain:: cmake

.. variable:: TFA_CTX_INCLUDE_EL2_REGS

Provides context save/restore operations when entering/exiting an
EL2 execution context. This is of primary interest when the Armv8.4-A Secure
EL2 extension is implemented.

This option is disabled by default, but is
forcibly enabled if the Secure Payload Dispatcher is configured to be |SPMD|.
This option is forcibly enabled if :variable:`SPMD_SPM_AT_SEL2` is enabled.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

