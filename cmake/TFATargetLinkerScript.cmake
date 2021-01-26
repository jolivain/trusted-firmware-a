#
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include_guard()

include(TFAPreprocessSource)

# .rst:
#
# .. command:: tfa_target_linker_script
#
# Add a linker script to a target.
#
# .. cmake:: tfa_target_linker_script(<target> <source>)
#
# This macro preprocesses a linker script ``<source>`` for the target
# ``<target>``. The include directories and compile definitions given to the
# preprocessor are given by the ``INCLUDE_DIRECTORIES` and
# ``COMPILE_DEFINITIONS`` properties of the target.
macro(tfa_target_linker_script target source)
    set(subtarget "${target}-lds")
    set(output "${CMAKE_CURRENT_BINARY_DIR}/${target}.ld")

    tfa_preprocess_source(${subtarget} "${source}" "${output}")

    set_target_properties(
        ${subtarget}
        PROPERTIES
            INCLUDE_DIRECTORIES
                "$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>"
            COMPILE_DEFINITIONS
                "$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>;__LINKER__"
            COMPILE_OPTIONS "$<TARGET_PROPERTY:${target},COMPILE_OPTIONS>")

    #
    # Add the linker script to the dependencies of the target.
    #

    add_dependencies(${target} ${subtarget})

    set_target_properties(${target}
                          PROPERTIES INTERFACE_LINK_DEPENDS "${output}")

    if(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
        target_link_options(${target} PUBLIC "LINKER:--scatter"
                                             "LINKER:${output}")
    else()
        target_link_options(${target} PUBLIC "LINKER:-T" "LINKER:${output}")
    endif()
endmacro()
