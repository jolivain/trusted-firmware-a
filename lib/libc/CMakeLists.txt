#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.13)

project(libc LANGUAGES C ASM)

stgt_create(NAME libc)
stgt_add_setting(NAME libc GROUPS default compiler hw_plat)
stgt_set_target(NAME libc TYPE lib)

stgt_add_src(NAME libc SRC
	${CMAKE_CURRENT_LIST_DIR}/abort.c
	${CMAKE_CURRENT_LIST_DIR}/assert.c
	${CMAKE_CURRENT_LIST_DIR}/exit.c
	${CMAKE_CURRENT_LIST_DIR}/memchr.c
	${CMAKE_CURRENT_LIST_DIR}/memcmp.c
	${CMAKE_CURRENT_LIST_DIR}/memcpy.c
	${CMAKE_CURRENT_LIST_DIR}/memmove.c
	${CMAKE_CURRENT_LIST_DIR}/memrchr.c
	${CMAKE_CURRENT_LIST_DIR}/memset.c
	${CMAKE_CURRENT_LIST_DIR}/printf.c
	${CMAKE_CURRENT_LIST_DIR}/putchar.c
	${CMAKE_CURRENT_LIST_DIR}/puts.c
	${CMAKE_CURRENT_LIST_DIR}/snprintf.c
	${CMAKE_CURRENT_LIST_DIR}/strchr.c
	${CMAKE_CURRENT_LIST_DIR}/strcmp.c
	${CMAKE_CURRENT_LIST_DIR}/strlcpy.c
	${CMAKE_CURRENT_LIST_DIR}/strlen.c
	${CMAKE_CURRENT_LIST_DIR}/strncmp.c
	${CMAKE_CURRENT_LIST_DIR}/strnlen.c
	${CMAKE_CURRENT_LIST_DIR}/strrchr.c
)

stgt_add_src_cond(NAME libc KEY ARCH VAL "aarch64" SRC ${CMAKE_CURRENT_LIST_DIR}/aarch64/setjmp.S)
