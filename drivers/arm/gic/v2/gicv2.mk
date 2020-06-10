#
# Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# No GICv4 extension
GIC_ENABLE_V4_EXTN	:=	0

GICV2_SOURCES	+=	drivers/arm/gic/v2/gicv2_main.c		\
			drivers/arm/gic/v2/gicv2_helpers.c	\
			drivers/arm/gic/v2/gicdv2_helpers.c

# Set GICv4 extension
$(eval $(call assert_boolean,GIC_ENABLE_V4_EXTN))
$(eval $(call add_define,GIC_ENABLE_V4_EXTN))
