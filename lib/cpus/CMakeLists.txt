#
# Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-lib-cpu-common INTERFACE)

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ENABLE_FEAT_RNG=__ARM_FEATURE_RNG")

add_subdirectory("${TFA_ARCH_STATE_LOWER}" EXCLUDE_FROM_ALL)

add_library(tfa-lib-cpu-aem-generic INTERFACE)

target_sources(tfa-lib-cpu-aem-generic
    INTERFACE "${TFA_ARCH_STATE_LOWER}/aem_generic.S")

add_library(tfa-lib-cpu-helpers INTERFACE)

target_include_directories(tfa-lib-cpu-helpers
    INTERFACE "${TFA_SOURCE_DIR}/include/lib/cpus/${TFA_ARCH_STATE_LOWER}")

target_sources(tfa-lib-cpu-helpers
    INTERFACE "${TFA_ARCH_STATE_LOWER}/cpu_helpers.S")

add_library(tfa-lib-cpu-errata-reporting INTERFACE)

target_sources(tfa-lib-cpu-errata-reporting
    INTERFACE "errata_report.c")

if(TFA_ARCH_STATE_AARCH64)
    add_library(tfa-lib-cpu-dsu-helpers INTERFACE)

    target_sources(tfa-lib-cpu-dsu-helpers
        INTERFACE "aarch64/dsu_helpers.S")
endif()
