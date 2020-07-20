#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED PROJECT_SOURCE_DIR)
	message(FATAL_ERROR "PROJECT_SOURCE_DIR not defined")
endif()

# Include framework files
include(Common/STGT)

#plat_bl_common
stgt_add_src_param(NAME bl1 bl2 bl31 bl32 KEY ARCH SRC
	${PROJECT_SOURCE_DIR}/drivers/arm/pl011/@ARCH@/pl011_console.S
	${PROJECT_SOURCE_DIR}/plat/arm/board/common/@ARCH@/board_arm_helpers.S
)

#plat_bl1, plat_bl2, plat_bl31
stgt_add_src(NAME bl1 bl2 bl31 SRC
	${PROJECT_SOURCE_DIR}/drivers/cfi/v2m/v2m_flash.c
)

#TODO: TRUSTED_BOARD_BOOT
