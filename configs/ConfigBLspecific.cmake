#-------------------------------------------------------------------------------
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#-------------------------------------------------------------------------------

include(Common/Group)

group_new(NAME bl1_specific)
group_add(NAME bl1_specific TYPE DEFINE KEY IMAGE_BL1)
group_add(
    NAME
    bl1_specific
    TYPE
    LDFLAG
    KEY
    "-Wl,-Map"
    VAL
    "${CMAKE_BINARY_DIR}/bl1.map")

group_new(NAME bl2_specific)
group_add(NAME bl2_specific TYPE DEFINE KEY IMAGE_BL2)
group_add(
    NAME
    bl2_specific
    TYPE
    LDFLAG
    KEY
    "-Wl,-Map"
    VAL
    "${CMAKE_BINARY_DIR}/bl2.map")

group_new(NAME bl31_specific)
group_add(NAME bl31_specific TYPE DEFINE KEY IMAGE_BL31)
group_add(
    NAME
    bl31_specific
    TYPE
    LDFLAG
    KEY
    "-Wl,-Map"
    VAL
    "${CMAKE_BINARY_DIR}/bl31.map")

group_new(NAME bl32_specific)
group_add(NAME bl32_specific TYPE DEFINE KEY IMAGE_BL32)
group_add(
    NAME
    bl32_specific
    TYPE
    LDFLAG
    KEY
    "-Wl,-Map"
    VAL
    "${CMAKE_BINARY_DIR}/bl32.map")
