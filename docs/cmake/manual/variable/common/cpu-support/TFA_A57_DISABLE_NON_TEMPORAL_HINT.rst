TFA_A57_DISABLE_NON_TEMPORAL_HINT
=================================

.. default-domain:: cmake

.. variable:: TFA_A57_DISABLE_NON_TEMPORAL_HINT

Enables the same behaviour as :variable:`A53_DISABLE_NON_TEMPORAL_HINT`
but for the Cortex-A57. This needs to be enabled only for revisions <= r1p2 of
the CPU and is enabled by default, as recommended in section "4.7
Non-Temporal Loads/Stores" of the `Cortex-A57 Software Optimization Guide`_.

.. _Cortex-A57 Software Optimization Guide: http://infocenter.arm.com/help/topic/com.arm.doc.uan0015b/Cortex_A57_Software_Optimization_Guide_external.pdf

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
