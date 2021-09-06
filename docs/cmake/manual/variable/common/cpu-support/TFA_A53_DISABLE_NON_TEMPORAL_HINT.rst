TFA_A53_DISABLE_NON_TEMPORAL_HINT
=================================

.. default-domain:: cmake

.. variable:: TFA_A53_DISABLE_NON_TEMPORAL_HINT

Disables the cache non-temporal hint. The LDNP/STNP instructions as implemented
on the Cortex-A53 do not behave in a way most programmers
expect, and will most probably result in a significant speed degradation to
any code that employs them. The Armv8-A architecture (see `Arm DDI`_ 0487A.h,
section D3.4.3) allows cores to ignore the non-temporal hint and treat
LDNP/STNP as LDP/STP instead. Enabling this option enforces this behaviour.

This needs to be enabled only for revisions <= r0p3 of the CPU and is enabled
by default.

.. _ARM DDI: https://developer.arm.com/documentation/ddi0487/latest

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
