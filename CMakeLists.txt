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

arm_config_option(
    NAME TFA_PLATFORM
    HELP "Platform to build."
    STRINGS ${platforms})

tfa_platform_path(TFA_PLATFORM_SOURCE_DIR
    PLATFORM "${TFA_PLATFORM}")

#
# Because the platform's source directory might have come from outside, CMake
# may be unable to derive the platform's binary directory automatically. As an
# alternative, we'll use the platform's target name as its binary directory.
#

tfa_platform_target(target
    PLATFORM "${TFA_PLATFORM}")

set(TFA_PLATFORM_BINARY_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/${target}")

#
# Configure the Arm architecture to target. In the legacy build system the
# version parts were configured through `ARM_ARCH_MAJOR` and `ARM_ARCH_MINOR`.
# Here, we instead accept a string in the branded format - "ArmvX.Y-Z" - and
# derive the versions from that.
#

tfa_platform_architectures(architectures
    PLATFORM ${TFA_PLATFORM})

tfa_legacy_option(
    OLD ARM_ARCH_MAJOR ARM_ARCH_MINOR
    NEW TFA_ARCH)

arm_config_option(
    NAME TFA_ARCH
    HELP "Arm architecture to target."
    STRINGS ${architectures})

tfa_parse_arch_version(
    VERSION "${TFA_ARCH}"
    MAJOR TFA_ARCH_VERSION_MAJOR
    MINOR TFA_ARCH_VERSION_MINOR
    CLASS TFA_ARCH_VERSION_CLASS)

set(TFA_ARCH_VERSION "${TFA_ARCH_VERSION_MAJOR}.${TFA_ARCH_VERSION_MINOR}")

string(TOUPPER "${TFA_ARCH}" TFA_ARCH_UPPER)
string(MAKE_C_IDENTIFIER "${TFA_ARCH_UPPER}" TFA_ARCH_UPPER_IDENT)
string(TOLOWER "${TFA_ARCH}" TFA_ARCH_LOWER)
string(MAKE_C_IDENTIFIER "${TFA_ARCH_LOWER}" TFA_ARCH_LOWER_IDENT)

arm_config_option_override(
    NAME TFA_ARCH_${TFA_ARCH_UPPER_IDENT}
    VALUE TRUE)

#
# Configure the instruction set state. In the legacy build system this was done
# through `ARCH`, which took `aarch32` or `aarch64` regardless of the platform.
# We do something similar here, except we ask the platform to provide the list
# of architectures it supports if using an ISA supporting multiple instruction
# set states.
#

tfa_platform_architecture_states(states
    PLATFORM "${TFA_PLATFORM}"
    ARCHITECTURE "${TFA_ARCH}")

tfa_legacy_option(
    OLD ARCH
    NEW TFA_ARCH_STATE)

arm_config_option(
    NAME TFA_ARCH_STATE
    HELP "Arm instruction set state."
    STRINGS ${states})

string(TOUPPER ${TFA_ARCH_STATE} TFA_ARCH_STATE_UPPER)
string(MAKE_C_IDENTIFIER ${TFA_ARCH_STATE_UPPER} TFA_ARCH_STATE_UPPER_IDENT)
string(TOLOWER ${TFA_ARCH_STATE} TFA_ARCH_STATE_LOWER)
string(MAKE_C_IDENTIFIER ${TFA_ARCH_STATE_LOWER} TFA_ARCH_STATE_LOWER_IDENT)

arm_config_option_override(
    NAME TFA_ARCH_STATE_${TFA_ARCH_STATE_UPPER_IDENT}
    VALUE TRUE)

#
# Create convenience variables for every architecture and state described by the
# metadata. This allows build system developers to use short-hand variable names
# for basic boolean operations on the current architecture, instead of relying
# on string comparisons.
#

tfa_architectures(architectures)

foreach(architecture IN LISTS architectures)
    string(MAKE_C_IDENTIFIER ${architecture} architecture-ident)
    string(TOUPPER ${architecture-ident} architecture-ident)

    arm_config_option(
        NAME TFA_ARCH_${architecture-ident} HIDDEN
        HELP "Is the ${architecture} architecture in use?"
        DEFAULT FALSE)

    tfa_architecture_states(states
        ARCHITECTURE "${architecture}")

    foreach(state IN LISTS states)
        string(MAKE_C_IDENTIFIER ${state} state-ident)
        string(TOUPPER ${state-ident} state-ident)

        arm_config_option(
            NAME TFA_ARCH_STATE_${state-ident} HIDDEN
            HELP "Is the ${architecture} ${state} state in use?"
            DEFAULT FALSE)
    endforeach()
endforeach()

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

