#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

include(ArmConfigOption)
include(ArmExpand)
include(ArmPreprocessSource)
include(ArmTargetLinkerScript)

#
# Read the global metadata file. This is a JSON-formatted file that contains
# information about the contents of the repository that are relevant to the
# build system.
#

file(READ "metadata.json" TFA_METADATA)
arm_expand(OUTPUT TFA_METADATA STRING "${TFA_METADATA}")

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
