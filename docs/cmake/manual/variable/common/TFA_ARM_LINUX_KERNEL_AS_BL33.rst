TFA_ARM_LINUX_KERNEL_AS_BL33
============================

.. default-domain:: cmake

.. variable:: TFA_ARM_LINUX_KERNEL_AS_BL33

The Linux kernel expects registers x0-x3 to have specific values at boot.
This option allows |TF-A| to use a Linux kernel image as BL33 by configuring
these registers prior to BL33.

Disabled by default. Forcibly disabled if:

- :variable:`TFA_PRELOADED_BL33_BASE` is disabled
- :variable:`TFA_ARM_PRELOADED_DTB_BASE` is disabled
- :variable:`TFA_RESET_TO_BL31` is disabled when building for |AArch64|
- :variable:`TFA_RESET_TO_SP_MIN` is disabled when building for |AArch32|

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

