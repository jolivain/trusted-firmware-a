TFA_COT_DESC_IN_DTB
===================

.. default-domain:: cmake

.. variable:: TFA_COT_DESC_IN_DTB

Determines whether |CoT| descriptors are created at runtime using
the firmware configuration framework. If enabled, |CoT| descriptors are
statically captured in the ``tb_fw_config`` file in the form of device tree
nodes and properties. Currently, |CoT| descriptors used by BL2 are moved to the
device tree and |CoT| descriptors used by BL1 are retained in the code base
statically.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

