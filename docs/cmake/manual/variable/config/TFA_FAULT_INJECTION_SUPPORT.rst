TFA_FAULT_INJECTION_SUPPORT
===========================

.. default-domain:: cmake

.. variable:: TFA_FAULT_INJECTION_SUPPORT

ARMv8.4-A extensions introduced support for fault injection from lower ELs,
and this build option enables lower ELs to use Error Records accessed via
System Registers to inject faults.

This feature is intended for testing purposes only, and it is advisable to
keep it disabled for production images.

Disabled by default. Forcibly disabled unless building for AArch64.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
