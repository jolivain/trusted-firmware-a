#
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-driver-gic-v3 INTERFACE)

target_sources(tfa-driver-gic-v3
    INTERFACE "gic-x00.c"
              "gicv3_main.c"
              "gicv3_helpers.c"
              "gicdv3_helpers.c"
              "gicrv3_helpers.c")

if(NOT TFA_GICV3_OVERRIDE_DISTIF_PWR_OPS)
    target_sources(tfa-driver-gic-v3
        INTERFACE "arm_gicv3_common.c")
endif()

if(TFA_GICV3_IMPL_GIC600_MULTICHIP)
    target_sources(tfa-driver-gic-v3
        INTERFACE "gic600_multichip.c")
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GICV3_SUPPORT_GIC600

   Add support for the GIC-600 variants of GICv3. Enabling this option will add
   runtime detection support for the GIC-600, so is safe to select even for a
   GIC-500 implementation.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_GICV3_SUPPORT_GIC600
    HELP "Enable GIC-600 support?")

target_compile_definitions(tfa-driver-gic-v3
    INTERFACE "GICV3_SUPPORT_GIC600=$<BOOL:${TFA_GICV3_SUPPORT_GIC600}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GICV3_IMPL_GIC600_MULTICHIP

   Selects the GIC-600 variant with multichip functionality.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_GICV3_IMPL_GIC600_MULTICHIP
    HELP "Enable GIC-600 multichip support?")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GICV3_OVERRIDE_DISTIF_PWR_OPS

   Allows overriding of the default implementations of the
   ``arm_gicv3_distif_pre_save`` and ``arm_gicv3_distif_post_restore``
   functions.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_GICV3_OVERRIDE_DISTIF_PWR_OPS
    HELP "Allow overriding of GIC distributor interface power operations?")

target_compile_definitions(tfa-driver-gic-v3
    INTERFACE "GICV3_OVERRIDE_DISTIF_PWR_OPS=$<BOOL:${TFA_GICV3_OVERRIDE_DISTIF_PWR_OPS}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GIC_ENABLE_V4_EXTN

   Enables GICv4-related changes in the GICv3 driver.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_GIC_ENABLE_V4_EXTN
    HELP "Enable GICv4-related changes in the GICv3 driver?")

target_compile_definitions(tfa-driver-gic-v3
    INTERFACE "GIC_ENABLE_V4_EXTN=$<BOOL:${TFA_GIC_ENABLE_V4_EXTN}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GIC_EXT_INTID

   When enabled, the GICv3 driver will support extended PPI (1056-1119) and SPI
   (4096-5119) ranges.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_GIC_EXT_INTID
    HELP "Enable GICv3 extended PPI and SPI ranges?")

target_compile_definitions(tfa-driver-gic-v3
    INTERFACE "GIC_EXT_INTID=$<BOOL:${TFA_GIC_EXT_INTID}>")
