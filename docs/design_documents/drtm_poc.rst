DRTM Proof of Concept
=====================

Dynamic Root of Trust for Measurement (DRTM) begins a new trust environment
trust by measuring and executing a protected payload.

SRTM, or measured boot implementation, currently used by TF-A covers all
firmware, from the boot ROM to the normal world bootloader. As a whole, they
make up the system's TCB. These boot measurements allow attesting to what
software is running on the system and enable enforcing security policies.

As the boot chain grows or firmware becomes dynamically extensible,
establishing an attestable TCB becomes more challenging. DRTM  provides a
solution to this problem by allowing measurement chains to be started at
any time. As these measurements are stored separately from the boot-time
measurements, they reduce the size of the TCB, which helps reduce the attack
surface and the risk of untrusted code executing, which could compromise
the security of the system.

Components
~~~~~~~~~~
   - **D-CRTM and DCE**: These components are part of BL31, and
     handle SMCs fired from DCE-Preamble and DLME.
     They measures various components mentioned in `DRTM-specification`_
     They prepares memory region for target payload(DLME), and transfer
     control to it.

   - **DCE-Preamble and DLME**: In this proof of concept, these components
     are part of UEFI application. DCE-Preamble prepares the platform
     for DRTM by preparing input parameters needed by DRTM, loading
     the target payload image(DLME).
     DCE provides data to DLME that it can use to verify the configuration
     of the system.

This manual provides instructions to build TF-A code with pre-buit EDK2
and DRTM UEFI application.

Building the PoC for the Arm FVP platform
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

(1) Obtain EDK2 bin *FVP_AARCH64_EFI.fd* from the `prebuilts-drtm-bins`_

(2) Use the below command to clone TF-A source code -

.. code:: shell

   $ git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git

(3) There are prebuilt binaries required to execute the DRTM implementation
    in the `prebuilts-drtm-bins`_.
    Download EDK2  *FVP_AARCH64_EFI.fd* and UEFI DRTM application *test-disk.img*
    binary from `prebuilts-drtm-bins`_.

(4) Build the TF-A code using below command

.. code:: shell

   $ make CROSS_COMPILE=aarch64-none-elf- ARM_ROTPK_LOCATION=devel_rsa
     DEBUG=1 V=1 BL33=</path/to/FVP_AARCH64_EFI.fd> DRTM_SUPPORT=1
     MBEDTLS_DIR=</path/to/mbedTLS-source> USE_ROMLIB=1 all fip

Running DRTM UEFI application on the Armv8-A AEM FVP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
To run the DRTM test application along with DRTM implementation in BL31,
you need an FVP model. Please use the version of FVP_Base_RevC-2xAEMvA model
advertised in the TF-A documentation.

.. code:: shell

    FVP_Base_RevC-2xAEMvA \
    --data cluster0.cpu0= </path/to/romlib.bin>@0x03ff2000 \
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
following to indicate that last SMC to unprotect memory has been fired
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
