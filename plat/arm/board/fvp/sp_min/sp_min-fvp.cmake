#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED PROJECT_SOURCE_DIR)
    message(FATAL_ERROR "PROJECT_SOURCE_DIRnot defined")
endif()

# SP_MIN source files specific to FVP platform
stgt_add_src(
    NAME
    bl32
    SRC
    ${PROJECT_SOURCE_DIR}/drivers/arm/fvp/fvp_pwrc.c
    ${PROJECT_SOURCE_DIR}/lib/utils/mem_region.c
    ${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/aarch32/fvp_helpers.S
    ${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_pm.c
    ${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_console.c
    ${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_topology.c
    ${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/sp_min/fvp_sp_min_setup.c
    ${PROJECT_SOURCE_DIR}/plat/arm/common/arm_nor_psci_mem_protect.c)

target_link_libraries(bl32 PRIVATE common v2m)

stgt_add_sourcelist(
    NAME
    bl32
    SRCLIST
    FVP_CPU_LIBS
    FVP_GIC_SOURCES
    FVP_INTERCONNECT_SOURCES
    FVP_SECURITY_SOURCES)

# Support for fconf in SP_MIN(BL32)
stgt_add_src_cond(
    NAME
    bl32
    KEY
    BL2_AT_EL3
    RESET_TO_SP_MIN
    VAL
    0
    0
    SRC
    ${PROJECT_SOURCE_DIR}/common/fdt_wrappers.c
    ${PROJECT_SOURCE_DIR}/lib/fconf/fconf.c
    ${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fconf/fconf_hw_config_getter.c)

stgt_add_src_cond(
    NAME
    bl32
    KEY
    BL2_AT_EL3
    RESET_TO_SP_MIN
    SEC_INT_DESC_IN_FCONF
    VAL
    0
    0
    1
    SRC
    ${PROJECT_SOURCE_DIR}/plat/arm/common/fconf/fconf_sec_intr_config.c)

include(${PROJECT_SOURCE_DIR}/plat/arm/common/sp_min/arm_sp_min.cmake)
