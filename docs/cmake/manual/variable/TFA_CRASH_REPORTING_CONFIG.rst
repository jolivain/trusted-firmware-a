TFA_CRASH_REPORTING_<CONFIG>
============================

.. default-domain:: cmake

.. variable:: TFA_CRASH_REPORTING_<CONFIG>

Enables a console dump of processor register state when an unexpected exception
occurs during execution of BL31. This is initialized for each of the supported
build modes:

- :variable:`TFA_CRASH_REPORTING_Debug <TFA_CRASH_REPORTING_<CONFIG>>`
- :variable:`TFA_CRASH_REPORTING_RelWithDebInfo <TFA_CRASH_REPORTING_<CONFIG>>`
- :variable:`TFA_CRASH_REPORTING_MinSizeRel <TFA_CRASH_REPORTING_<CONFIG>>`
- :variable:`TFA_CRASH_REPORTING_Release <TFA_CRASH_REPORTING_<CONFIG>>`

By default, crash reporting is only enabled for the ``Debug`` build mode.

For more information on the crash reporting mechanism, see the documentation for
:ref:`Crash Reporting in BL31`.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
