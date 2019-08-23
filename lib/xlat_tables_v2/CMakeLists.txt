#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.13)

project(xlat_tables LANGUAGES C ASM)

# xlat_tables depends on IMAGE_BLx definition
stgt_create(NAME xlat_tables_bl1 )
stgt_create(NAME xlat_tables_bl2 )
stgt_create(NAME xlat_tables_bl31)

stgt_add_setting(NAME xlat_tables_bl1  GROUPS default compiler hw_plat bl1_specific )
stgt_add_setting(NAME xlat_tables_bl2  GROUPS default compiler hw_plat bl2_specific )
stgt_add_setting(NAME xlat_tables_bl31 GROUPS default compiler hw_plat bl31_specific)

stgt_set_target(NAME xlat_tables_bl1  TYPE lib)
stgt_set_target(NAME xlat_tables_bl2  TYPE lib)
stgt_set_target(NAME xlat_tables_bl31 TYPE lib)

#TODO: RO xlat tables
stgt_add_src_param(NAME xlat_tables_bl1 xlat_tables_bl2 xlat_tables_bl31 KEY ARCH SRC
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/enable_mmu.S
	${CMAKE_CURRENT_LIST_DIR}/@ARCH@/xlat_tables_arch.c
	${CMAKE_CURRENT_LIST_DIR}/xlat_tables_context.c
	${CMAKE_CURRENT_LIST_DIR}/xlat_tables_core.c
	${CMAKE_CURRENT_LIST_DIR}/xlat_tables_utils.c
)

stgt_link_libraries(NAME xlat_tables_bl1  LIBS libc)
stgt_link_libraries(NAME xlat_tables_bl2  LIBS libc)
stgt_link_libraries(NAME xlat_tables_bl31 LIBS libc)
