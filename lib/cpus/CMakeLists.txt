#
# Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(lib-cpu-common INTERFACE)

target_compile_definitions(lib-cpu-common
    INTERFACE
        "ENABLE_FEAT_RNG=__ARM_FEATURE_RNG")

add_subdirectory("${TFA_ARCH}" EXCLUDE_FROM_ALL)

add_library(lib-cpu-aem-generic INTERFACE)

target_sources(lib-cpu-aem-generic
    INTERFACE
        "${TFA_ARCH}/aem_generic.S")

add_library(lib-cpu-helpers INTERFACE)

target_include_directories(lib-cpu-helpers
    INTERFACE
        "${PROJECT_SOURCE_DIR}/include/lib/cpus/${TFA_ARCH}")

target_sources(lib-cpu-helpers
    INTERFACE
        "${TFA_ARCH}/cpu_helpers.S")

add_library(lib-cpu-errata-reporting INTERFACE)

target_sources(lib-cpu-errata-reporting
    INTERFACE
        "errata_report.c")

if(TFA_AARCH64)
    add_library(lib-cpu-dsu-helpers INTERFACE)

    target_sources(lib-cpu-dsu-helpers
        INTERFACE
            "aarch64/dsu_helpers.S")
endif()
