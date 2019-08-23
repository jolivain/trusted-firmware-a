#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

#TODO: revise config in this file, compare with .mk files

#plat_bl_common
stgt_add_src_param(NAME bl1 bl2 bl31 KEY ARCH SRC
	${TFA_ROOT_DIR}/plat/arm/board/juno/@ARCH@/juno_helpers.S
	${TFA_ROOT_DIR}/plat/arm/board/juno/juno_common.c
)

#plat_bl1
stgt_add_src(NAME bl1 SRC
	${TFA_ROOT_DIR}/plat/arm/board/juno/juno_err.c
	${TFA_ROOT_DIR}/plat/arm/board/juno/juno_bl1_setup.c
	${TFA_ROOT_DIR}/drivers/arm/sp805/sp805.c
)

stgt_add_src_cond(NAME bl1 KEY CSS_USE_SCMI_SDS_DRIVER VAL 1 SRC
	${TFA_ROOT_DIR}/drivers/arm/css/sds/sds.c
)

#plat_bl2
stgt_add_src_param(NAME bl2 KEY ARCH SRC
	${TFA_ROOT_DIR}/drivers/arm/sp805/sp805.c
	${TFA_ROOT_DIR}/lib/utils/mem_region.c
	${TFA_ROOT_DIR}/plat/arm/board/juno/juno_err.c
	${TFA_ROOT_DIR}/plat/arm/board/juno/juno_bl2_setup.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_nor_psci_mem_protect.c
)

#plat_bl31
stgt_add_src_param(NAME bl31 KEY ARCH SRC
	${TFA_ROOT_DIR}/lib/utils/mem_region.c
	${TFA_ROOT_DIR}/plat/arm/board/juno/juno_pm.c
	${TFA_ROOT_DIR}/plat/arm/board/juno/juno_topology.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_nor_psci_mem_protect.c

	#Juno GIC sources
	${TFA_ROOT_DIR}/drivers/arm/gic/common/gic_common.c
	${TFA_ROOT_DIR}/drivers/arm/gic/v2/gicv2_main.c
	${TFA_ROOT_DIR}/drivers/arm/gic/v2/gicv2_helpers.c
	${TFA_ROOT_DIR}/plat/common/plat_gicv2.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_gicv2.c
)

stgt_add_src(NAME bl1 bl31 SRC
	#Juno interconnect sources
	${TFA_ROOT_DIR}/drivers/arm/cci/cci.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_cci.c
)

stgt_add_src(NAME bl1 bl2 bl31 SRC
	#Juno security sources
	${TFA_ROOT_DIR}/drivers/arm/tzc/tzc400.c
	${TFA_ROOT_DIR}/plat/arm/board/juno/juno_security.c
	${TFA_ROOT_DIR}/plat/arm/board/juno/juno_trng.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_tzc400.c
)

stgt_add_src_cond(NAME bl1 bl2 KEY ENABLE_STACK_PROTECTOR VAL 1 SRC
	#Juno security sources
	${TFA_ROOT_DIR}/plat/arm/board/juno/juno_stack_protector.c
)

stgt_add_src(NAME bl1 bl31 SRC
	#CPU libs
	${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a53.S
	${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a57.S
	${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a72.S
)

include(${TFA_ROOT_DIR}/plat/arm/board/common/board_common.cmake)
include(${TFA_ROOT_DIR}/plat/arm/common/arm_common.cmake)
include(${TFA_ROOT_DIR}/plat/arm/soc/common/soc_css.cmake)
include(${TFA_ROOT_DIR}/plat/arm/css/common/css_common.cmake)

#TODO: TRUSTED_BOARD_BOOT, ROMLIB
