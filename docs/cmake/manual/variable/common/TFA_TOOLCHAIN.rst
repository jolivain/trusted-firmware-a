TFA_TOOLCHAIN
=============

.. default-domain:: cmake

.. variable:: TFA_TOOLCHAIN

Name of the toolchain to compile |TF-A| with. Supported toolchains are as
follows:

+-----------+---------------------------+----------------+
| Toolchain | :variable:`TFA_TOOLCHAIN` | Toolchain file |
+===========+===========================+================+
| ...       | ...                       | ...            |
+-----------+---------------------------+----------------+

.. note::

    If :variable:`CMAKE_TOOLCHAIN_FILE <variable:CMAKE_TOOLCHAIN_FILE>` is
    provided directly, this option is ignored.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
