TFA_SEPARATE_NOBITS_REGION
==========================

.. default-domain:: cmake

.. variable:: TFA_SEPARATE_NOBITS_REGION

Allows the NOBITS sections of BL31 (``.bss``, stacks, page tables, and
coherent memory) to be allocated in RAM discontiguous from the loaded
firmware image. When enabled, the platform is expected to provide definitions
for ``BL31_NOBITS_BASE`` and ``BL31_NOBITS_LIMIT``. When disabled, ``NOBITS``
sections are placed in RAM immediately following the loaded firmware image.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
