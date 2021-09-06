TFA_FVP_INTERCONNECT_DRIVER
===========================

.. default-domain:: cmake

.. variable:: TFA_FVP_INTERCONNECT_DRIVER

Selects the interconnect driver to use. The default interconnect driver
depends on the value of ``TFA_FVP_CLUSTER_COUNT`` as explained in the options
below:

-  ``CCI``: The CCI driver is selected. This is the default if 0 <
   ``TFA_FVP_CLUSTER_COUNT`` <= 2.
-  ``CCN``: The CCN driver is selected. This is the default if
   ``TFA_FVP_CLUSTER_COUNT`` > 2.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
