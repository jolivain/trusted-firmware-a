#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
BuildMessage
============

This module builds upon ``stgt_link_build_messages()`` from the |TFACMF| and
serves as "project specific configuration". |TFACMF| captures how build message
generation does work, and this module captures what information the build
message file shall hold.

This module generates a source file called ``build_message.c``. The generated
file captures the version string and the time stamp of the build.

The version string is extracted from the git working copy, and ``__TIME__``
and ``__DATE__`` ANSI-C macros are used for the timestamp.

The generated file defines two global variables:

   - ``const char build_message[]`` is a string capturing the time stamp
   - ``const char version_string[]`` is a string capturing the version string

When included, the module will create a CMake object library named
``build_message``. Any executable target wishes to use the "build message" shall
call ``stgt_link_build_messages()`` and pass the name of the object library
(``build_message``) to it.

Example::

    include(${PROJECT_SOURCE_DIR}/cmake/BuildMessage/BuildMessage)
    ...
    add_executable(my_exe ...)
    ...
    stgt_link_build_messages(NAME my_exe LIBS build_message)
    ...

#]===]

if(NOT DEFINED PROJECT_SOURCE_DIR)
	message(FATAL_ERROR "PROJECT_SOURCE_DIR not defined")
endif()

# Get git version string
find_package(Git)

execute_process(
	COMMAND ${GIT_EXECUTABLE} -C ${PROJECT_SOURCE_DIR} describe
		--always --dirty --tags
	OUTPUT_VARIABLE GIT_DESC
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Create build_message.c file
configure_file(
	${CMAKE_CURRENT_LIST_DIR}/build_message.c.in
	${CMAKE_CURRENT_BINARY_DIR}/build_message.c
	@ONLY
)

stgt_create(NAME build_message)
stgt_set_target(NAME build_message TYPE objlib ARGS EXCLUDE_FROM_ALL)

stgt_add_src(NAME build_message SRC
	${CMAKE_CURRENT_BINARY_DIR}/build_message.c
)
