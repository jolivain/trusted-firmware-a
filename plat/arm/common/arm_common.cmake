#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

#plat_bl_common
stgt_add_src_param(NAME bl1 bl2 bl31 KEY ARCH SRC
	${TFA_ROOT_DIR}/plat/arm/common/@ARCH@/arm_helpers.S
	${TFA_ROOT_DIR}/plat/arm/common/arm_common.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_console.c

	#TODO: xlat_tables_v1
	#TODO: pauth
)

#plat_bl1
stgt_add_src(NAME bl1 SRC
	${TFA_ROOT_DIR}/drivers/io/io_fip.c
	${TFA_ROOT_DIR}/drivers/io/io_memmap.c
	${TFA_ROOT_DIR}/drivers/io/io_storage.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_bl1_setup.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_err.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_io_storage.c
	${TFA_ROOT_DIR}/plat/arm/common/fconf/arm_fconf_io.c
	${TFA_ROOT_DIR}/lib/fconf/fconf.c

	#Dynamic config sources
	${TFA_ROOT_DIR}/plat/arm/common/arm_dyn_cfg.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_dyn_cfg_helpers.c
	${TFA_ROOT_DIR}/common/fdt_wrappers.c

	#TODO: if EL3_PAYLOAD_BASE=1
	#TODO: TBBR
)

#plat_bl2
stgt_add_src_param(NAME bl2 KEY ARCH SRC
	${TFA_ROOT_DIR}/drivers/delay_timer/delay_timer.c #TODO: this should be in platform.cmake?
	${TFA_ROOT_DIR}/drivers/delay_timer/generic_delay_timer.c
	${TFA_ROOT_DIR}/drivers/io/io_fip.c
	${TFA_ROOT_DIR}/drivers/io/io_memmap.c
	${TFA_ROOT_DIR}/drivers/io/io_storage.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_bl2_setup.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_err.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_io_storage.c
	${TFA_ROOT_DIR}/plat/arm/common/fconf/arm_fconf_io.c
	${TFA_ROOT_DIR}/lib/fconf/fconf.c
	${TFA_ROOT_DIR}/lib/fconf/fconf_dyn_cfg_getter.c

	#Dynamic config sources
	${TFA_ROOT_DIR}/plat/arm/common/arm_dyn_cfg.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_dyn_cfg_helpers.c
	${TFA_ROOT_DIR}/common/fdt_wrappers.c

	${TFA_ROOT_DIR}/plat/arm/common/@ARCH@/arm_bl2_mem_params_desc.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_image_load.c
	${TFA_ROOT_DIR}/common/desc_image_load.c

	#TODO: if BL2_AT_EL3=1
	#TODO: if JUNO_AARCH32_EL3_RUNTIME=1
	#TODO: SPD
	#TODO: TBBR
)

stgt_add_src_cond(NAME bl2 KEY BL2_AT_EL3 VAL 1 SRC
	${TFA_ROOT_DIR}/plat/arm/common/arm_bl2_el3_setup.c
)

#plat_bl31
stgt_add_src(NAME bl31 SRC
	${TFA_ROOT_DIR}/plat/arm/common/arm_bl31_setup.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_pm.c
	${TFA_ROOT_DIR}/plat/arm/common/arm_topology.c
	${TFA_ROOT_DIR}/plat/common/plat_psci_common.c

	#TODO: if SDEI_SUPPORT=1
	#TODO: if ENABLE_SPM=1
)

stgt_add_src_cond(NAME bl31 KEY RAS_EXTENSION VAL 1 SRC
	${TFA_ROOT_DIR}/lib/extensions/ras/std_err_record.c
	${TFA_ROOT_DIR}/lib/extensions/ras/ras_common.c
)

stgt_add_src_cond(NAME bl31 KEY EL3_EXCEPTION_HANDLING VAL 1 SRC
	${TFA_ROOT_DIR}/plat/arm/common/aarch64/arm_ehf.c
)

stgt_add_src_cond(NAME bl31 KEY ENABLE_PMF VAL 1 SRC
	${TFA_ROOT_DIR}/plat/arm/common/aarch64/execution_state_switch.c #TODO: compare to .mk
	${TFA_ROOT_DIR}/plat/arm/common/arm_sip_svc.c
	${TFA_ROOT_DIR}/lib/pmf/pmf_smc.c
)

stgt_add_src(NAME bl1 bl2 SRC
	${TFA_ROOT_DIR}/lib/fconf/fconf.c
	${TFA_ROOT_DIR}/lib/fconf/fconf_dyn_cfg_getter.c
)
