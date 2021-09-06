TFA_SPD
=======

.. default-domain:: cmake

.. variable:: TFA_SPD

The Secure Payload Dispatcher component to be built into TF-A. Supported
values are:

- ``None``: no Secure Payload Dispatcher.

The SPM Dispatcher cannot be enabled when the
``SPM_MM`` option is enabled.

Defaults to ``None``. Forcibly set to ``None`` if not building for AArch64,
if :ref:``TFA_EL3_PAYLOAD_BASE`` is enabled, or if ``SPM_MM`` is enabled.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
