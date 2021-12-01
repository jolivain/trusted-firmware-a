#
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# No support for extended PPI and SPI range
GIC_EXT_INTID	:=	0

# Workaround for Qualcomm QGIC2 implementation (disabled by default)
GICV2_QTI_QGIC2	?=	0

GICV2_SOURCES	+=	drivers/arm/gic/v2/gicv2_main.c		\
			drivers/arm/gic/v2/gicv2_helpers.c	\
			drivers/arm/gic/v2/gicdv2_helpers.c

# Set GICv2 build option
$(eval $(call add_define,GIC_EXT_INTID))
$(eval $(call assert_boolean,GICV2_QTI_QGIC2))
$(eval $(call add_define,GICV2_QTI_QGIC2))
