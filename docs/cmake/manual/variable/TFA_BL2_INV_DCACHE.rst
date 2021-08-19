TFA_BL2_INV_DCACHE
==================

.. default-domain:: cmake

.. variable:: TFA_BL2_INV_DCACHE

Invalidates cached read-write memory used by the BL2 image when running at EL3.
This is done to safeguard against possible corruption of this memory by dirty
cache lines in a system cache as a result of use by an earlier boot loader
stage.

This option is enabled by default, but disabling invalidation may be necessary
if, for example, the caches are not enabled prior to BL2 entry.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
