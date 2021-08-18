TFA_ARM_RECOM_STATE_ID_ENC
==========================

.. default-domain:: cmake

.. variable:: TFA_ARM_RECOM_STATE_ID_ENC

Configures whether the State-ID encoding recommended in the |PSCI| 1.0
specification will be used or not. If disabled, the platform will expect
``NULL`` in the State-ID field.

.. note::

    Most existing |PSCI| clients currently do not yet support State-ID encoding.

Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

