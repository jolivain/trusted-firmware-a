TFA_PSCI_EXTENDED_STATE_ID
==========================

.. default-domain:: cmake

.. variable:: TFA_PSCI_EXTENDED_STATE_ID

    Per the PSCI 1.0 specification, there are two formats possible for the PSCI
    State-ID format: original and extended. When enabled, this option configures
    the generic PSCI layer to use the extended format.

    This option should be overridden by the platform, and it governs the return
    value of the ``PSCI_FEATURES`` API for the ``CPU_SUSPEND`` SMC function ID.

    Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
