#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

#plat_bl_common
stgt_add_src_param(NAME bl1 bl2 bl31 bl32 KEY ARCH SRC
	${TFA_ROOT_DIR}/drivers/arm/pl011/@ARCH@/pl011_console.S
	${TFA_ROOT_DIR}/plat/arm/board/common/@ARCH@/board_arm_helpers.S
)

#plat_bl1, plat_bl2, plat_bl31
stgt_add_src(NAME bl1 bl2 bl31 SRC
	${TFA_ROOT_DIR}/drivers/cfi/v2m/v2m_flash.c
)

#TODO: TRUSTED_BOARD_BOOT
