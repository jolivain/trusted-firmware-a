#
# Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(aem-generic INTERFACE)

target_sources(aem-generic
    INTERFACE
        "${TFA_ARCH}/aem_generic.S")

add_subdirectory("${TFA_ARCH}" EXCLUDE_FROM_ALL)

add_library(cpu-helpers INTERFACE)

target_include_directories(cpu-helpers
    INTERFACE
        "${PROJECT_SOURCE_DIR}/include/lib/cpus/${TFA_ARCH}")

target_sources(cpu-helpers
    INTERFACE
        "${TFA_ARCH}/cpu_helpers.S")

add_library(errata-reporting INTERFACE)

target_sources(errata-reporting
    INTERFACE
        "errata_report.c")
