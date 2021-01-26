#-------------------------------------------------------------------------------
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------

#[===[.rst:
Top-level CMakeLists
====================

This file is the main entrypoint of the CMake based build system, or by using
CMake terminology "the top-level cmake file".

Its main purpose is to integrate all the sub-modules into a single build flow,
and to set some global settings.
#]===]

# Set required CMake version
cmake_minimum_required(VERSION 3.13)

# Add project specific paths
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Ext")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/HwPlat")

# Set default build type
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type.")

#Append project root to module path to allow root relative cmake includes.
#TODO: make a clean split between cmake modules living in CMake repo, and components.
#      (included like include(foo/bar) vs include (foo/bar.cmake))
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Start CMake project. After this line, PROJECT_SOURCE_DIR is defined.
project(tfa LANGUAGES C ASM)

# Download/find the cmake framework.
include(GetTFACMF)

#[===[.rst:
.. cmake:variable:: TFA_TARGET_PLATFORM

   The name of the target platform.
#]===]

# Set default values for the three main configuration sets.
set(TFA_TARGET_PLATFORM "fvp" CACHE STRING "Target platform name.")

# Find and include the needed files.
find_package(${TFA_TARGET_PLATFORM} REQUIRED)

# Include config files
include(${PROJECT_SOURCE_DIR}/configs/ConfigBLspecific.cmake)
include(${PROJECT_SOURCE_DIR}/configs/ConfigDefault.cmake)
include(${PROJECT_SOURCE_DIR}/configs/Config${CMAKE_C_COMPILER_ID}.cmake)

# Add sub-projects
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libc)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/xlat_tables_v2)

# Create STGT targets
include(${PROJECT_SOURCE_DIR}/cmake/Targets.cmake)

# TODO: some of these shall be converted to sub-projects
include(${HW_PLAT_TARGET})
include(BuildMessage/BuildMessage)
include(${PROJECT_SOURCE_DIR}/lib/libfdt/libfdt.cmake)
include(${PROJECT_SOURCE_DIR}/bl1/bl1.cmake)
include(${PROJECT_SOURCE_DIR}/bl2/bl2.cmake)
include(${PROJECT_SOURCE_DIR}/bl31/bl31.cmake)

if(AARCH32_SP)
	include(${PROJECT_SOURCE_DIR}/bl32/${AARCH32_SP}/${AARCH32_SP}.cmake)
endif()

include(${PROJECT_SOURCE_DIR}/lib/psci/psci_lib.cmake)
include(${PROJECT_SOURCE_DIR}/common/bl_common.cmake)

# Include FW image creation rules.
include(${HW_PLAT_IMAGE})
