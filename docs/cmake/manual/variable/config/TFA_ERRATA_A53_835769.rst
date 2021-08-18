TFA_ERRATA_A53_835769
============================

.. default-domain:: cmake

.. variable:: TFA_ERRATA_A53_835769

This applies erratum 835769 workaround at compile and link time to the
Cortex-A53 CPU. This needs to be enabled for some variants of revision <=
r0p4. This workaround can lead the linker to create ``*.stub`` sections.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

