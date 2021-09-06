TFA_SKIP_A57_L1_FLUSH_PWR_DWN
=============================

.. default-domain:: cmake

.. variable:: TFA_SKIP_A57_L1_FLUSH_PWR_DWN

Enables an optimization in the Cortex-A57 cluster power down sequence by not
flushing the Level 1 data cache. The L1 data cache and the L2 unified cache
are inclusive. A flush of the L2 by set/way flushes any dirty lines from
the L1 as well. This is a known safe deviation from the Cortex-A57 Technical
Reference Manual (TRM)-defined power down sequence. Each Cortex-A57 based
platform must make its own decision on whether to use the optimization.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
