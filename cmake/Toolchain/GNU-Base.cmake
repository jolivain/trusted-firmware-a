#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

set(CMAKE_ASM_COMPILER
    "${CMAKE_TOOLCHAIN_PREFIX}gcc"
    CACHE FILEPATH "Path to the assembler.")

set(CMAKE_C_COMPILER
    "${CMAKE_TOOLCHAIN_PREFIX}gcc"
    CACHE FILEPATH "Path to the C compiler.")
