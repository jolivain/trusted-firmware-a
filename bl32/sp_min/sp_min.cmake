#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED PROJECT_SOURCE_DIR)
	message(FATAL_ERROR "PROJECT_SOURCE_DIRnot defined")
endif()

# Include framework files
include(Common/STGT)

stgt_is_defined(NAME bl32 RET _bl32_defined)
if (NOT _bl32_defined)
	message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE}: STGT target 'bl32' is not defined.")
endif()
unset(_bl32_defined)


stgt_get_param(NAME bl32 KEY ARCH RET _arch)
if(NOT _arch STREQUAL aarch32)
	message(FATAL_ERROR "SP_MIN is only supported on AArch32 platforms")
endif()
unset(_arch)

stgt_add_inc(NAME bl32 INC ${PROJECT_SOURCE_DIR}/include/bl32/sp_min)

stgt_add_src(NAME bl32 SRC
	${CMAKE_CURRENT_LIST_DIR}/sp_min_main.c
	${CMAKE_CURRENT_LIST_DIR}/aarch32/entrypoint.S
	${PROJECT_SOURCE_DIR}/common/runtime_svc.c
	${PROJECT_SOURCE_DIR}/plat/common/aarch32/plat_sp_min_common.c
	${PROJECT_SOURCE_DIR}/services/std_svc/std_svc_setup.c
)

stgt_add_src_cond(NAME bl32 KEY ENABLE_PMF VAL 1 SRC
	${PROJECT_SOURCE_DIR}/lib/pmf/pmf_main.c
)

stgt_add_src_cond(NAME bl32 KEY ENABLE_AMU VAL 1 SRC
	${PROJECT_SOURCE_DIR}/lib/extensions/amu/aarch32/amu.c
	${PROJECT_SOURCE_DIR}/lib/extensions/amu/aarch32/amu_helpers.S
)

stgt_add_src_cond(NAME bl32 KEY WORKAROUND_CVE_2017_5715 VAL 1 SRC
	${CMAKE_CURRENT_LIST_DIR}/wa_cve_2017_5715_bpiall.S
	${CMAKE_CURRENT_LIST_DIR}/wa_cve_2017_5715_icache_inv.S
)

include(${HW_PLAT_DIR}/sp_min/sp_min-${TARGET_PLATFORM}.cmake)

stgt_link_libraries(NAME bl32 LIBS xlat_tables_bl32 libc libfdt)
stgt_link_build_messages(NAME bl32 LIBS build_message)

get_target_property(_defs bl32 COMPILE_DEFINITIONS)
get_target_property(_inc bl32 INCLUDE_DIRECTORIES)

stgt_set_linker_script(NAME bl32 FILE "${CMAKE_CURRENT_LIST_DIR}/sp_min.ld.S" DEF ${_defs} __LINKER__ INC ${_inc})

compiler_generate_binary_output(bl32)
