#
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include("${CMAKE_CURRENT_LIST_DIR}/GNU-Baremetal.cmake")

foreach(language IN ITEMS ASM C CXX)
    string(APPEND CMAKE_${language}_FLAGS_INIT "-mgeneral-regs-only ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-mstrict-align ")
endforeach()
