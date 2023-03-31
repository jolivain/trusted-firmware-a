#
# Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

CPUS_ENABLE		+=	aem_generic 				\
				cortex_a15

BL32_SOURCES		+=	plat/qemu/sp_min/sp_min_setup.c		\
				plat/qemu/aarch32/plat_helpers.S	\
				plat/qemu/qemu_pm.c			\
				plat/qemu/topology.c

BL32_SOURCES		+=	plat/common/aarch32/platform_mp_stack.S \
				plat/common/plat_psci_common.c \
				plat/common/plat_gicv2.c


BL32_SOURCES		+=	drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/common/gic_common.c
