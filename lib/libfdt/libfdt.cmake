#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

stgt_create(NAME libfdt)
stgt_add_setting(NAME libfdt GROUPS default compiler hw_plat)
stgt_set_target(NAME libfdt TYPE lib)

stgt_add_src(NAME libfdt SRC
	${CMAKE_CURRENT_LIST_DIR}/fdt.c
	${CMAKE_CURRENT_LIST_DIR}/fdt_addresses.c
	${CMAKE_CURRENT_LIST_DIR}/fdt_empty_tree.c
	${CMAKE_CURRENT_LIST_DIR}/fdt_ro.c
	${CMAKE_CURRENT_LIST_DIR}/fdt_rw.c
	${CMAKE_CURRENT_LIST_DIR}/fdt_strerror.c
	${CMAKE_CURRENT_LIST_DIR}/fdt_sw.c
	${CMAKE_CURRENT_LIST_DIR}/fdt_wip.c
)

stgt_link_libraries(NAME libfdt LIBS libc)
