TFA_COLD_BOOT_SINGLE_CPU
========================

.. default-domain:: cmake

.. variable:: TFA_COLD_BOOT_SINGLE_CPU

Indicates whether or not the platform will release just a single core out of
reset, as opposed to several simultaneously.

If the platform always brings up a single CPU, there is no need to distinguish
between primary and secondary CPUs, and so the boot path can be optimized. The
``plat_is_my_cpu_primary()`` and ``plat_secondary_cold_boot_setup()`` platform
porting interfaces do not need to be implemented in this case.

See the :ref:`cold boot documentation <Cold Boot>` for more information.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
