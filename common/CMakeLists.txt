#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Configure backtracing support.
#

arm_config_option(
    NAME TFA_ENABLE_BACKTRACE
    HELP "Enable backtracing in crash dumps."
    DEPENDS NOT TFA_ARCH_STATE_AARCH32_THUMB)

target_compile_definitions(tfa-common
    INTERFACE "ENABLE_BACKTRACE=$<BOOL:${TFA_ENABLE_BACKTRACE}>")

if(TFA_ENABLE_BACKTRACE)
    add_subdirectory("backtrace" EXCLUDE_FROM_ALL)

    target_link_libraries(tfa-common
        INTERFACE tfa-common-backtrace)
endif()

#
# Set up common bootloader sources. Structurally, this is inherited from the
# legacy build system, so it's not entirely clear why these are isolated
# components.
#

add_library(tfa-common-bl INTERFACE)

target_sources(tfa-common-bl
    INTERFACE "bl_common.c"
              "tf_log.c")

target_link_libraries(tfa-common-bl
    INTERFACE tfa-driver-multi-console)

add_subdirectory("${TFA_ARCH_STATE_LOWER}")

target_link_libraries(tfa-common-bl
    INTERFACE tfa-common-bl-${TFA_ARCH_STATE_LOWER})

#
# Create the common flat device tree (FDT) wrappers.
#

add_library(tfa-common-fdt-wrappers INTERFACE)

target_sources(tfa-common-fdt-wrappers
    INTERFACE "fdt_wrappers.c")

target_link_libraries(tfa-common-fdt-wrappers
    INTERFACE tfa-lib-fdt)
