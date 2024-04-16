RD-1 AE (Kronos) Platform
============================

Some of the features of the RD-1 AE platform referenced in TF-A include:

- Neoverse-V3AE, Arm9.2-A application processor (64-bit mode)
- A GICv4-compatible GIC-720AE

Further information on RD1-AE is available at `rd1ae`_

Boot Sequence
-------------

BL2 –> BL31 –> BL33

The boot process starts from RSE (Runtime Security Engine) that loads the BL2 image
and signals the System Control Processor (SCP) to power up the Application Processor (AP)
which then executes BL2. BL2 copies the BL31 and continues executing the remaining Trusted
Firmware-A (TF-A) stages, including the BL31 runtime stage. Finally, it hands off
execution to the non-secure world BL33 (u-boot).

Build Procedure (TF-A only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Obtain `Arm toolchain`_ and set the CROSS_COMPILE environment variable to
   point to the toolchain folder.

-  Build TF-A:

   .. code:: shell

      make PLAT=rd1ae BL33=<path_to_uboot.bin>

*Copyright (c) 2024, Arm Limited. All rights reserved.*

.. _Arm Toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads

.. _rd1ae: https://developer.arm.com/Tools%20and%20Software/Arm%20Reference%20Design-1%20AE
