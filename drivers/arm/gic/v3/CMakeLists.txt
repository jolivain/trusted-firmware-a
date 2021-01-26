#
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(TFAConfigOption)

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GICV3_SUPPORT_GIC600

   Add support for the GIC-600 variants of GICv3. Enabling this option will add runtime detection support for the GIC-600, so is safe to select even for a GIC500 implementation. This option defaults to 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_GICV3_SUPPORT_GIC600
    HELP "Add support for the GIC-600 variants of GICv3. Enabling this option will add runtime detection support for the GIC-600, so is safe to select even for a GIC500 implementation. This option defaults to 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GICV3_IMPL_GIC600_MULTICHIP

   Selects GIC-600 variant with multichip functionality. This option defaults to 0
#]=======================================================================]

tfa_config_option(
    NAME TFA_GICV3_IMPL_GIC600_MULTICHIP
    HELP "Selects GIC-600 variant with multichip functionality. This option defaults to 0")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GICV3_OVERRIDE_DISTIF_PWR_OPS

   Allows override of default implementation of ``arm_gicv3_distif_pre_save`` and ``arm_gicv3_distif_post_restore`` functions. This is required for FVP platform which need to simulate GIC save and restore during SYSTEM_SUSPEND without powering down GIC. Default is 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_GICV3_OVERRIDE_DISTIF_PWR_OPS
    HELP "Allows override of default implementation of ``arm_gicv3_distif_pre_save`` and ``arm_gicv3_distif_post_restore`` functions. This is required for FVP platform which need to simulate GIC save and restore during SYSTEM_SUSPEND without powering down GIC. Default is 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GIC_ENABLE_V4_EXTN

   Enables GICv4 related changes in GICv3 driver. This option defaults to 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_GIC_ENABLE_V4_EXTN
    HELP "Enables GICv4 related changes in GICv3 driver. This option defaults to 0.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_GIC_EXT_INTID

   When set to ``1``, GICv3 driver will support extended PPI (1056-1119) and SPI (4096-5119) range. This option defaults to 0.
#]=======================================================================]

tfa_config_option(
    NAME TFA_GIC_EXT_INTID
    HELP "When set to ``1``, GICv3 driver will support extended PPI (1056-1119) and SPI (4096-5119) range. This option defaults to 0.")

add_library(gic-v3 INTERFACE)

target_compile_definitions(gic-v3
    INTERFACE
        "GIC_ENABLE_V4_EXTN=$<BOOL:${TFA_GIC_ENABLE_V4_EXTN}>"
        "GIC_EXT_INTID=$<BOOL:${TFA_GIC_EXT_INTID}>"
        "GICV3_OVERRIDE_DISTIF_PWR_OPS=$<BOOL:${TFA_GICV3_OVERRIDE_DISTIF_PWR_OPS}>"
        "GICV3_SUPPORT_GIC600=$<BOOL:${TFA_GICV3_SUPPORT_GIC600}>")

target_sources(gic-v3
    INTERFACE
        "gic-x00.c"
        "gicv3_main.c"
        "gicv3_helpers.c"
        "gicdv3_helpers.c"
        "gicrv3_helpers.c")

if(TFA_GICV3_OVERRIDE_DISTIF_PWR_OPS)
    target_sources(gic-v3
        INTERFACE
            "arm_gicv3_common.c")
endif()

if(TFA_GICV3_IMPL_GIC600_MULTICHIP)
    target_sources(gic-v3
        INTERFACE
            "gic600_multichip.c")
endif()
