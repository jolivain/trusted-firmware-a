TFA_HW_ASSISTED_COHERENCY
=========================

.. default-domain:: cmake

.. variable:: TFA_HW_ASSISTED_COHERENCY


   On most Arm systems to-date, platform-specific software operations are
   required for CPUs to enter and exit coherency. However, newer systems exist
   where CPUs' entry to and exit from coherency is managed in hardware. Such
   systems require software to only initiate these operations, and the rest is
   managed in hardware, minimizing active software management. In such systems,
   this option allows TF-A to carry out build and run-time optimizations during
   boot and power management operations.  is also enabled.

   If this flag is disabled while the platform includes cores that manage
   coherency in hardware, then a compilation error is generated. This is due to
   the fact that a system cannot have, at the same time, cores that manage
   coherency in hardware and cores that don't. In other words, a platform cannot
   have, at the same time, cores that require ``TFA_HW_ASSISTED_COHERENCY=TRUE``
   and cores that require ``TFA_HW_ASSISTED_COHERENCY=FALSE``.

   Disabled by default. Forcibly disabled unless warm boot D-cache early is
   enabled and version 2 of the translation tables library is in use.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
