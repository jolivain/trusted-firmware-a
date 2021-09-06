TFA_ARM_RECOM_STATE_ID_ENC
==========================

.. default-domain:: cmake

.. variable:: TFA_ARM_RECOM_STATE_ID_ENC

The |PSCI| 1.0 specification recommends an encoding for the construction of
composite State-IDs, but most existing |PSCI| clients currently do yet support
State-ID encoding yet. This option is used to configure whether to use the
recommended State-ID encoding or not. If disabled, the platform is configured
to expect ``NULL`` in the State-ID field.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

