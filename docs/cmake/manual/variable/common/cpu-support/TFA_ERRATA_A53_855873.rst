TFA_ERRATA_A53_855873
=====================

.. default-domain:: cmake

.. variable:: TFA_ERRATA_A53_855873

This applies errata 855873 workaround to the Cortex-A53 CPUs. Though the
erratum is present in every revision of the CPU, this workaround is only
applied to CPUs from r0p3 onwards, which feature a chicken bit in
`CPUACTLR_EL1` to enable a hardware workaround. Earlier revisions of the CPU
have other errata which require the same workaround in software, so they
should be covered anyway.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
