#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

# Select interconnect driver based on cluster count
stgt_get_param(NAME bl1 KEY FVP_CLUSTER_COUNT RET _fvp_cluster_count)
group_new(NAME fvp_interconnect_driver)
if (_fvp_cluster_count GREATER 2)
	group_add(NAME fvp_interconnect_driver TYPE CONFIG DEFINE KEY FVP_INTERCONNECT_DRIVER VAL FVP_CCN)
else()
	group_add(NAME fvp_interconnect_driver TYPE CONFIG DEFINE KEY FVP_INTERCONNECT_DRIVER VAL FVP_CCI)
endif()
group_apply(NAME fvp_interconnect_driver TARGETS bl1 bl2 bl31)
unset(_fvp_cluster_count)


#plat_bl_common
stgt_add_src(NAME bl1 bl2 bl31 SRC
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_common.c
)

stgt_add_src_cond(NAME bl1 bl2 bl31 KEY ENABLE_STACK_PROTECTOR VAL 1 SRC
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_stack_protector.c
)

#FVP CPU libs
stgt_add_src_param(NAME bl1 bl31 KEY ARCH SRC
	${TFA_ROOT_DIR}/lib/cpus/@ARCH@/aem_generic.S
)

stgt_get_param(NAME bl1 KEY ARCH RET _arch)
stgt_get_param(NAME bl1 KEY HW_ASSISTED_COHERENCY RET _hw_assisted_coherency)

if(_arch STREQUAL aarch64)
	if(_hw_assisted_coherency EQUAL 0)
		stgt_add_src(NAME bl1 bl31 SRC
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a35.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a53.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a57.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a72.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a73.S
		)
	else()
		stgt_add_src(NAME bl1 bl31 SRC
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a55.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a75.S
		)

		stgt_add_src_cond(NAME bl1 bl31 KEY CTX_INCLUDE_AARCH32_REGS VAL 0 SRC
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a76.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a76ae.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a77.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a78.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/neoverse_n1.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/neoverse_e1.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/neoverse_zeus.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_hercules_ae.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_klein.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_matterhorn.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a65.S
			${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a65ae.S
		)
	endif()
elseif(_arch STREQUAL aarch32)
	stgt_add_src(NAME bl1 bl31 SRC
		${TFA_ROOT_DIR}/lib/cpus/aarch32/cortex_a32.S
	)
endif()

unset(_arch)
unset(_ctx_include_aarch32_regs)

#add appropriate sources for the interconnect driver
stgt_add_src_cond(NAME bl1 bl31 KEY FVP_INTERCONNECT_DRIVER VAL FVP_CCI SRC
	${TFA_ROOT_DIR}/drivers/arm/cci/cci.c
)
stgt_add_src_cond(NAME bl1 bl31 KEY FVP_INTERCONNECT_DRIVER VAL FVP_CCN SRC
	${TFA_ROOT_DIR}/drivers/arm/ccn/ccn.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_ccn.c
)

#plat_bl1
stgt_add_src_param(NAME bl1 KEY ARCH SRC
	${TFA_ROOT_DIR}/drivers/arm/smmu/smmu_v3.c
	${TFA_ROOT_DIR}/drivers/arm/sp805/sp805.c
	${TFA_ROOT_DIR}/drivers/delay_timer/delay_timer.c
	${TFA_ROOT_DIR}/drivers/io/io_semihosting.c
	${TFA_ROOT_DIR}/lib/semihosting/semihosting.c
	${TFA_ROOT_DIR}/lib/semihosting/@ARCH@/semihosting_call.S
	${TFA_ROOT_DIR}/plat/arm/board/fvp/@ARCH@/fvp_helpers.S
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_bl1_setup.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_err.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_io_storage.c
)

stgt_add_src_cond(NAME bl1 KEY FVP_USE_SP804_TIMER VAL 0 SRC
	${TFA_ROOT_DIR}/drivers/delay_timer/generic_delay_timer.c
)
stgt_add_src_cond(NAME bl1 KEY FVP_USE_SP804_TIMER VAL 1 SRC
	${TFA_ROOT_DIR}/drivers/arm/sp804/sp804_delay_timer.c
)

#plat_bl2
stgt_add_src_param(NAME bl2 KEY ARCH SRC
	${TFA_ROOT_DIR}/drivers/arm/sp805/sp805.c
	${TFA_ROOT_DIR}/drivers/io/io_semihosting.c
	${TFA_ROOT_DIR}/lib/utils/mem_region.c
	${TFA_ROOT_DIR}/lib/semihosting/semihosting.c
	${TFA_ROOT_DIR}/lib/semihosting/@ARCH@/semihosting_call.S
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_bl2_setup.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_err.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_io_storage.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_nor_psci_mem_protect.c

	#FVP security sources
	${TFA_ROOT_DIR}/drivers/arm/tzc/tzc400.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_security.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_tzc400.c

	#TODO: if BL2_AT_EL3=1
)

stgt_add_src_cond(NAME bl2 KEY FVP_USE_SP804_TIMER VAL 1 SRC
	${TFA_ROOT_DIR}/drivers/arm/sp804/sp804_delay_timer.c
)

#plat_bl31
stgt_add_src_param(NAME bl31 KEY ARCH SRC
	${TFA_ROOT_DIR}/drivers/arm/fvp/fvp_pwrc.c
	${TFA_ROOT_DIR}/drivers/arm/smmu/smmu_v3.c
	${TFA_ROOT_DIR}/drivers/delay_timer/delay_timer.c
	${TFA_ROOT_DIR}/lib/utils/mem_region.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_bl31_setup.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_pm.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_topology.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/aarch64/fvp_helpers.S
	${TFA_ROOT_DIR}/plat/arm/common/arm_nor_psci_mem_protect.c

	#FVP security sources
	${TFA_ROOT_DIR}/drivers/arm/tzc/tzc400.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fvp_security.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_tzc400.c

	#fconf support
	${TFA_ROOT_DIR}/common/fdt_wrappers.c
	${TFA_ROOT_DIR}/lib/fconf/fconf.c
	${TFA_ROOT_DIR}/plat/arm/board/fvp/fconf/fconf_hw_config_getter.c

	#TODO: if RAS_EXTENSION=1
)

#add appropriate sources for GIC driver
stgt_add_src_cond(NAME bl31 KEY FVP_USE_GIC_DRIVER VAL FVP_GICV3 SRC
	${TFA_ROOT_DIR}/drivers/arm/gic/v3/gicv3_main.c
	${TFA_ROOT_DIR}/drivers/arm/gic/v3/gicv3_helpers.c
	${TFA_ROOT_DIR}/drivers/arm/gic/v3/gicdv3_helpers.c
	${TFA_ROOT_DIR}/drivers/arm/gic/v3/gicrv3_helpers.c
	${TFA_ROOT_DIR}/plat/common/plat_gicv3.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_gicv3.c
	${TFA_ROOT_DIR}/drivers/arm/gic/v3/gic-x00.c
)
stgt_add_src_cond(NAME bl31 KEY FVP_USE_GIC_DRIVER VAL FVP_GICV2 SRC
	${TFA_ROOT_DIR}/drivers/arm/gic/common/gic_common.c
	${TFA_ROOT_DIR}/drivers/arm/gic/v2/gicv2_main.c
	${TFA_ROOT_DIR}/drivers/arm/gic/v2/gicv2_helpers.c
	${TFA_ROOT_DIR}/plat/common/plat_gicv2.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_gicv2.c
)

#conditionally add AMU sources
stgt_add_src_cond(NAME bl31 KEY ENABLE_AMU VAL 1 SRC
	${TFA_ROOT_DIR}/lib/cpus/aarch64/cpuamu.c
	${TFA_ROOT_DIR}/lib/cpus/aarch64/cpuamu_helpers.S
)

#additional sources when ENABLE_AMU = HW_ASSISTED_COHERENCY = 1
stgt_add_src_cond(
	NAME bl31
	KEY ENABLE_AMU HW_ASSISTED_COHERENCY
	VAL 1          1
	SRC
	${TFA_ROOT_DIR}/lib/cpus/aarch64/cortex_a75_pubsub.c
	${TFA_ROOT_DIR}/lib/cpus/aarch64/neoverse_n1_pubsub.c
)

stgt_add_src_cond(NAME bl31 KEY FVP_USE_SP804_TIMER VAL 0 SRC
	${TFA_ROOT_DIR}/drivers/delay_timer/generic_delay_timer.c
)
stgt_add_src_cond(NAME bl31 KEY FVP_USE_SP804_TIMER VAL 1 SRC
	${TFA_ROOT_DIR}/drivers/arm/sp804/sp804_delay_timer.c
)

include(${TFA_ROOT_DIR}/plat/arm/board/common/board_common.cmake)
include(${TFA_ROOT_DIR}/plat/arm/common/arm_common.cmake)

#TODO: TRUSTED_BOARD_BOOT
