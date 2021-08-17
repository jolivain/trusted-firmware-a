TFA_SPIN_ON_BL1_EXIT
====================

.. default-domain:: cmake

.. variable:: TFA_SPIN_ON_BL1_EXIT

Introduces an infinite loop in BL1, and is disabled by default.
This loop stops execution in BL1 just before handing over to BL31. At this
point, all firmware images have been loaded into memory, and the MMU and
caches are turned off.

Refer to the :ref:`Debugging options` section for more details.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
