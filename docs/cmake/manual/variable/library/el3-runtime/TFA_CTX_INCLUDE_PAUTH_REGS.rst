TFA_CTX_INCLUDE_PAUTH_REGS
==========================

.. default-domain:: cmake

.. variable:: TFA_CTX_INCLUDE_PAUTH_REGS

Enables Pointer Authentication for the Secure world. This will cause the
ARMv8.3-A Pointer Authentication registers to be included when saving and
restoring the CPU context as part of world switches. This is an experimental
feature and is disabled by default.

.. note::

    Pointer Authentication is enabled for the Non-secure world
    irrespective of the value of this flag if the CPU supports it.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

