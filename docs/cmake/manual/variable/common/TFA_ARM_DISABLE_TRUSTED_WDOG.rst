TFA_ARM_DISABLE_TRUSTED_WDOG
============================

.. default-domain:: cmake

.. variable:: TFA_ARM_DISABLE_TRUSTED_WDOG

Disables the Trusted Watchdog. By default, Arm platforms use a watchdog to
trigger a system reset in case an error is encountered during the boot
process (for example, when an image could not be loaded or authenticated).
The watchdog is enabled in the early platform setup hook at BL1 and disabled
in the BL1 prepare exit hook. The Trusted Watchdog may be disabled at build
time for testing or development purposes.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

