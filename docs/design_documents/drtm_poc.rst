DRTM execution flow
===================

Dynamic Root of Trust for Measurement (DRTM) begins a new chain of
trust by measuring and executing a protected payload.
DRTM is useful in case of number of components in boot-chain grows
as it helps to reduce the attack surface and the risk of executing
un-trusted code compromising the security.

Components
~~~~~~~~~~
   - **D-CRTM and DCE**: These components are part of BL31, and
     handles SMCs fired from DCE-Preamble and DLME.
     It measures various componets mentioned in `DRTM-specification`_
     It prepares memory region for target payload(DLME), and transfer
     control to it.

   - **DCE-Preamble and DLME**: As a part of PoC, these components
     are part of UEFI application. DCE-Preamble prepares platform
     for DRTM by preparing input parameters needed by DRTM, loading
     the target payload image(DLME).

This manual provides instructions to build TF-A code with pre-buit EDK2
and DRTM UEFI application.

Building the PoC for the Arm FVP platform
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

(1) Obtain EDK2 bin *FVP_AARCH64_EFI.fd* from the `prebuilts-drtm-bins`_

(2) Checkout the TF-A DRTM code using below command -

.. code:: shell

   $ git fetch ssh://<user-name>@review.trustedfirmware.org:29418/TF-A/trusted-firmware-a refs/changes/45/15745/5 && git checkout FETCH_HEAD

(3) Build the TF-A code using below command

.. code:: shell

   $ make CROSS_COMPILE=aarch64-none-elf- ARM_ROTPK_LOCATION=devel_rsa
     DEBUG=1 V=1 BL33=</path/to/FVP_AARCH64_EFI.fd> DRTM_SUPPORT=1
     MBEDTLS_DIR=~/mbedtls-2.28.0 USE_ROMLIB=1 all fip

(4) Obtain pre-built binary *test-disk.img* containing UEFI DRTM
    application from `prebuilts-drtm-bins`_

Running DRTM UEFI application on the Armv8-A AEM FVP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
To run the DRTM test application along with DRTM implementation in BL31,
you need an FVP model. Please use the :ref:`latest version
<Arm Fixed Virtual Platforms (FVP)>` of *FVP_Base_RevC-2xAEMvA* model.

.. code:: shell

    FVP_Base_RevC-2xAEMvA \
    --data cluster0.cpu0=romlib.bin@0x03ff2000 \
    --stat \
    -C bp.flashloader0.fname=<path/to/fip.bin> \
    -C bp.secureflashloader.fname=<path/to/bl1.bin> \
    -C bp.ve_sysregs.exit_on_shutdown=1 \
    -C bp.virtioblockdevice.image_path=<path/to/test-disk.img> \
    -C cache_state_modelled=0 \
    -C cluster0.check_memory_attributes=0 \
    -C cluster0.cpu0.etm-present=0 \
    -C cluster0.cpu1.etm-present=0 \
    -C cluster0.cpu2.etm-present=0 \
    -C cluster0.cpu3.etm-present=0 \
    -C cluster0.stage12_tlb_size=1024 \
    -C cluster1.check_memory_attributes=0 \
    -C cluster1.cpu0.etm-present=0 \
    -C cluster1.cpu1.etm-present=0 \
    -C cluster1.cpu2.etm-present=0 \
    -C cluster1.cpu3.etm-present=0 \
    -C cluster1.stage12_tlb_size=1024 \
    -C pctl.startup=0.0.0.0 \
    -Q 1000 \
    "$@"

The bottom of the output from *uart1* should look something like the
following to indicate last SMC to unprotect memory been fired
successfully.

.. code-block:: shell

 ...

 INFO:    DRTM service handler: version
 INFO:    ++ DRTM service handler: TPM features
 INFO:    ++ DRTM service handler: Min. mem. requirement features
 INFO:    ++ DRTM service handler: DMA protection features
 INFO:    ++ DRTM service handler: Boot PE ID features
 INFO:    ++ DRTM service handler: TCB-hashes features
 INFO:    DRTM service handler: dynamic launch
 INFO:    DRTM service handler: close locality
 INFO:    DRTM service handler: unprotect mem

--------------

*Copyright (c) 2022, Arm Limited. All rights reserved.*

.. _prebuilts-drtm-bins: https://downloads.trustedfirmware.org/tf-a/drtm
.. _DRTM-specification: https://developer.arm.com/documentation/den0113/a
