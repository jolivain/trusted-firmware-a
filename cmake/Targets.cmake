#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#[===[.rst:
Targets
=======

This file contains forward declarations for all BL targets. This allows sources
to be added to them before or after the blxx.cmake files are included, without
throwing an error due to non-existent targets.

Certain targets should only be built in specific situations. Targets are set to
not be built by setting the EXCLUDE_FROM_ALL property to 1 on the target.

#]===]


# BL1
stgt_create(NAME bl1)
stgt_add_setting(NAME bl1 GROUPS default compiler hw_plat bl1_specific)
stgt_set_target(NAME bl1 TYPE exe)

# BL2
stgt_create(NAME bl2)
stgt_add_setting(NAME bl2 GROUPS default compiler hw_plat bl2_specific)
stgt_set_target(NAME bl2 TYPE exe)

# BL31 (AArch64 Only)
stgt_create(NAME bl31)
stgt_add_setting(NAME bl31 GROUPS default compiler hw_plat bl31_specific)
stgt_set_target(NAME bl31 TYPE exe)

stgt_get_param(NAME bl31 KEY ARCH RET _arch)
if (NOT _arch STREQUAL aarch64)
	set_target_properties(bl31 PROPERTIES EXCLUDE_FROM_ALL 1)
endif()
unset(_arch)

# BL32 (SP_MIN Only: AArch32 Only)
stgt_create(NAME bl32)
stgt_add_setting(NAME bl32 GROUPS default compiler hw_plat bl32_specific)
stgt_set_target(NAME bl32 TYPE exe)
if (NOT AARCH32_SP)
	set_target_properties(bl31 PROPERTIES EXCLUDE_FROM_ALL 1)
endif()
