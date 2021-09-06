TFA_RECLAIM_INIT_CODE
=====================

.. default-domain:: cmake

.. variable:: TFA_RECLAIM_INIT_CODE

    A significant amount of the code used for the initialization of BL31 is not
    needed again after boot time. In order to reduce the runtime memory
    footprint, the memory used for this code can be reclaimed after
    initialization. This option enables this reclamation.

    Certain boot-time functions are marked with the ``__init`` attribute to
    enable this reclamation.

    Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
