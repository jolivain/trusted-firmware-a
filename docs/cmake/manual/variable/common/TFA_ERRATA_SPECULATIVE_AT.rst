TFA_ERRATA_SPECULATIVE_AT
=========================

.. default-domain:: cmake

.. variable:: TFA_ERRATA_SPECULATIVE_AT

   Determines whether to enable the ``AT`` instruction speculation
   errata workaround or not.

   This workaround disables stage 1 page table walks for lower ELs (EL1 and EL0)
   in EL3 so that speculative fetches triggered by an ``AT`` instruction produce
   either the correct result or failure without a TLB allocation. This
   workaround is common to several CPU errata.

   This option is intended to be enabled only by other workarounds that require
   it.

   Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
