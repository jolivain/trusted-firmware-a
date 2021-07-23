TFA_BL2_AT_EL3
==============

.. default-domain:: cmake

.. variable:: TFA_BL2_AT_EL3

Some platforms have a non-|TF-A| Boot ROM that expects the next boot stage to
execute at EL3. On these platforms, |TF-A|'s BL1 is a waste of memory as its
only purpose is to ensure |TF-A|'s BL2 image is entered at S-EL1. To avoid this
waste, a special mode enables BL2 to execute at EL3, which allows a non-|TF-A|
Boot ROM to load and jump directly to BL2.

For more information on this mode of operation, see the documentation for
:ref:`running BL2 at EL3 <Running BL2 at EL3>`.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
