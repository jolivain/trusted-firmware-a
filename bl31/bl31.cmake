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

stgt_is_defined(NAME bl31 RET _bl31_defined)
if (NOT _bl31_defined)
	message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE}: STGT target 'bl31' is not defined.")
endif()
unset(_bl31_defined)


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
	${PROJECT_SOURCE_DIR}/common/runtime_svc.c
	${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/dsu_helpers.S
	${PROJECT_SOURCE_DIR}/plat/common/aarch64/platform_mp_stack.S
	${PROJECT_SOURCE_DIR}/services/arm_arch_svc/arm_arch_svc_setup.c
	${PROJECT_SOURCE_DIR}/services/std_svc/std_svc_setup.c

	#TODO: SPM sources
	#TODO: SPMD sources
	#TODO: debugfs sources
	#TODO: if SDEI_SUPPORT=1
	#TODO: if ENABLE_SVE_FOR_NS=1
	#TODO: if ENABLE_MPAM_FOR_LOWER_ELS=1
)

stgt_add_src_cond(NAME bl31 KEY EL3_EXCEPTION_HANDLING VAL 1 SRC
	${PROJECT_SOURCE_DIR}/bl31/ehf.c
)

stgt_add_src_cond(NAME bl31 KEY ENABLE_AMU VAL 1 SRC
	${PROJECT_SOURCE_DIR}/lib/extensions/amu/aarch64/amu.c
	${PROJECT_SOURCE_DIR}/lib/extensions/amu/aarch64/amu_helpers.S
)

stgt_add_src_cond(NAME bl31 KEY ENABLE_PMF VAL 1 SRC
	${PROJECT_SOURCE_DIR}/lib/pmf/pmf_main.c
)

stgt_add_src_cond(NAME bl31 KEY ENABLE_SPE_FOR_LOWER_ELS VAL 1 SRC
	${PROJECT_SOURCE_DIR}/lib/extensions/spe/spe.c
)

stgt_add_src_cond(NAME bl31 KEY WORKAROUND_CVE_2017_5715 VAL 1 SRC
	${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/wa_cve_2017_5715_bpiall.S
	${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/wa_cve_2017_5715_mmu.S
)

#TODO: CRASH_REPORTING

# Only compile under aarch64
stgt_get_param(NAME bl31 KEY ARCH RET _arch)
if(_arch STREQUAL aarch64)
	stgt_link_libraries(NAME bl31 LIBS libc xlat_tables_bl31 libfdt)
	stgt_link_build_messages(NAME bl31 LIBS build_message)

	get_target_property(_defs bl31 COMPILE_DEFINITIONS)
	get_target_property(_inc bl31 INCLUDE_DIRECTORIES)

	stgt_set_linker_script(NAME bl31 FILE "${PROJECT_SOURCE_DIR}/bl31/bl31.ld.S" DEF ${_defs} __LINKER__ INC ${_inc})

	compiler_generate_binary_output(bl31)
endif()
unset(_arch)
