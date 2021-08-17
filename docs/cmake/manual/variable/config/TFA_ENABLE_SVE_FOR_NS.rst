TFA_ENABLE_SVE_FOR_NS
=====================

.. default-domain:: cmake

.. variable:: TFA_ENABLE_SVE_FOR_NS

Enables |SVE| for the Normal world only. |SVE| is an
optional architectural feature for |AArch64|. Note that when |SVE| is enabled
for the Normal world, access to SIMD and floating-point functionality from the
Secure world is disabled. This is to avoid corruption of the Normal world
data in the Z-registers, which are aliased by the |SIMD| and Floating Point
registers.

Enabled by default. Forcibly disabled if :variable:`TFA_CTX_INCLUDE_FPREGS` is
enabled, or if building for |AArch32|.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

