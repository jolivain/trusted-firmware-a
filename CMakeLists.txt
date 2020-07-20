#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
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

# Set default build type
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type.")

#Append project root to module path to allow root relative cmake includes.
#TODO: make a clean split between cmake modules living in CMake repo, and components.
#      (included like include(foo/bar) vs include (foo/bar.cmake))
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Download/find the cmake framework.
include(GetTFACMF)

#[===[.rst:
.. cmake:variable:: TARGET_PLATFORM

   The name of the target platform.

.. cmake:variable:: COMPILER

   Name of the compiler toolchain.

.. cmake:variable:: SW_CONFIG

   Name of the software configuration to be used.

#]===]

# Set default values for the three main configuration sets.
set(TARGET_PLATFORM "fvp" CACHE STRING "Target platform name.")
set(COMPILER "GCC" CACHE STRING "Compiler name.")
set(SW_CONFIG "Default" CACHE STRING "Software configuration settings.")

# Add project specific paths
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Ext")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/HwPlat")

#[===[.rst:
.. cmake:variable:: CMAKE_SYSTEM_NAME

   This is a "built-in" CMake variable. It is set to the value ``Embedded`` to
   signal CMake this is a cross-compilation build.
   This system is introduced by |TFACMF| and it is mandatory to be used. For
   details please refer to TFACMF documentation and to the
   `CMake documentation <https://cmake.org/cmake/help/latest/variable/CMAKE_SYSTEM_NAME.html>`_.


.. cmake:variable::CMAKE_SYSTEM_PROCESSOR

   This is a ``built-in`` CMake variable. It is set to ``arm``. For details plese see
   and to the
   the `CMake documentation <https://cmake.org/cmake/help/latest/variable/CMAKE_SYSTEM_PROCESSOR.html>`_.

#]===]
# Set CMake system parameters
set(CMAKE_SYSTEM_NAME Embedded)
set(CMAKE_SYSTEM_PROCESSOR arm)

#[===[.rst:
.. cmake:variable:: PROJECT_SOURCE_DIR

   This variable holds the location of the root directory of the TF-A source in
   the file system. It is set when ``project()`` is first called.
#]===]
# Start CMake project
project(tfa LANGUAGES C ASM)

# Find and include the needed files.
find_package(${TARGET_PLATFORM} REQUIRED)

#Include config files
include(${PROJECT_SOURCE_DIR}/configs/ConfigBLspecific.cmake)
include(${PROJECT_SOURCE_DIR}/configs/ConfigDefault.cmake)
include(${PROJECT_SOURCE_DIR}/configs/Config${COMPILER}.cmake)

#Add sub-projects
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libc)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/xlat_tables_v2)

# Create STGT targets
include(${PROJECT_SOURCE_DIR}/cmake/Targets.cmake)

#TODO: some of these shall be converted to sub-projects
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
