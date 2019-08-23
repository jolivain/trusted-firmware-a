#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------


#[===[.rst:
Findfiptool
================

The TF-A target platforms which use the |FIP| format, depend on the
``fiptool``. This is a small utility written in C, able to manipulate |FIP|
packages. The tool lives in the TF-A repository, and needs to be built as an
early build step.

This module builds the ``fiptool`` targetting the build host or by CMake
terminology the ``host platform``. As a result separation is needed form the
TF-A build flow because CMake does not support using multiple compilers in the
same flow. This module implements the separation by building ``fiptool`` using
:cmake:command:`exeute_process()`.

The module uses :cmake:command:`find_program` to make the build
executable available to the TF-A build. As a result by setting the
:cmake:variable:`fiptool` on the command line, a pre-built executable can
be used.

Examples:

    Assuming ``fiptool`` is available at ``/foo/bar/my_fiptool``:

    .. code-block:: shell

        cmake -Dfiptool=/foo/bar/my_fiptool

    Assuming ``fiptool`` is available on the PATH:

    .. code-block:: shell

        cmake -Dfiptool=fiptool

#]===]

# Explanation for the unset=CC solution:
# 1. If the CC environment variable is set, CMake will prefer to use this as a
#    compiler instead of running the normal toolchain finding process.
# 2. When running a child process from CMake using execute_process, CMake will set
#    the CC variable in the child process' environment to the compiler of the
#    parent process.
# 3. This means if the child process is CMake, by default it will use the compiler
#    of the parent process. This is a problem if the child process should use a
#    different compiler.
# 4. To avoid this, we have to manually unset the CC variable in the environment
#    of the child process.

#TODO: revise paths used in this file
find_program(fiptool NAMES fiptool fiptool.exe HINTS ${PROJECT_SOURCE_DIR}/tools/fiptool)

if(NOT fiptool)
	execute_process(
		COMMAND
		${CMAKE_COMMAND} -E env --unset=CC
		${CMAKE_COMMAND} -G "Unix Makefiles"
		-S ${PROJECT_SOURCE_DIR}/tools/fiptool
		-B ${CMAKE_BINARY_DIR}/fiptool
		-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/fiptool
		-DPROJECT_SOURCE_DIR:PATH=${PROJECT_SOURCE_DIR}
	)
	execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/fiptool -- install)
	find_program(fiptool NAMES fiptool fiptool.exe HINTS ${CMAKE_BINARY_DIR}/fiptool/bin)
endif()
