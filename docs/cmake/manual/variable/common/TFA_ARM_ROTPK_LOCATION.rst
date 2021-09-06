TFA_ARM_ROTPK_LOCATION
======================

.. default-domain:: cmake

.. variable:: TFA_ARM_ROTPK_LOCATION

Used when :variable:`TFA_TRUSTED_BOARD_BOOT` is set. It specifies the location of
the ROTPK hash returned by the function ``plat_get_rotpk_info()`` for Arm
platforms.
Depending on the selected option, the proper private key must be specified using
the :variable:`TFA_ROT_KEY` option when building the Trusted Firmware. This
private key will be used by the certificate generation tool to sign the BL2 and
Trusted Key certificates. Available options for
:variable:`TFA_ARM_ROTPK_LOCATION` are:

* ``regs``: return the ROTPK hash stored in the Trusted root-key storage registers.
* ``devel_rsa``: return a development public key hash embedded in the BL1 and BL2 binaries. This hash has been obtained from the RSA public key ``arm_rotpk_rsa.der``, located in ``plat/arm/board/common/rotpk``. To use this option, ``arm_rotprivk_rsa.pem`` must be specified as :variable:`TFA_ROT_KEY` when creating the certificates.
* ``devel_ecdsa``: return a development public key hash embedded in the BL1 and BL2 binaries. This hash has been obtained from the ECDSA public key ``arm_rotpk_ecdsa.der``, located in ``plat/arm/board/common/rotpk``. To use this option, ``arm_rotprivk_ecdsa.pem`` must be specified as :variable:`TFA_ROT_KEY` when creating the certificates.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

