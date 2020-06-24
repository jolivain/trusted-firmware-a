#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

stgt_create(NAME bl31)
stgt_add_setting(NAME bl31 GROUPS default compiler hw_plat bl31_specific)
stgt_set_target(NAME bl31 TYPE exe)

stgt_add_src(NAME bl31 SRC
	${CMAKE_CURRENT_LIST_DIR}/bl31_main.c
	${CMAKE_CURRENT_LIST_DIR}/interrupt_mgmt.c
	${CMAKE_CURRENT_LIST_DIR}/aarch64/bl31_entrypoint.S
	${CMAKE_CURRENT_LIST_DIR}/aarch64/crash_reporting.S
	${CMAKE_CURRENT_LIST_DIR}/aarch64/ea_delegate.S
	${CMAKE_CURRENT_LIST_DIR}/aarch64/runtime_exceptions.S
	${CMAKE_CURRENT_LIST_DIR}/bl31_context_mgmt.c
)

stgt_add_src(NAME bl31 SRC
	${TFA_ROOT_DIR}/common/runtime_svc.c
	${TFA_ROOT_DIR}/lib/cpus/aarch64/dsu_helpers.S
	${TFA_ROOT_DIR}/plat/common/aarch64/platform_mp_stack.S
	${TFA_ROOT_DIR}/services/arm_arch_svc/arm_arch_svc_setup.c
	${TFA_ROOT_DIR}/services/std_svc/std_svc_setup.c

	#TODO: SPM sources
	#TODO: SPMD sources
	#TODO: debugfs sources
	#TODO: if SDEI_SUPPORT=1
	#TODO: if ENABLE_SVE_FOR_NS=1
	#TODO: if ENABLE_MPAM_FOR_LOWER_ELS=1
)

stgt_add_src_cond(NAME bl31 KEY EL3_EXCEPTION_HANDLING VAL 1 SRC
	${TFA_ROOT_DIR}/bl31/ehf.c
)

stgt_add_src_cond(NAME bl31 KEY ENABLE_AMU VAL 1 SRC
	${TFA_ROOT_DIR}/lib/extensions/amu/aarch64/amu.c
	${TFA_ROOT_DIR}/lib/extensions/amu/aarch64/amu_helpers.S
)

stgt_add_src_cond(NAME bl31 KEY ENABLE_PMF VAL 1 SRC
	${TFA_ROOT_DIR}/lib/pmf/pmf_main.c
)

stgt_add_src_cond(NAME bl31 KEY ENABLE_SPE_FOR_LOWER_ELS VAL 1 SRC
	${TFA_ROOT_DIR}/lib/extensions/spe/spe.c
)

stgt_add_src_cond(NAME bl31 KEY WORKAROUND_CVE_2017_5715 VAL 1 SRC
	${TFA_ROOT_DIR}/lib/cpus/aarch64/wa_cve_2017_5715_bpiall.S
	${TFA_ROOT_DIR}/lib/cpus/aarch64/wa_cve_2017_5715_mmu.S
)

#TODO: CRASH_REPORTING

stgt_link_libraries(NAME bl31 LIBS libc xlat_tables_bl31 libfdt)
stgt_link_build_messages(NAME bl31 LIBS build_message)

get_target_property(_defs bl31 COMPILE_DEFINITIONS)
get_target_property(_inc bl31 INCLUDE_DIRECTORIES)

stgt_set_linker_script(NAME bl31 FILE "${TFA_ROOT_DIR}/bl31/bl31.ld.S" DEF ${_defs} __LINKER__ INC ${_inc})

compiler_generate_binary_output(bl31)
