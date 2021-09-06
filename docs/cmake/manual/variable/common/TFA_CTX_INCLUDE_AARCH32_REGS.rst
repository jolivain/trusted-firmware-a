TFA_CTX_INCLUDE_AARCH32_REGS
============================

.. default-domain:: cmake

.. variable:: TFA_CTX_INCLUDE_AARCH32_REGS

Provides context save/restore operations for the |AArch32| system
registers. The option must not be enabled for |AArch64| only platforms (that
is, on hardware that does not implement |AArch32|, or at least not at EL1 and
higher ELs).

Enabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

