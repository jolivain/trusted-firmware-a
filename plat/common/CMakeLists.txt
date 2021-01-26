#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(platform-stack-setup INTERFACE)

target_sources(platform-stack-setup INTERFACE "${TFA_ARCH}/platform_up_stack.S")

add_library(common-platform INTERFACE)

target_sources(common-platform
    INTERFACE
        "plat_bl_common.c"
        "plat_log_common.c"
        "${TFA_ARCH}/plat_common.c"
        "${TFA_ARCH}/platform_helpers.S")

if(TFA_SANITIZE_UB)
    target_sources(common-platform
        INTERFACE
            "ubsan.c")
endif()

add_library(common-platform-bl1 INTERFACE)

target_sources(common-platform-bl1
    INTERFACE
        "plat_bl1_common.c")

target_link_libraries(common-platform-bl1
    INTERFACE
        common-platform)
