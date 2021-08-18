TFA_ARM_CRYPTOCELL_INTEG
========================

.. default-domain:: cmake

.. variable:: TFA_ARM_CRYPTOCELL_INTEG

Allows |TF-A| to invoke Arm® TrustZone® CryptoCell functionality for Trusted
Board Boot on capable Arm platforms. If this option is specified, then the
path to the CryptoCell SBROM library must be specified via
:variable:`TFA_CCSBROM_LIB_PATH`.

Disabled by default. Forcibly disabled if :variable:`TFA_USE_COHERENT_MEM` is
disabled.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

