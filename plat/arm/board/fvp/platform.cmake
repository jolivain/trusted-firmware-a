#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED PROJECT_SOURCE_DIR)
	message(FATAL_ERROR "PROJECT_SOURCE_DIR not defined")
endif()

# Include framework files
include(Common/STGT)

#plat_bl_common
stgt_add_src(NAME bl1 bl2 bl31 SRC
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_common.c
	#TODO: stack protector
)

#FVP CPU libs
stgt_add_src_param(NAME bl1 bl31 KEY ARCH SRC
	${PROJECT_SOURCE_DIR}/lib/cpus/@ARCH@/aem_generic.S
)

stgt_get_param(NAME bl1 KEY ARCH RET _arch)
stgt_get_param(NAME bl1 KEY HW_ASSISTED_COHERENCY RET _hw_assisted_coherency)

if(_arch STREQUAL aarch64)
	if(_hw_assisted_coherency EQUAL 0)
		stgt_add_src(NAME bl1 bl31 SRC
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a35.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a53.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a57.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a72.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a73.S
		)
	else()
		stgt_add_src(NAME bl1 bl31 SRC
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a55.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a75.S
		)

		stgt_add_src_cond(NAME bl1 bl31 KEY CTX_INCLUDE_AARCH32_REGS VAL 0 SRC
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a76.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a76ae.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a77.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a78.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/neoverse_n1.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/neoverse_e1.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/neoverse_zeus.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_hercules_ae.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_klein.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_matterhorn.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a65.S
			${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cortex_a65ae.S
		)
	endif()
elseif(_arch STREQUAL aarch32)
	stgt_add_src(NAME bl1 bl31 SRC
		${PROJECT_SOURCE_DIR}/lib/cpus/aarch32/cortex_a32.S
	)
endif()

unset(_arch)
unset(_ctx_include_aarch32_regs)

#plat_bl1
stgt_add_src_param(NAME bl1 KEY ARCH SRC
	${PROJECT_SOURCE_DIR}/drivers/arm/smmu/smmu_v3.c
	${PROJECT_SOURCE_DIR}/drivers/arm/sp805/sp805.c
	${PROJECT_SOURCE_DIR}/drivers/delay_timer/delay_timer.c
	${PROJECT_SOURCE_DIR}/drivers/io/io_semihosting.c
	${PROJECT_SOURCE_DIR}/lib/semihosting/semihosting.c
	${PROJECT_SOURCE_DIR}/lib/semihosting/@ARCH@/semihosting_call.S
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/@ARCH@/fvp_helpers.S
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_bl1_setup.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_err.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_io_storage.c

	#TODO: select interconnect
	${PROJECT_SOURCE_DIR}/drivers/arm/cci/cci.c
)

stgt_add_src_cond(NAME bl1 KEY FVP_USE_SP804_TIMER VAL 0 SRC
	${PROJECT_SOURCE_DIR}/drivers/delay_timer/generic_delay_timer.c
)
stgt_add_src_cond(NAME bl1 KEY FVP_USE_SP804_TIMER VAL 1 SRC
	${PROJECT_SOURCE_DIR}/drivers/arm/sp804/sp804_delay_timer.c
)

#plat_bl2
stgt_add_src_param(NAME bl2 KEY ARCH SRC
	${PROJECT_SOURCE_DIR}/drivers/arm/sp805/sp805.c
	${PROJECT_SOURCE_DIR}/drivers/io/io_semihosting.c
	${PROJECT_SOURCE_DIR}/lib/utils/mem_region.c
	${PROJECT_SOURCE_DIR}/lib/semihosting/semihosting.c
	${PROJECT_SOURCE_DIR}/lib/semihosting/@ARCH@/semihosting_call.S
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_bl2_setup.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_err.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_io_storage.c
	${PROJECT_SOURCE_DIR}/plat/arm/common/arm_nor_psci_mem_protect.c

	#FVP security sources
	${PROJECT_SOURCE_DIR}/drivers/arm/tzc/tzc400.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_security.c
	${PROJECT_SOURCE_DIR}/plat/arm/common/arm_tzc400.c

	#TODO: if BL2_AT_EL3=1
)

stgt_add_src_cond(NAME bl2 KEY FVP_USE_SP804_TIMER VAL 1 SRC
	${PROJECT_SOURCE_DIR}/drivers/arm/sp804/sp804_delay_timer.c
)

#plat_bl31
stgt_add_src_param(NAME bl31 KEY ARCH SRC
	${PROJECT_SOURCE_DIR}/drivers/arm/fvp/fvp_pwrc.c
	${PROJECT_SOURCE_DIR}/drivers/arm/smmu/smmu_v3.c
	${PROJECT_SOURCE_DIR}/drivers/delay_timer/delay_timer.c
	${PROJECT_SOURCE_DIR}/lib/utils/mem_region.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_bl31_setup.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_pm.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_topology.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/aarch64/fvp_helpers.S
	${PROJECT_SOURCE_DIR}/plat/arm/common/arm_nor_psci_mem_protect.c

	#GICv3 sources
	${PROJECT_SOURCE_DIR}/drivers/arm/gic/v3/gicv3_main.c
	${PROJECT_SOURCE_DIR}/drivers/arm/gic/v3/gicv3_helpers.c
	${PROJECT_SOURCE_DIR}/drivers/arm/gic/v3/gicdv3_helpers.c
	${PROJECT_SOURCE_DIR}/drivers/arm/gic/v3/gicrv3_helpers.c
	${PROJECT_SOURCE_DIR}/plat/common/plat_gicv3.c
	${PROJECT_SOURCE_DIR}/plat/arm/common/arm_gicv3.c

	#TODO: select GIC model
	${PROJECT_SOURCE_DIR}/drivers/arm/gic/v3/gic-x00.c

	#TODO: select interconnect
	${PROJECT_SOURCE_DIR}/drivers/arm/cci/cci.c

	#FVP security sources
	${PROJECT_SOURCE_DIR}/drivers/arm/tzc/tzc400.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fvp_security.c
	${PROJECT_SOURCE_DIR}/plat/arm/common/arm_tzc400.c

	#fconf support
	${PROJECT_SOURCE_DIR}/common/fdt_wrappers.c
	${PROJECT_SOURCE_DIR}/lib/fconf/fconf.c
	${PROJECT_SOURCE_DIR}/plat/arm/board/fvp/fconf/fconf_hw_config_getter.c

	#TODO: if RAS_EXTENSION=1
)

stgt_add_src_cond(NAME bl31 KEY ENABLE_AMU VAL 1 SRC
	${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cpuamu.c
	${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/cpuamu_helpers.S
	#TODO: HW_ASSISTED_COHERENCY AMU sources
)

stgt_add_src_cond(NAME bl31 KEY FVP_USE_SP804_TIMER VAL 0 SRC
	${PROJECT_SOURCE_DIR}/drivers/delay_timer/generic_delay_timer.c
)
stgt_add_src_cond(NAME bl31 KEY FVP_USE_SP804_TIMER VAL 1 SRC
	${PROJECT_SOURCE_DIR}/drivers/arm/sp804/sp804_delay_timer.c
)

include(${PROJECT_SOURCE_DIR}/plat/arm/board/common/board_common.cmake)
include(${PROJECT_SOURCE_DIR}/plat/arm/common/arm_common.cmake)

#TODO: TRUSTED_BOARD_BOOT
