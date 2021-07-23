#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20)

#
# Ensure our own CMake modules can be loaded.
#

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

include(ArmConfigOption)
include(ArmExpand)
include(ArmPreprocessSource)
include(ArmTargetLinkerScript)

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
        DEFAULT "${config-types}")

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
# Read the global metadata file. This is a JSON-formatted file that contains
# information about the contents of the repository that are relevant to the
# build system.
#

file(READ "metadata.json" TFA_METADATA)
arm_expand(OUTPUT TFA_METADATA STRING "${TFA_METADATA}")

#
# Allow the user to provide their own platform list metadata. This allows
# developers to use out-of-tree platforms (platforms that live outside of this
# repository). The platforms list given by this file is superimposed onto the
# global metadata file.
#

arm_config_option(
    NAME TFA_PLATFORMS_METADATA_PATH
    HELP "Path to an alternative platforms metadata file."
    TYPE FILEPATH ADVANCED)

if(TFA_PLATFORMS_METADATA_PATH)
    cmake_path(GET TFA_PLATFORMS_METADATA_PATH
        PARENT_PATH TFA_PLATFORMS_METADATA_DIR)

    arm_assert(
        CONDITION EXISTS "${TFA_PLATFORMS_METADATA_PATH}"
        MESSAGE "The platforms metadata file does not exist: "
                "${TFA_PLATFORMS_METADATA_PATH}")

    file(READ "${TFA_PLATFORMS_METADATA_PATH}" platforms-metadata)
    arm_expand(OUTPUT platforms-metadata STRING "${platforms-metadata}")

    string(JSON platforms-count LENGTH "${platforms-metadata}")
    math(EXPR platforms-count "${platforms-count} - 1")

    foreach(i RANGE ${platforms-count})
        string(JSON platform-name MEMBER "${platforms-metadata}" ${i})
        string(JSON platform-path GET "${platforms-metadata}"
            "${platform-name}")

        string(JSON TFA_METADATA SET "${TFA_METADATA}" "platforms"
            "${platform-name}" "\"${platform-path}\"")
    endforeach()
endif()

#
# Retrieve the list of platforms from the global metadata file and present them
# to the user.
#

set(platforms)

string(JSON platforms-metadata GET "${TFA_METADATA}" "platforms")

string(JSON platforms-count LENGTH "${platforms-metadata}")
math(EXPR platforms-count "${platforms-count} - 1")

foreach(i RANGE ${platforms-count})
    string(JSON platform MEMBER "${platforms-metadata}" ${i})
    list(APPEND platforms "${platform}")
endforeach()

arm_config_option(
    NAME TFA_PLATFORM
    HELP "Platform to build."
    STRINGS ${platforms})

#
# Read the platform metadata file, which is how the platform communicates
# structured data to us.
#

string(JSON TFA_PLATFORM_SOURCE_DIR GET "${TFA_METADATA}"
    "platforms" "${TFA_PLATFORM}")

arm_assert(
    CONDITION EXISTS "${TFA_PLATFORM_SOURCE_DIR}/platform.json"
    MESSAGE "This platform has not provided a platform metadata file.")

file(READ "${TFA_PLATFORM_SOURCE_DIR}/platform.json" TFA_PLATFORM_METADATA)
arm_expand(OUTPUT TFA_PLATFORM_METADATA STRING "${TFA_PLATFORM_METADATA}")

#
# Because the platform's source directory might have come from outside, CMake
# may be unable to derive the platform's binary directory automatically. As an
# alternative, we'll use the platform's target name as its binary directory.
#

string(JSON TFA_PLATFORM_TARGET ERROR_VARIABLE error
    GET "${TFA_PLATFORM_METADATA}" "target")

if(error)
    message(FATAL_ERROR
        "This platform's metadata file has not provided a target name.")
endif()

set(TFA_PLATFORM_BINARY_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/${TFA_PLATFORM_TARGET}")

#
# If no toolchain file has been provided through the default CMake mechanism, we
# use our own mechanism for toolchain selection, with the intention of making it
# easier for users to specify a valid toolchain file.
#
# Rather than ask the user to provide a toolchain file, we instead glob for
# toolchain files in the `toolchains` directory of the repository. The filename
# of each toolchain file becomes a value we accept in the `TFA_TOOLCHAINS`
# configuration option.
#

set(toolchains "")
file(GLOB toolchain-files "toolchains/*.cmake")

foreach(toolchain-file IN LISTS toolchain-files)
    cmake_path(GET toolchain-file STEM toolchain)
    list(APPEND toolchains "${toolchain}")
endforeach()

if(DEFINED TFA_TOOLCHAIN_OLD)
    arm_config_option_override(
        NAME TFA_TOOLCHAIN
        VALUE "${TFA_TOOLCHAIN_OLD}")
endif()

arm_config_option(
    NAME TFA_TOOLCHAIN
    HELP "Toolchain name."
    STRINGS ${toolchains}
    DEFAULT "GNU")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    arm_config_option_override(
        NAME CMAKE_TOOLCHAIN_FILE
        VALUE "${CMAKE_CURRENT_LIST_DIR}/toolchains/${TFA_TOOLCHAIN}.cmake")
endif()

#
# Allow the user to configure the toolchain file. This uses the standard CMake
# mechanism for describing the toolchain.
#

if(DEFINED CMAKE_TOOLCHAIN_FILE_OLD)
    arm_config_option_override(
        NAME CMAKE_TOOLCHAIN_FILE
        VALUE "${CMAKE_TOOLCHAIN_FILE_OLD}")
endif()

arm_config_option(
    NAME CMAKE_TOOLCHAIN_FILE
    HELP "Path to the CMake toolchain file."
    TYPE FILEPATH)

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

