TFA_OPTEE_SP_FW_CONFIG
======================

.. default-domain:: cmake

.. variable:: TFA_OPTEE_SP_FW_CONFIG

    Device Tree Compiler (DTC) build flag to include Open Portable
    Trusted Execution Environment (OP-TEE) as Secure Partition (SP)
    in the tb_fw_config Device Tree.

    This flag is defined only when ``ARM_SPMC_MANIFEST_DTS`` manifest
    file name contains pattern optee_sp.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
