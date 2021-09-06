TFA_SDEI_IN_FCONF
=================

.. default-domain:: cmake

.. variable:: TFA_SDEI_IN_FCONF

    Configures Software Delegated Exception Interface (SDEI) setup at runtime
    using the firmware configuration framework.

    The platform-specific SDEI shared and private events configuration is
    retrieved from the device tree rather than static C structures at compile
    time.

    Disabled by default. Forcibly disabled unless :ref:``TFA_SDEI_SUPPORT`` is
    enabled.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
