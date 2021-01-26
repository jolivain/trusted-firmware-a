#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Include framework files
include(Common/STGT)

stgt_is_defined(NAME bl1 RET _bl1_defined)
if(NOT _bl1_defined)
    message(
        FATAL_ERROR
            "${CMAKE_CURRENT_LIST_FILE}: STGT target 'bl1' is not defined.")
endif()
unset(_bl1_defined)

stgt_add_src(
    NAME
    bl1
    SRC
    ${CMAKE_CURRENT_LIST_DIR}/bl1_main.c
    ${CMAKE_CURRENT_LIST_DIR}/${TFA_ARCH}/bl1_arch_setup.c
    ${CMAKE_CURRENT_LIST_DIR}/${TFA_ARCH}/bl1_context_mgmt.c
    ${CMAKE_CURRENT_LIST_DIR}/${TFA_ARCH}/bl1_entrypoint.S
    ${CMAKE_CURRENT_LIST_DIR}/${TFA_ARCH}/bl1_exceptions.S)

stgt_add_src(
    NAME
    bl1
    SRC
    ${PROJECT_SOURCE_DIR}/lib/cpus/${TFA_ARCH}/cpu_helpers.S
    ${PROJECT_SOURCE_DIR}/lib/cpus/errata_report.c
    ${PROJECT_SOURCE_DIR}/lib/el3_runtime/${TFA_ARCH}/context_mgmt.c
    ${PROJECT_SOURCE_DIR}/plat/common/plat_bl1_common.c
    ${PROJECT_SOURCE_DIR}/plat/common/${TFA_ARCH}/platform_up_stack.S
    # TODO: mbed TLS sources TODO: TBB sources
)

if(TFA_AARCH64)
    stgt_add_src(
        NAME bl1 SRC ${PROJECT_SOURCE_DIR}/lib/cpus/aarch64/dsu_helpers.S
        ${PROJECT_SOURCE_DIR}/lib/el3_runtime/aarch64/context.S)
endif()

stgt_link_libraries(NAME bl1 LIBS fdt)
stgt_link_build_messages(NAME bl1 LIBS build_message)

target_link_libraries(bl1 PRIVATE common)

if(TARGET platform-bl1)
    target_link_libraries(bl1 PRIVATE platform-bl1)
endif()

get_target_property(_defs bl1 COMPILE_DEFINITIONS)
get_target_property(_inc bl1 INCLUDE_DIRECTORIES)

include(TFATargetLinkerScript)

tfa_target_linker_script(bl1 "${PROJECT_SOURCE_DIR}/bl1/bl1.ld.S")

add_custom_command(
    TARGET bl1 POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:bl1>
            $<TARGET_FILE_DIR:bl1>/bl1.bin)
