#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

# SP_MIN source files specific to FVP platform
stgt_add_src(NAME bl32 SRC
	${TFA_ROOT_DIR}/drivers/arm/fvp/fvp_pwrc.c
	${TFA_ROOT_DIR}/drivers/cfi/v2m/v2m_flash.c
	${TFA_ROOT_DIR}/lib/utils/mem_region.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/aarch32/fvp_helpers.S
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_pm.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_console.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_topology.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/sp_min/fvp_sp_min_setup.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_nor_psci_mem_protect.c
)

stgt_add_sourcelist(NAME bl32 SRCLIST
	FVP_CPU_LIBS
	FVP_GIC_SOURCES
	FVP_INTERCONNECT_SOURCES
	FVP_SECURITY_SOURCES
)

# Support for fconf in SP_MIN(BL32)
stgt_add_src_cond(
	NAME bl32
	KEY BL2_AT_EL3 RESET_TO_SP_MIN
	VAL 0          0
	SRC
	${TFA_ROOT_DIR}/common/fdt_wrappers.c
	${TFA_ROOT_DIR}/lib/fconf/fconf.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fconf/fconf_hw_config_getter.c
)

stgt_add_src_cond(
	NAME bl32
	KEY BL2_AT_EL3 RESET_TO_SP_MIN SEC_INT_DESC_IN_FCONF
	VAL 0          0               1
	SRC
	${TFA_ROOT_DIR}/plat/arm/common/fconf/fconf_sec_intr_config.c
)

include(${TFA_ROOT_DIR}/plat/arm/common/sp_min/arm_sp_min.cmake)
