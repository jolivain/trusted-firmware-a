TFA_BL2_IN_XIP_MEM
==================

.. default-domain:: cmake

.. variable:: TFA_BL2_IN_XIP_MEM

Initializes the read-write sections in RAM while leaving the read-only sections
in place, which is necessary when BL2 is stored in eXecute In Place (XIP)
memory.

This option is only supported in combination with :variable:`TFA_BL2_AT_EL3`,
and is disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
