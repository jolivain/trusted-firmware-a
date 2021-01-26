#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(CMAKE_TOOLCHAIN_PREFIX "arm-none-eabi-")

set(CMAKE_ASM_COMPILER_TARGET "arm-none-eabi")
set(CMAKE_C_COMPILER_TARGET "arm-none-eabi")
set(CMAKE_CXX_COMPILER_TARGET "arm-none-eabi")

include("${CMAKE_CURRENT_LIST_DIR}/GNU-Baremetal.cmake")

foreach(language IN ITEMS ASM C CXX)
    string(APPEND CMAKE_${language}_FLAGS_INIT "-mno-unaligned-access ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-mthumb ")
endforeach()
