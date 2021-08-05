TFA_ENABLE_BACKTRACE
====================

.. default-domain:: cmake

.. variable:: TFA_ENABLE_BACKTRACE

Enables backtraced crash dumps.

Note that this option is supported for both AArch64 and AArch32, but in AArch32
the format of frame records is not defined in the |AAPCS| and is instead defined
by the implementation. The AArch32 backtracing implementation used by |TF-A|
only supports the format used by GCC with |A32| code generation, so this option
is therefore only available if :variable:`TFA_ARCH_STATE_AARCH32_THUMB` is
disabled.

Enabled by default unless building for AArch32.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
