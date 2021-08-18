TFA_ENABLE_MPAM_FOR_LOWER_ELS
=============================

.. default-domain:: cmake

.. variable:: TFA_ENABLE_MPAM_FOR_LOWER_ELS

Enables |MPAM| feature in ELs lower than EL3. |MPAM| is an optional Armv8.4-a
extension that enables various memory system components and resources to
define partitions; software running at various ELs can assign themselves to a
desired partition to control their performance aspects.

When this option is enabled, EL3 allows lower ELs to access their own |MPAM|
registers without trapping into EL3. However, this option doesn't make use of
partitioning in EL3. Platform initialization code should configure and use
partitions in EL3 as required.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

