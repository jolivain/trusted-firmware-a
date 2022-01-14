#
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20)

#
# Ensure our own CMake modules can be loaded.
#

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

include(ArmConfigOption)

include(TFAMetadata)

#
# Set up the configuration types for both single and multi-configuration
# generators.
#

set(config-types "Debug" "RelWithDebInfo" "MinSizeRel" "Release")
set(default-config "MinSizeRel")

get_property(multi-config GLOBAL PROPERTY "GENERATOR_IS_MULTI_CONFIG")

if(multi-config)
    arm_config_option(
        NAME CMAKE_CONFIGURATION_TYPES HIDDEN
        HELP "Multi-generator configuration types."
        DEFAULT ${config-types})

    arm_config_option(
        NAME CMAKE_DEFAULT_BUILD_TYPE HIDDEN
        HELP "Default multi-generator configuration type."
        DEFAULT "${default-config}")
else()
    arm_config_option(
        NAME CMAKE_BUILD_TYPE
        HELP "Build type."
        STRINGS ${config-types}
        DEFAULT ${default-config}
        FORCE NOT CMAKE_BUILD_TYPE)
endif()

#
# Retrieve the list of platforms from the global metadata file and present them
# to the user.
#

tfa_platforms(platforms)

arm_assert(
    CONDITION platforms
    MESSAGE "No platforms defined!")

arm_config_option(
    NAME TFA_PLATFORM
    HELP "Platform to build."
    STRINGS ${platforms})

tfa_platform_path(TFA_PLATFORM_SOURCE_DIR
    PLATFORM "${TFA_PLATFORM}")

arm_assert(
    CONDITION EXISTS "${TFA_PLATFORM_SOURCE_DIR}"
    MESSAGE "The source directory for the current platform does not exist:\n"

            "${TFA_PLATFORM_SOURCE_DIR}")

#
# Because the platform's source directory might have come from outside, CMake
# may be unable to derive the platform's binary directory automatically. As an
# alternative, we'll use the platform's target name as its binary directory.
#

tfa_platform_target(target
    PLATFORM "${TFA_PLATFORM}")

arm_config_option(
    NAME TFA_PLATFORM_BINARY_DIR HIDDEN
    HELP "Platform binary directory."
    DEFAULT "${CMAKE_CURRENT_BINARY_DIR}/${target}"
    TYPE PATH)

#
# Retrieve the platform variant metadata.
#

tfa_platform_variants(variants
    PLATFORM ${TFA_PLATFORM})

arm_config_option(
    NAME TFA_PLATFORM_VARIANT
    HELP "Platform variant to build."
    STRINGS ${variants})

#
# If no toolchain file has been provided through the default CMake mechanism, we
# use our own mechanism for toolchain selection, with the intention of making it
# easier for users to specify a valid toolchain file.
#
# Rather than ask the user to provide a toolchain file, we instead glob for
# toolchain files in the `toolchains` directory of the repository. The filename
# of each toolchain file becomes a value we accept in the `TFA_TOOLCHAIN`
# configuration option.
#


set(toolchains "")
file(GLOB toolchain-files "toolchains/*.cmake")

arm_assert(
    CONDITION toolchain-files
    MESSAGE "No toolchain files defined!")

foreach(toolchain-file IN LISTS toolchain-files)
    cmake_path(GET toolchain-file STEM toolchain)
    list(APPEND toolchains "${toolchain}")
endforeach()

if(DEFINED TFA_SOURCE_DIR)
    arm_assert(
        CONDITION TFA_TOOLCHAIN STREQUAL TFA_TOOLCHAIN_NEW
        MESSAGE "Sorry, the toolchain cannot be changed after configuration "
                "has succeeded. Please clear the CMake cache (e.g. by wiping "
                "the build directory) and configure again.\n"

                "The currently configured toolchain is ${TFA_TOOLCHAIN_NEW}, "
                "but ${TFA_TOOLCHAIN} was requested.")
endif()

arm_config_option(
    NAME TFA_TOOLCHAIN
    HELP "Toolchain name."
    STRINGS ${toolchains}
    DEFAULT "GNU"
    DEPENDS NOT DEFINED TFA_SOURCE_DIR
    ELSE "${TFA_TOOLCHAIN}")

#
# Allow the user to configure the toolchain file. This uses the standard CMake
# mechanism for describing the toolchain.
#

if(DEFINED TFA_SOURCE_DIR)
    arm_assert(
        CONDITION CMAKE_TOOLCHAIN_FILE STREQUAL CMAKE_TOOLCHAIN_FILE_NEW
        MESSAGE "Sorry, the toolchain cannot be changed after configuration "
                "has succeeded. Please clear the CMake cache (e.g. by wiping "
                "the build directory) and configure again.")
endif()

arm_config_option(
    NAME CMAKE_TOOLCHAIN_FILE
    HELP "Path to the CMake toolchain file."
    TYPE FILEPATH
    DEFAULT "${CMAKE_CURRENT_LIST_DIR}/toolchains/${TFA_TOOLCHAIN}.cmake"
    DEPENDS NOT DEFINED TFA_SOURCE_DIR
    ELSE "${CMAKE_TOOLCHAIN_FILE}")

arm_assert(
    CONDITION EXISTS "${CMAKE_TOOLCHAIN_FILE}"
    MESSAGE "Couldn't find toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

#
# We're done with very early setup, so we can now create the project. This will
# do some of the automatic compiler detection, which we need for setting up
# further configuration options.
#
# Note that this creates the following version variables:
#
# - `TFA_VERSION`
# - `TFA_VERSION_MAJOR`
# - `TFA_VERSION_MINOR`
# - `TFA_VERSION_PATCH`
# - `TFA_VERSION_TWEAK`
#
# Also, these directory variables:
#
# - `TFA_SOURCE_DIR`
# - `TFA_BINARY_DIR`
#
# Don't swap `C` and `ASM`. Per the CMake documentation:
#
# > If enabling `ASM`, list it last so that CMake can check whether compilers
# > for other languages like `C` work for assembly too.
#

project(TFA VERSION 2.5 LANGUAGES C ASM)
