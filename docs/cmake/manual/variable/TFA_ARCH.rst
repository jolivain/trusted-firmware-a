TFA_ARCH
========

.. default-domain:: cmake

.. variable:: TFA_ARCH

    Defines the target build architecture for TF-A, either ``aarch64`` or
    ``aarch32``. This value is derived from the value of
    :variable:`CMAKE_SIZEOF_VOID_P <variable:CMAKE_SIZEOF_VOID_P>`:

    - ``4``: :variable:`TFA_ARCH` is ``aarch32``
    - ``8``: :variable:`TFA_ARCH` is ``aarch64``

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
