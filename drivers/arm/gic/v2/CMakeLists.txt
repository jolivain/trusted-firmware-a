#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-driver-gic-v2 INTERFACE)

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GICV2_G0_FOR_EL3

   Unlike GICv3, the GICv2 architecture doesn't have inherent support for
   specific EL3 type interrupts. Enabling this option assumes GICv2 *Group 0*
   interrupts are expected to target EL3, both by
   :ref:`platform abstraction layer<platform Interrupt Controller API>` and
   :ref:`Interrupt Management Framework<Interrupt Management Framework>`. This
   allows GICv2 platforms to enable features requiring EL3 interrupt type. This
   also means that all GICv2 Group 0 interrupts are delivered to EL3, and the
   Secure Payload interrupts needs to be synchronously handed over to Secure EL1
   for handling.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_GICV2_G0_FOR_EL3
    HELP "Assume GICv2 Group 0 interrupts are handled by EL3?")

target_compile_definitions(tfa-driver-gic-v2
    INTERFACE "TFA_${GICV2_G0_FOR_EL3}=$<BOOL:${TFA_GICV2_G0_FOR_EL3}>")
