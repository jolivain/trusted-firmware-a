#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

stgt_create(NAME bl1)
stgt_add_setting(NAME bl1 GROUPS default compiler hw_plat bl1_specific)
stgt_set_target(NAME bl1 TYPE exe)

stgt_add_src_param(NAME bl1 KEY ARCH SRC
	${CMAKE_CURRENT_LIST_DIR}/bl1_main.c
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl1_arch_setup.c
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl1_context_mgmt.c
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl1_entrypoint.S
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl1_exceptions.S
)

stgt_add_src_param(NAME bl1 KEY ARCH SRC
	${TFA_ROOT_DIR}/lib/cpus/@ARCH@/cpu_helpers.S
	${TFA_ROOT_DIR}/lib/cpus/errata_report.c
	${TFA_ROOT_DIR}/lib/el3_runtime/@ARCH@/context_mgmt.c
	${TFA_ROOT_DIR}/plat/common/plat_bl1_common.c
	${TFA_ROOT_DIR}/plat/common/@ARCH@/platform_up_stack.S
	#TODO: mbed TLS sources
	#TODO: TBB sources
)

stgt_add_src_cond(NAME bl1 KEY ARCH VAL "aarch64" SRC
	${TFA_ROOT_DIR}/lib/cpus/aarch64/dsu_helpers.S
	${TFA_ROOT_DIR}/lib/el3_runtime/aarch64/context.S
)

stgt_link_libraries(NAME bl1 LIBS libc xlat_tables_bl1 libfdt)
stgt_link_build_messages(NAME bl1 LIBS build_message)

get_target_property(_defs bl1 COMPILE_DEFINITIONS)
get_target_property(_inc bl1 INCLUDE_DIRECTORIES)

stgt_set_linker_script(NAME bl1 FILE "${TFA_ROOT_DIR}/bl1/bl1.ld.S" DEF ${_defs} __LINKER__ INC ${_inc})

compiler_generate_binary_output(bl1)
