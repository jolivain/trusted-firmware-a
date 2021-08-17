TFA_SEC_INT_DESC_IN_FCONF
=========================

.. default-domain:: cmake

.. variable:: TFA_SEC_INT_DESC_IN_FCONF

Determines whether to configure Group 0 and Group 1 secure interrupts using
the firmware configuration framework. The platform-specific secure interrupt
property descriptor is retrieved from the device tree at runtime rather than
depending on static C structures at compile-time.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
