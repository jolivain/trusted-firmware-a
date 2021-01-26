#
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

set(CMAKE_SYSTEM_PROCESSOR "cortex-a57.cortex-a53")
set(CMAKE_TOOLCHAIN_PREFIX "aarch64-none-elf-")

set(CMAKE_ASM_COMPILER_TARGET "aarch64-none-elf")
set(CMAKE_C_COMPILER_TARGET "aarch64-none-elf")
set(CMAKE_CXX_COMPILER_TARGET "aarch64-none-elf")

include("${CMAKE_CURRENT_LIST_DIR}/../../../../../cmake/Toolchain/GNU-Baremetal-AArch64.cmake")
