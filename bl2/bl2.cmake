#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

stgt_is_defined(NAME bl2 RET _bl2_defined)
if (NOT _bl2_defined)
	message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE}: STGT target 'bl2' is not defined.")
endif()
unset(_bl2_defined)


stgt_add_src_param(NAME bl2 KEY ARCH SRC
	${CMAKE_CURRENT_LIST_DIR}/bl2_image_load_v2.c
	${CMAKE_CURRENT_LIST_DIR}/bl2_main.c
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl2_arch_setup.c
	${TFA_ROOT_DIR}/lib/locks/exclusive/@ARCH@/spinlock.S
	${TFA_ROOT_DIR}/plat/common/@ARCH@/platform_up_stack.S
	#TODO: mbed TLS sources
)

stgt_get_param(NAME bl2 KEY BL2_AT_EL3 RET _bl2_at_el3)
if(_bl2_at_el3 EQUAL 1)
	stgt_add_src_param(NAME bl2 KEY ARCH SRC
		${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl2_el3_entrypoint.S
		${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl2_el3_exceptions.S
		${TFA_ROOT_DIR}/lib/cpus/@ARCH@/cpu_helpers.S
		${TFA_ROOT_DIR}/lib/cpus/errata_report.c
	)

	stgt_add_src_cond(NAME bl2 KEY ARCH VAL aarch64 SRC
		${TFA_ROOT_DIR}/lib/cpus/aarch64/dsu_helpers.S
	)

	group_new(NAME fvp_image_at_el3)
	group_add(NAME fvp_image_at_el3 TYPE DEFINE KEY IMAGE_AT_EL3 VAL 1)
	group_apply(NAME fvp_image_at_el3 TARGETS bl2)
else()
	stgt_add_src_param(NAME bl2 KEY ARCH SRC
		${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl2_entrypoint.S
	)
endif()

stgt_add_src_cond(NAME bl2 KEY ARCH VAL "aarch64" SRC
	${TFA_ROOT_DIR}/common/aarch64/early_exceptions.S
)

stgt_link_libraries(NAME bl2 LIBS libc xlat_tables_bl2 libfdt)
stgt_link_build_messages(NAME bl2 LIBS build_message)

get_target_property(_defs bl2 COMPILE_DEFINITIONS)
get_target_property(_inc bl2 INCLUDE_DIRECTORIES)

if(_bl2_at_el3 EQUAL 1)
	stgt_set_linker_script(
		NAME bl2
		FILE "${TFA_ROOT_DIR}/bl2/bl2_el3.ld.S"
		DEF ${_defs} __LINKER__
		INC ${_inc}
	)
else()
	stgt_set_linker_script(
		NAME bl2
		FILE "${TFA_ROOT_DIR}/bl2/bl2.ld.S"
		DEF ${_defs} __LINKER__
		INC ${_inc}
	)
endif()
unset(_bl2_at_el3)

compiler_generate_binary_output(bl2)
