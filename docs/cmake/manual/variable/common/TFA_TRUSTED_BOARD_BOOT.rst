TFA_TRUSTED_BOARD_BOOT
======================

.. default-domain:: cmake

.. variable:: TFA_TRUSTED_BOARD_BOOT

Includes support for the Trusted Board Boot feature. When enabled, BL1 and
BL2 images include support for loading and verifying the certificates and
images in a FIP, and BL1 includes support for Firmware Update. Generation and
inclusion of certificates in the Firmware Image Package (FIP) and Firmware
Update (FWU) FIP depends upon the value of the :ref:``TFA_GENERATE_COT`` option.

Disabled by default. Forcibly disabled unless :ref:``TFA_CREATE_KEYS`` is
enabled.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
