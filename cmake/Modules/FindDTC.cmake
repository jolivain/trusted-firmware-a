#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
Finddtc
=======

The Find Device Tree Compiler module looks for the ``dtc`` executable. For
searching, the :cmake:command:`find_program` is used. For details on the search
process and where the function looks for the executable please refer to the
`CMake documentation`_

The variable :cmake:variable:`dtc_executable` can be set on the command line if
the tool is installed at a non-standard location.

Example:

    Assuming ``dtc`` is available at ``/foo/bar/my_dtc``:

.. code:: shell

    cmake -DDTC_EXECUTABLE=/foo/bar/my_dtc

The module defines the function :cmake:command:`dtc_compile` which can be used
in CMake scripts to execute the compiler during the build.

.. _CMake documentation: https://cmake.org/cmake/help/v3.14/command/find_program.html

#]=======================================================================]

# Find the dtc executable.
find_program(dtc_executable NAMES dtc dtc.exe REQUIRED)

#[=======================================================================[.rst:
.. cmake:command:: dtc_compile

    .. code:: cmake

       dtc_compile(SRC "foo.dts" DST "foo.dtb")

   Compile DTC source. The source file will be run trough the C pre-processor
   before compilation. The ``<TF-A root>/fdts`` and ``<TF-A root>/include``
   directories will be set as include paths.

   Inputs:

       ``SRC``
       Path to source file.

       ``DST``
       Path to binary output.

#]=======================================================================]

# Include utilities from TFACMF
include(Common/Utils)

function(dtc_compile)
    set(_OPTIONS_ARGS)
    set(_ONE_VALUE_ARGS SRC DST)
    set(_MULTI_VALUE_ARGS)
    cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}"
                          "${_MULTI_VALUE_ARGS}" ${ARGN})

    check_args(dtc_compile SRC DST)

    get_filename_component(_src_name ${_MY_PARAMS_SRC} NAME_WE)
    get_filename_component(_src_ext ${_MY_PARAMS_SRC} EXT)
    get_filename_component(_dst_ext ${_MY_PARAMS_DST} EXT)

    if(NOT ("${_src_ext}" STREQUAL ".dts" AND "${_dst_ext}" STREQUAL ".dtb"))
        message(WARNING "dtc_compile(): extension mismatch")
    endif()

    set(_src_preproc ${CMAKE_BINARY_DIR}/${_src_name}.pre.dts)

    if(NOT COMMAND compiler_preprocess_file)
        message(
            FATAL_ERROR
                "The compiler specific file of TFACMF must be included before this file."
        )
    endif()

    compiler_preprocess_file(
        SRC
        ${_MY_PARAMS_SRC}
        DST
        ${_src_preproc}
        INCLUDES
        ${PROJECT_SOURCE_DIR}/fdts
        ${PROJECT_SOURCE_DIR}/include)

    add_custom_command(
        OUTPUT ${_MY_PARAMS_DST}
        DEPENDS ${_src_preproc}
        COMMAND ${dtc_executable} -I dts -O dtb -o ${_MY_PARAMS_DST}
                ${_src_preproc})
endfunction()
