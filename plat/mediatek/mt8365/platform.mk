#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT     := plat/mediatek
MTK_PLAT_SOC := ${MTK_PLAT}/${PLAT}

include drivers/arm/gic/v3/gicv3.mk
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES := -I${MTK_PLAT}/common/	       \
		 -I${MTK_PLAT}/drivers/	       \
		 -I${MTK_PLAT}/drivers/gic600/ \
		 -I${MTK_PLAT}/include/	       \
		 -I${MTK_PLAT_SOC}/drivers/    \
		 -I${MTK_PLAT_SOC}/include/

# BL31
BL31_SOURCES += common/desc_image_load.c		      \
		drivers/delay_timer/delay_timer.c	      \
		drivers/delay_timer/generic_delay_timer.c     \
		lib/bl_aux_params/bl_aux_params.c	      \
		lib/cpus/aarch64/cortex_a53.S		      \
		plat/common/aarch64/crash_console_helpers.S   \
		plat/common/plat_gicv3.c		      \
		plat/common/plat_psci_common.c		      \
		${GICV3_SOURCES}			      \
		${MTK_PLAT_SOC}/aarch64/plat_helpers.S	      \
		${MTK_PLAT_SOC}/aarch64/platform_common.c     \
		${MTK_PLAT_SOC}/bl31_plat_setup.c	      \
		${MTK_PLAT_SOC}/drivers/spm/mt_spm.c	      \
		${MTK_PLAT_SOC}/drivers/spm/mt_spm_internal.c \
		${MTK_PLAT_SOC}/mt8365_console_setup.c	      \
		${MTK_PLAT_SOC}/plat_pm.c		      \
		${MTK_PLAT_SOC}/plat_topology.c		      \
		${MTK_PLAT}/common/params_setup.c	      \
		${MTK_PLAT}/drivers/gic600/mt_gic_v3.c	      \
		${XLAT_TABLES_LIB_SRCS}

# Enable workarounds for selected Cortex-A53 erratas
ERRATA_A53_855873 := 1
ERRATA_A53_1530924 := 1
