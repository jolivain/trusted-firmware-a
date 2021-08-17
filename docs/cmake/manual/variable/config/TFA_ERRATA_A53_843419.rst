TFA_ERRATA_A53_843419
=====================

.. default-domain:: cmake

.. variable:: TFA_ERRATA_A53_843419

This applies erratum 843419 workaround at link time to the Cortex-A53 CPU.
This needs to be enabled for some variants of revision <= r0p4. This
workaround can lead the linker to emit ``*.stub`` sections which are 4kB
aligned.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

