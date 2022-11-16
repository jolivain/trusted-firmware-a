#
# Copyright (c) 2019-2021, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk

CRASH_REPORTING	:=	1

ifeq (${SPM_MM},1)
NEED_BL32		:=	yes
EL3_EXCEPTION_HANDLING	:=	1
GICV2_G0_FOR_EL3	:=	1
endif

# Enable new version of image loading on QEMU platforms
LOAD_IMAGE_V2		:=	1

PLAT_QEMU_PATH		:=	plat/qemu/qemu_sbsa

include plat/qemu/common/common.mk

BL2_SOURCES		+=	$(LIBFDT_SRCS)

BL31_SOURCES		+=	${PLAT_QEMU_PATH}/sbsa_pm.c			\
				${PLAT_QEMU_PATH}/sbsa_topology.c

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

ifeq (${SPM_MM},1)
	BL31_SOURCES		+=	${PLAT_QEMU_COMMON_PATH}/qemu_spm.c
endif

MULTI_CONSOLE_API	:= 1

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT	:= 0

# Use known base for UEFI if not given from command line
# By default BL33 is at FLASH1 base
PRELOADED_BL33_BASE	?= 0x10000000

# Qemu SBSA plafrom only support SEC_SRAM
BL32_RAM_LOCATION_ID	= SEC_SRAM_ID
