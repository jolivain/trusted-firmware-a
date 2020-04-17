TC0 Total Compute Platform
==========================

Some of the features of TC0 platform referenced in TF-A include:

- A `System Control Processor <https://github.com/ARM-software/SCP-firmware>`_
  to abstract abstract power and system management tasks away from application
  processors. The ram firmware for SCP is included in the TF-A FIP and is
  loaded by BL2 from flash in FIP to SRAM for copying by SCP.
- GICv4
- Trusted Board Boot
- SCMI
- MHUv2

Boot Sequence
-------------

The execution begins from SCP_BL1. SCP_BL1 powers up the AP which starts
executing AP_BL1 and then AP_BL2 which loads the SCP_BL2 from FIP to SRAM
using SDS for SCP to copy. The AP then continues executing the rest of TF-A
stages including BL31 runtime stage and hands of executing to
Non-secure world (u-boot).

Build Procedure (TF-A only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Obtain arm `toolchain <https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads>`_.
   Set the CROSS_COMPILE environment variable to point to the toolchain folder.

-  Build TF-A:

   .. code:: shell

      make -j 56 PLAT=tc0 ARCH=aarch64 BL33=<path_to_uboot.bin> \
      SCP_BL2=<path_to_scp_ramfw.bin>  all fip

   Enable TBBR by adding the following options to the make command:

   .. code:: shell

      MBEDTLS_DIR=<path_to_mbedtls_directory>  \
      TRUSTED_BOARD_BOOT=1 \
      GENERATE_COT=1 \
      ARM_ROTPK_LOCATION=devel_rsa  \
      ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem

*Copyright (c) 2020, Arm Limited. All rights reserved.*
