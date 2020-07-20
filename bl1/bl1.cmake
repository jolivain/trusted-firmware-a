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

stgt_is_defined(NAME bl1 RET _bl1_defined)
if (NOT _bl1_defined)
	message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE}: STGT target 'bl1' is not defined.")
endif()
unset(_bl1_defined)


stgt_add_src_param(NAME bl1 KEY ARCH SRC
	${CMAKE_CURRENT_LIST_DIR}/bl1_main.c
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl1_arch_setup.c
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl1_context_mgmt.c
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl1_entrypoint.S
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl1_exceptions.S
)

stgt_add_src_param(NAME bl1 KEY ARCH SRC
	${PROJECT_SOURCE_DIR}/lib/cpus/@ARCH@/cpu_helpers.S
	${PROJECT_SOURCE_DIR}/lib/cpus/errata_report.c
	${PROJECT_SOURCE_DIR}/lib/el3_runtime/@ARCH@/context_mgmt.c
	${PROJECT_SOURCE_DIR}/plat/common/plat_bl1_common.c
	${PROJECT_SOURCE_DIR}/plat/common/@ARCH@/platform_up_stack.S
	#TODO: mbed TLS sources
	#TODO: TBB sources
)

stgt_add_src_cond(NAME bl1 KEY ARCH VAL "aarch64" SRC
	${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/dsu_helpers.S
	${PROJECT_SOURCE_DIR}/lib/el3_runtime/aarch64/context.S
)

stgt_link_libraries(NAME bl1 LIBS libc xlat_tables_bl1 libfdt)
stgt_link_build_messages(NAME bl1 LIBS build_message)

get_target_property(_defs bl1 COMPILE_DEFINITIONS)
get_target_property(_inc bl1 INCLUDE_DIRECTORIES)

stgt_set_linker_script(NAME bl1 FILE "${PROJECT_SOURCE_DIR}/bl1/bl1.ld.S" DEF ${_defs} __LINKER__ INC ${_inc})

compiler_generate_binary_output(bl1)
