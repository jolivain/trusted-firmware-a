#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

set(CMAKE_TOOLCHAIN_PREFIX "aarch64-none-elf-")

set(CMAKE_ASM_COMPILER_TARGET "aarch64-none-elf")
set(CMAKE_C_COMPILER_TARGET "aarch64-none-elf")
set(CMAKE_CXX_COMPILER_TARGET "aarch64-none-elf")

include("${CMAKE_CURRENT_LIST_DIR}/GNU-Baremetal.cmake")

foreach(language IN ITEMS ASM C CXX)
    string(APPEND CMAKE_${language}_FLAGS_INIT "-mgeneral-regs-only ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-mstrict-align ")
endforeach()
