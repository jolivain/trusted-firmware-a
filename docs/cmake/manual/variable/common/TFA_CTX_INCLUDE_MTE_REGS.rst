TFA_CTX_INCLUDE_MTE_REGS
========================

.. default-domain:: cmake

.. variable:: TFA_CTX_INCLUDE_MTE_REGS

Enables Memory Tagging for the Secure world. This will cause the ARMv8.5-A
Memory Tagging registers to be included when saving and restoring the CPU
context as part of world switches. This is an experimental feature and is
disabled by default.

.. note::

    Memory Tagging is enabled for the Non-secure world irrespective of the value
    of this flag if the CPU supports it.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

