#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Include framework files
include(Common/STGT)

stgt_is_defined(NAME bl2 RET _bl2_defined)
if(NOT _bl2_defined)
    message(
        FATAL_ERROR
            "${CMAKE_CURRENT_LIST_FILE}: STGT target 'bl2' is not defined.")
endif()
unset(_bl2_defined)

stgt_add_src(
    NAME
    bl2
    SRC
    ${CMAKE_CURRENT_LIST_DIR}/bl2_image_load_v2.c
    ${CMAKE_CURRENT_LIST_DIR}/bl2_main.c
    ${CMAKE_CURRENT_LIST_DIR}/${TFA_ARCH}/bl2_arch_setup.c
    ${PROJECT_SOURCE_DIR}/lib/locks/exclusive/${TFA_ARCH}/spinlock.S
    ${PROJECT_SOURCE_DIR}/plat/common/${TFA_ARCH}/platform_up_stack.S
    # TODO: mbed TLS sources
)

stgt_get_param(NAME bl2 KEY BL2_AT_EL3 RET _bl2_at_el3)
if(_bl2_at_el3 EQUAL 1)
    stgt_add_src(
        NAME
        bl2
        SRC
        ${CMAKE_CURRENT_LIST_DIR}/${TFA_ARCH}/bl2_el3_entrypoint.S
        ${CMAKE_CURRENT_LIST_DIR}/${TFA_ARCH}/bl2_el3_exceptions.S
        ${PROJECT_SOURCE_DIR}/lib/cpus/${TFA_ARCH}/cpu_helpers.S
        ${PROJECT_SOURCE_DIR}/lib/cpus/errata_report.c)

    if(TFA_AARCH64)
        stgt_add_src(NAME bl2 SRC
                     ${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/dsu_helpers.S)
    endif()

    group_new(NAME fvp_image_at_el3)
    group_add(
        NAME
        fvp_image_at_el3
        TYPE
        DEFINE
        KEY
        IMAGE_AT_EL3
        VAL
        1)
    group_apply(NAME fvp_image_at_el3 TARGETS bl2)
else()
    stgt_add_src(NAME bl2 SRC
                 ${CMAKE_CURRENT_LIST_DIR}/${TFA_ARCH}/bl2_entrypoint.S)
endif()

if(TFA_AARCH64)
    stgt_add_src(NAME bl2 SRC
                 ${PROJECT_SOURCE_DIR}/common/aarch64/early_exceptions.S)
endif()

stgt_link_libraries(NAME bl2 LIBS fdt)
stgt_link_build_messages(NAME bl2 LIBS build_message)

target_link_libraries(bl2 PRIVATE common)

include(TFATargetLinkerScript)

if(_bl2_at_el3 EQUAL 1)
    tfa_target_linker_script(bl2 "${PROJECT_SOURCE_DIR}/bl2/bl2_el3.ld.S")
else()
    tfa_target_linker_script(bl2 "${PROJECT_SOURCE_DIR}/bl2/bl2.ld.S")
endif()
unset(_bl2_at_el3)

add_custom_command(
    TARGET bl2 POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:bl2>
            $<TARGET_FILE_DIR:bl2>/bl2.bin)
