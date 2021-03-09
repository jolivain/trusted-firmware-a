TFA_BL2_IN_XIP_MEM
==================

.. default-domain:: cmake

.. variable:: TFA_BL2_IN_XIP_MEM

In some situations BL2 will be stored in eXecute In Place (XIP) memory, as BL1
is. In these situations, it is necessary to initialize the read-write sections
in RAM while leaving the read-only sections in place, which this option
facilitates.

This option is only supported in combination with :variable:`TFA_BL2_AT_EL3`,
and is disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
