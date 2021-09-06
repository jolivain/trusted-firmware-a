#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-common-platform-stack-setup INTERFACE)

target_sources(tfa-common-platform-stack-setup
    INTERFACE "${TFA_ARCH_STATE_LOWER}/platform_up_stack.S")

add_library(tfa-common-platform INTERFACE)

target_sources(tfa-common-platform
    INTERFACE "plat_bl_common.c"
              "plat_log_common.c"
              "${TFA_ARCH_STATE_LOWER}/plat_common.c"
              "${TFA_ARCH_STATE_LOWER}/platform_helpers.S")

target_link_libraries(tfa-common-platform
    INTERFACE tfa-common-bl)

if(TFA_SANITIZE_UB)
    target_sources(tfa-common-platform
        INTERFACE "ubsan.c")
endif()

add_library(tfa-common-platform-bl1 INTERFACE)

target_sources(tfa-common-platform-bl1
    INTERFACE "plat_bl1_common.c")

target_link_libraries(tfa-common-platform-bl1
    INTERFACE tfa-common-platform)
