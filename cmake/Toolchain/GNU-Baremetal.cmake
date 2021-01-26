#
# Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include("${CMAKE_CURRENT_LIST_DIR}/Generic-Baremetal.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/GNU-Base.cmake")

foreach(language IN ITEMS ASM C CXX)
    if(CMAKE_SYSTEM_PROCESSOR)
        string(APPEND CMAKE_${language}_FLAGS_INIT
            "-mcpu=${CMAKE_SYSTEM_PROCESSOR} ")
    endif()

    if(CMAKE_SYSTEM_ARCH)
        string(APPEND CMAKE_${language}_FLAGS_INIT
            "-march=${CMAKE_SYSTEM_ARCH} ")
    endif()

    string(APPEND CMAKE_${language}_FLAGS_INIT "-ffunction-sections ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-fdata-sections ")

    string(APPEND CMAKE_${language}_FLAGS_DEBUG_INIT "-Og")
endforeach()

string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections ")
