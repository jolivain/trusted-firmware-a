TFA_EL3_EXCEPTION_HANDLING
==========================

.. default-domain:: cmake

.. variable:: TFA_EL3_EXCEPTION_HANDLING

Enables handling of exceptions targeted at EL3.

.. warning::

    When disabled no exceptions are expected or handled at EL3, and a panic will
    result.

Disabled by default. Forcibly disabled if not building for |AArch64|.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

