TFA_RAS_EXTENSION
=================

.. default-domain:: cmake

.. variable:: TFA_RAS_EXTENSION

    Enables Reliability, Availability, and Serviceability (RAS) features.

    RAS is an optional extension for Armv8-A CPUs prior to Armv8.2-A,
    at which point it became mandatory.

    Disabled by default. Forcibly disabled if :ref:``TFA_HANDLE_EA_EL3_FIRST`` is
    disabled.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
