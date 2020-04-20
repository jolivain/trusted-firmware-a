DTB binding for framework properties
====================================

This document defines how the properties related to |FCONF| should be
represented in device tree format. These properties are not related to a
specific platform and can be queried from common code.

Dynamic configuration
~~~~~~~~~~~~~~~~~~~~~

The framework expect a *dtb-registry* node with the following field:

- compatible [mandatory]
   - value type: <string>
   - Must be the string "fconf,dyn_cfg-dtb_registry".

Every configuration node must be formed with the following field:

- load-address [mandatory]
    - value type: <u64>
    - Physical loading base address of the configuration.

- max-size [mandatory]
    - value type: <u32>
    - Maximum size of the configuration.

- id [mandatory]
    - value type: <u32>
    - Image ID of the configuration.