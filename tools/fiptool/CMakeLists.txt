#
# Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
FIPTool
=======

This project is normally built as part of the TF-A build flow, but standalone
builds (where the FIPTool directory is used as the CMake source directory) are
also supported.

For details on the |FIP| format please refer to :ref:`Firmware Design`.

The section :ref:`Building Supporting Tools` contains usage information
about ``fiptool``. Please ignore the legacy build information.

Building ``fiptool``
--------------------

The usual CMake build process is used, and requires no additional configuration.
Only the ``Unix Makefiles`` generator is officially supported, but other
generators should work without issue. Note that FIPTool is intended to compiled
using the host system toolchain.

Dependencies
------------

FIPTool relies on OpenSSL's crypto component library. Ubuntu users can install
this with::

    sudo apt install libssl-dev
#]=======================================================================]

cmake_minimum_required(VERSION 3.4)

project(fiptool LANGUAGES C)

find_package(OpenSSL COMPONENTS Crypto REQUIRED)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

add_executable(fiptool)

target_compile_definitions(fiptool PRIVATE "_GNU_SOURCE" "_XOPEN_SOURCE=700")
target_include_directories(fiptool PRIVATE "../../include/tools_share")
target_sources(fiptool PRIVATE "fiptool.c" "tbbr_config.c")
target_link_libraries(fiptool PRIVATE OpenSSL::Crypto)

install(TARGETS fiptool DESTINATION "bin")
