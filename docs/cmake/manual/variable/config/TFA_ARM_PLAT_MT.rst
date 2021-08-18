TFA_ARM_PLAT_MT
===============

.. default-domain:: cmake

.. variable:: TFA_ARM_PLAT_MT

Determines whether the Arm platform layer has to cater for the
multi-threading ``MT`` bit when accessing the |MPIDR|. When enabled, the
functions which deal with the |MPIDR| assume that the ``MT`` bit in |MPIDR| is
set and access the bit-fields in |MPIDR| accordingly.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

