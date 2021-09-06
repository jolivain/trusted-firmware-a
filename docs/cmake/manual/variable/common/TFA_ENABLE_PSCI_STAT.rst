TFA_ENABLE_PSCI_STAT
====================

.. default-domain:: cmake

.. variable:: TFA_ENABLE_PSCI_STAT

Enables support for the optional |PSCI| functions ``PSCI_STAT_RESIDENCY`` and
``PSCI_STAT_COUNT``. In the absence of an alternative statistics collection
backend, :variable:`TFA_ENABLE_PMF` must be enabled. If :variable:`TFA_ENABLE_PMF`
is enabled, the residency statistics are tracked in software.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
