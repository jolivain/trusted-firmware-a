#
# Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Add Property Access Layer platform specific files
FCONF_SOURCES		:=	drivers/fconf/fconf.c

BL1_SOURCES		+=	${FCONF_SOURCES}
BL2_SOURCES		+=	${FCONF_SOURCES}

