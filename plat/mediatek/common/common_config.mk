#
# Copyright (c) 2020, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS := 1
MULTI_CONSOLE_API := 1
COLD_BOOT_SINGLE_CPU := 1
# Build flag to include AArch32 registers in cpu context save and restore during
# world switch. This flag must be set to 0 for AArch64-only platforms.
CTX_INCLUDE_AARCH32_REGS := 0
PLAT_XLAT_TABLES_DYNAMIC := 1
# enable this definition to print irq dump status in tf-a
GIC_DEBUG := 0
# Enable stack protector.
# Allowed values are "all", "strong", "default" and "none"
ENABLE_STACK_PROTECTOR := strong
# AMU, Kernel will access amuserenr_el0 if PE supported
# Firmware _must_ implement AMU support
ENABLE_AMU := 1
VENDOR_EXTEND_PUBEVENT_ENABLE := 1

# MTK define options
MTK_SIP_KERNEL_BOOT_ENABLE := 1
MTK_BL33_IS_64BIT := 0
MTK_ADAPTED := 1
MT_MPU_DEBUG := 0

#MTK module config
CONFIG_BOOT_TAG := n
CONFIG_MTK_AEE := n
CONFIG_MTK_BOOT_TIME_PROFILE := n
CONFIG_MTK_MBLOCK := n
CONFIG_MTK_INTERRUPT := y
CONFIG_MTK_RGU := y
CONFIG_MTK_UART := y
CONFIG_MTK_SYSTIMER := y
CONFIG_MEM_CONSOLE := y
CONFIG_MTK_IPI := y
CONFIG_MTK_BSP_UT := n

#UART baudrate
UART_BAUDRATE := 921600

# TF-A ram dump
CONFIG_MTK_BL31_RAMDUMP := n

# Map debug buffer as non cacheable
MTK_DEBUG_BUF_MAP_NON_CACHEABLE := y
