#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

foreach(language IN ITEMS ASM C CXX)
    string(APPEND CMAKE_${language}_FLAGS " -ffreestanding")
    string(APPEND CMAKE_${language}_FLAGS " -fno-builtin")
    string(APPEND CMAKE_${language}_FLAGS " -nostdinc")
    string(APPEND CMAKE_${language}_FLAGS " -nostdlib")

    string(APPEND CMAKE_${language}_FLAGS " -Wall")
    string(APPEND CMAKE_${language}_FLAGS " -Wdisabled-optimization")
    string(APPEND CMAKE_${language}_FLAGS " -Wmaybe-uninitialized")
    string(APPEND CMAKE_${language}_FLAGS " -Wmissing-include-dirs")
    string(APPEND CMAKE_${language}_FLAGS " -Wno-error=cpp")
    string(APPEND CMAKE_${language}_FLAGS " -Wno-error=deprecated-declarations")
    string(APPEND CMAKE_${language}_FLAGS " -Wno-unused-parameter")
    string(APPEND CMAKE_${language}_FLAGS " -Wpacked-bitfield-compat")
    string(APPEND CMAKE_${language}_FLAGS " -Wshift-overflow=2")
    string(APPEND CMAKE_${language}_FLAGS " -Wunused-but-set-variable")
    string(APPEND CMAKE_${language}_FLAGS " -Wunused")
    string(APPEND CMAKE_${language}_FLAGS " -Wvla")
    string(APPEND CMAKE_${language}_FLAGS " -Werror")

    # FIXME: should be applied to backtrace library public compile flags
    string(APPEND CMAKE_${language}_FLAGS " -fno-omit-frame-pointer")

endforeach()

string(APPEND CMAKE_ASM_FLAGS " -Wa,--fatal-warnings")
string(APPEND CMAKE_ASM_FLAGS " -Wa,--gdwarf-2") # TODO: do we need this?

# Include framework files
include(Common/Group)
include(Compiler/GCC)

group_new(NAME compiler)
