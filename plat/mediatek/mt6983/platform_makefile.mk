#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/mediatek/build_helpers/mtk_build_helpers.mk

include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES	:=	-I${MTK_PLAT}/include		\
				-I${MTK_PLAT}/include/${MTK_SOC} \
				-I${MTK_PLAT}/include/${ARCH_VERSION} \
				-I${MTK_PLAT} \
				-Idrivers/arm/gic \

MODULES-y += $(MTK_PLAT)/common
MODULES-y += $(MTK_PLAT)/drivers/mtk_console
MODULES-y += $(MTK_PLAT)/drivers/el3_uptime
MODULES-y += $(MTK_PLAT)/lib/pm
MODULES-y += $(MTK_PLAT)/lib/mtk_init
MODULES-y += $(MTK_PLAT)/lib/stack_protector
MODULES-y += $(MTK_PLAT)/helpers
MODULES-y += $(MTK_PLAT)/topology
MODULES-$(CONFIG_MTK_UART)           += $(MTK_PLAT)/common/drivers/uart
MODULES-$(CONFIG_MTK_INTERRUPT)      += $(MTK_PLAT)/drivers/interrupt
MODULES-$(CONFIG_BOOT_TAG)           += $(MTK_PLAT)/lib/boot_tag
MODULES-$(CONFIG_MTK_GZ)             += $(MTK_PLAT)/geniezone

BL31_SOURCES		+=	drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch64/cortex_a510.S	\
				lib/cpus/aarch64/cortex_a710.S	\
				lib/cpus/aarch64/cortex_x2.S	\
				${GICV3_SOURCES}						\
				${XLAT_TABLES_LIB_SRCS}					\
				plat/common/plat_gicv3.c				\
				plat/common/plat_psci_common.c			\
				plat/arm/common/arm_gicv3.c				\
				plat/common/aarch64/crash_console_helpers.S \
				$(MTK_PLAT)/$(MTK_SOC)/plat_mmap.c

include drivers/arm/gic/v3/gicv3.mk

include plat/mediatek/build_helpers/mtk_build_helpers_epilogue.mk
