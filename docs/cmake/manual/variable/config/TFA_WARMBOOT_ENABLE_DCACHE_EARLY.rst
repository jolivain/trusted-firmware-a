TFA_WARMBOOT_ENABLE_DCACHE_EARLY
================================

.. default-domain:: cmake

.. variable:: TFA_WARMBOOT_ENABLE_DCACHE_EARLY

Enables D-caches early on the CPU after warm boot. This is applicable for
platforms which do not require interconnect programming to enable cache
coherency (e.g., single-cluster platforms). If this option is enabled, then
the warm boot path enables D-caches immediately after enabling the MMU.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
