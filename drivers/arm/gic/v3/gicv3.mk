#
# Copyright (c) 2013-2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

GICV3_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
			drivers/arm/gic/v3/gicv3_main.c		\
			drivers/arm/gic/v3/gicv3_helpers.c	\
			drivers/arm/gic/v3/gicdv3_helpers.c	\
			drivers/arm/gic/v3/gicrv3_helpers.c	\
			plat/common/plat_gicv3.c

ifneq (${GICV3_OVERRIDE_DISTIF_PWR_OPS}, 1)
GICV3_SOURCES	+=	drivers/arm/gic/v3/arm_gicv3_common.c
$(warning "ARM_GICV3_COMMON")
endif

ifeq (${GICV3_IMPL_GIC600}, 1)
# GIC-600 sources
GICV3_SOURCES	+=	drivers/arm/gic/v3/gic600.c
$(warning "GIC-600")
ifeq (${GIC600_MULTICHIP}, 1)
GICV3_SOURCES	+=	drivers/arm/gic/v3/gic600_multichip.c
$(warning "GIC600_MULTICHIP")
endif
else ifeq (${GICV3_IMPL_GIC500}, 1)
# GIC-500 sources
GICV3_SOURCES	+=	drivers/arm/gic/v3/gic500.c
$(warning "GIC-500")
endif

ifeq (${PLAT_ARM_GIC_INIT}, 1)
GICV3_SOURCES	+=	plat/arm/common/arm_gicv3.c
endif