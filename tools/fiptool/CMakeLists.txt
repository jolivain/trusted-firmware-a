#-------------------------------------------------------------------------------
# Copyright (c) 2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
Fiptool CMake project
=====================

This project is normally built part of the TF-A build flow, but stand alone
build (where CMake uses this directory as a source) is also supported.

For details on the |FIP| format please refer to
:ref:`Firmware Design`.

The section :ref:`Building Supporting Tools` contains usage information
about ``fiptool``. Please ignore the legacy build information.

Build parameters
----------------

``-DTFA_ROOT_DIR``
   Allows overriding the location of the TF-A working copy. Normally this is
   not needed.

``DCMAKE_INSTALL_PREFIX``
   Allow overriding where the build artifacts are installed. The default value
   is ``<CMAKE_BINARY_DIR>/install``.

Dependencies
------------

The ``fiptool`` runs on the build host and to compile it a compiler
suite running on, and targeting the build host is needed. The project relies on
CMake to detect the environment and find a suitable compiler.

``fiptool`` uses the following libraries:

   - libcypto from OpenSSL.

This project supports CMake version 3.10 and above.

``gnumake`` or a suitable build tool supported by CMake is needed. Officially
only gnumake is supported. Other generators may work or may not.

Building ``fiptool``
--------------------

The usual CMake build process is used. The project is tested with the
``Unix Makefiles`` generator and gnumake. Other generators may work but are not
officially supported.

For build commands and other CMake specific details please refer to section
:ref:`building_with_cmake_label`.

Build artifacts
---------------

Please build the ``install`` target to get reliable access to the build
artifacts. By default, the built executable will be installed under
``<CMAKE_BINARY_DIR>/install``, to avoid polluting the build host. This can be
overridden by using the ``-DCMAKE_INSTALL_PREFIX=<path>`` parameter.

Installation tree::

   <CMAKE_INSTALL_PREFIX>
      |
      \- bin
              fiptool or fiptool.exe

#]===]

cmake_minimum_required(VERSION 3.10)


# Set TF-A root. If a cache entry does exist, then this does nothing.
# Since the fiptool depends on the TF-A working copy because of include files,
# setting this based on a relative path is ok.
# This will only be needed if fiptool is built stand alone.
#
get_filename_component(_TFA_ROOT_DIR "../../" ABSOLUTE)
set(TFA_ROOT_DIR ${_TFA_ROOT_DIR} CACHE PATH "Root of the TF-A working copy.")
unset(_TFA_ROOT_DIR)

project(fiptool LANGUAGES C)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .dll.a .lib)

# Look for our dependencies. Please refer to the TF-A documentation to see how
# to install these on the various supported build hosts.
find_library(CRYPTO_LIB NAMES libcrypto.a libcrypto.dll.a libcrypto crypto REQUIRED)

add_executable(fiptool)

target_sources(fiptool PRIVATE
	fiptool.c
	tbbr_config.c
)

target_include_directories(fiptool PRIVATE
	${TFA_ROOT_DIR}/include/tools_share
)

target_link_libraries(fiptool PRIVATE ${CRYPTO_LIB})

target_compile_definitions(fiptool PRIVATE _GNU_SOURCE _XOPEN_SOURCE=700)
target_compile_options(fiptool PRIVATE -Wall -Werror -pedantic -std=c99)

#Is not configured otherwise install to the build directory.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "..." FORCE)
endif()

install(TARGETS fiptool DESTINATION bin)
