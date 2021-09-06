TFA_ENABLE_STACK_PROTECTOR
==========================

.. default-domain:: cmake

.. variable:: TFA_ENABLE_STACK_PROTECTOR

Additional stack protection flags, which are passed directly to the compiler.

If stack protection flags are provided, the platform must have implemented the
``plat_get_stack_protector_canary()`` function.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

