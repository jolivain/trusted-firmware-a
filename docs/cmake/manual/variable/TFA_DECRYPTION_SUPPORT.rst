TFA_DECRYPTION_SUPPORT
======================

.. default-domain:: cmake

.. variable:: TFA_DECRYPTION_SUPPORT

Selects the encryption algorithm firmware images will be encrypted with, or
decrypted with during the |TBB| boot process. An empty value disables image
encryption entirely. Supported values are:

- ``AesGcm``: Uses the AES GCM algorithm.

By default this option is left empty, disabling decryption. For more information
on the implications of this option, see the :ref:`Trusted Board Boot`
documentation.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
