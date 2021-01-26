#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Include framework files
include(Common/STGT)

# bl_common
stgt_add_src(
    NAME
    bl1
    bl2
    bl31
    bl32
    SRC
    ${PROJECT_SOURCE_DIR}/common/bl_common.c
    ${PROJECT_SOURCE_DIR}/common/tf_log.c
    ${PROJECT_SOURCE_DIR}/common/${TFA_ARCH}/debug.S
    ${PROJECT_SOURCE_DIR}/drivers/console/multi_console.c
    ${PROJECT_SOURCE_DIR}/lib/${TFA_ARCH}/cache_helpers.S
    ${PROJECT_SOURCE_DIR}/lib/${TFA_ARCH}/misc_helpers.S
    ${PROJECT_SOURCE_DIR}/plat/common/plat_bl_common.c
    ${PROJECT_SOURCE_DIR}/plat/common/plat_log_common.c
    ${PROJECT_SOURCE_DIR}/plat/common/${TFA_ARCH}/plat_common.c
    ${PROJECT_SOURCE_DIR}/plat/common/${TFA_ARCH}/platform_helpers.S
    ${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/popcountdi2.c
    ${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/popcountsi2.c
    # TODO: armclang, ubsan, stack protector
)

if(TFA_AARCH32)
    stgt_add_src(
        NAME
        bl1
        bl2
        bl31
        bl32
        SRC
        ${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/arm/aeabi_ldivmod.S
        ${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/arm/aeabi_uldivmod.S
        ${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/ctzdi2.c
        ${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/divdi3.c
        ${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/divmoddi4.c
        ${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/lshrdi3.c
        ${PROJECT_SOURCE_DIR}/lib/compiler-rt/builtins/udivmoddi4.c)
endif()
