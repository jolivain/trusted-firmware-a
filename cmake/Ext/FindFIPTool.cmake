#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
FindFIPTool
===========

TF-A target platforms which use |FIP| images to bootstrap the system depend on
FIPTool, a small utility written in C capable of manipulating |FIP| packages.
This tool lives in the TF-A repository, and needs to be built in order to create
the |FIP| image required to boot the target system.

This module creates an executable target representing the FIPTool binary. This
binary is compiled at build time, so do note that it will not exist during the
configure or generate stages. This is typically not a problem, however, as
FIPTool is only useful after all firmware images have been compiled.
#]=======================================================================]

include(ExternalProject)

ExternalProject_Add(
    fiptool-external
    EXCLUDE_FROM_ALL PREFIX
    "fiptool"
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/tools/fiptool"
    INSTALL_COMMAND "")

ExternalProject_Get_Property(fiptool-external INSTALL_DIR)

add_executable(fiptool IMPORTED)
add_dependencies(fiptool fiptool-external)

set_target_properties(
    fiptool
    PROPERTIES
        IMPORTED_LOCATION "${INSTALL_DIR}/src/fiptool-external-build/fiptool")
