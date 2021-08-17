TFA_PLATFORM
============

.. default-domain:: cmake

.. variable:: TFA_PLATFORM

The name of the target platform. The list of in-tree platform names is as
follows:

+--------+------------------------+--------------------------+
| Vendor | Platform               | :variable:`TFA_PLATFORM` |
+========+========================+==========================+
| Arm    | |FVP|                  | ``FVP``                  |
+        +------------------------+--------------------------+
|        | Juno Development Board | ``Juno``                 |
+--------+------------------------+--------------------------+

.. note::

    Out-of-tree platforms must define this in their :ref:`Platform Metadata`
    file.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
