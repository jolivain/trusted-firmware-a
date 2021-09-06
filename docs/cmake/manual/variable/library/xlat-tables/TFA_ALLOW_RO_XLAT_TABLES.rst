TFA_ALLOW_RO_XLAT_TABLES
========================

.. default-domain:: cmake

.. variable:: TFA_ALLOW_RO_XLAT_TABLES

Dictates whether the translation tables can be placed in the read-only
section of the bootloader binaries (``.rodata``). If not, they are always
placed in the zero-initialized section (``.bss``).

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

