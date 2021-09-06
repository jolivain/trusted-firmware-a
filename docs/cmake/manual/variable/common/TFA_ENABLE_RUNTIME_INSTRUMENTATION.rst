TFA_ENABLE_RUNTIME_INSTRUMENTATION
==================================

.. default-domain:: cmake

.. variable:: TFA_ENABLE_RUNTIME_INSTRUMENTATION

Enables runtime instrumentation, which injects timestamp collection points
into |TF-A| to allow runtime performance to be measured. Currently, only |PSCI|
is instrumented.

Disabled by default. Forcibly disabled unless the |PMF| is enabled.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

