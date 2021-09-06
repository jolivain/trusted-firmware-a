TFA_ARM_BL31_IN_DRAM
====================

.. default-domain:: cmake

.. variable:: TFA_ARM_BL31_IN_DRAM

Loads BL31 into |TZC| secured DRAM. By default, BL31 is in the secure SRAM. If
a |TSP| is present, then enabling this option also forcibly sets the |TSP|
location to DRAM and ignores the :variable:`TFA_ARM_TSP_RAM_LOCATION` build
flag.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

