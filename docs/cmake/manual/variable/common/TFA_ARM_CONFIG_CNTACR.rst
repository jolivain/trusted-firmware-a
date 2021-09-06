TFA_ARM_CONFIG_CNTACR
=====================

.. default-domain:: cmake

.. variable:: TFA_ARM_CONFIG_CNTACR

Unlocks access to the ``CNTBase<N>`` frame registers by setting the
``CNTCTLBase.CNTACR<N>`` register bits. The frame number ``<N>`` is defined
by ``PLAT_ARM_NSTIMER_FRAME_ID``, which should match the frame used by the
Non-Secure image (normally the Linux kernel).

Enabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

