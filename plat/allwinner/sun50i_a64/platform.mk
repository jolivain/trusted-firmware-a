#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# The differences between the platform are covered by the include files.
include plat/allwinner/common/allwinner-common.mk

BL31_SOURCES		+=	drivers/allwinner/sunxi_rsb.c
