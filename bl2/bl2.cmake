#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

stgt_create(NAME bl2)
stgt_add_setting(NAME bl2 GROUPS default compiler hw_plat bl2_specific)
stgt_set_target(NAME bl2 TYPE exe)

#TODO: if BL2_AT_EL3=1
stgt_add_src_param(NAME bl2 KEY ARCH SRC
	${CMAKE_CURRENT_LIST_DIR}/bl2_image_load_v2.c
	${CMAKE_CURRENT_LIST_DIR}/bl2_main.c
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl2_arch_setup.c
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/bl2_entrypoint.S
)

stgt_add_src_param(NAME bl2 KEY ARCH SRC
	${TFA_ROOT_DIR}/lib/locks/exclusive/@ARCH@/spinlock.S
	${TFA_ROOT_DIR}/plat/common/@ARCH@/platform_up_stack.S
	#TODO: mbed TLS sources
)

stgt_add_src_cond(NAME bl2 KEY ARCH VAL "aarch64" SRC
	${TFA_ROOT_DIR}/common/aarch64/early_exceptions.S
)

stgt_link_libraries(NAME bl2 LIBS libc xlat_tables_bl2 libfdt)
stgt_link_build_messages(NAME bl2 LIBS build_message)

get_target_property(_defs bl2 COMPILE_DEFINITIONS)
get_target_property(_inc bl2 INCLUDE_DIRECTORIES)

#TODO: if BL2_AT_EL3=1
stgt_set_linker_script(NAME bl2 FILE "${TFA_ROOT_DIR}/bl2/bl2.ld.S" DEF ${_defs} __LINKER__ INC ${_inc})

compiler_generate_binary_output(bl2)
