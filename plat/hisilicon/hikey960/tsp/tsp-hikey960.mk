#
# Copyright (c) 2021-2022, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# TSP source files specific to Hikey960 platform
BL32_SOURCES		+=	plat/common/aarch64/platform_mp_stack.S			\
				plat/hisilicon/hikey960/tsp/hikey960_tsp_setup.c	\
				plat/hisilicon/hikey960/aarch64/hikey960_helpers.S	\
				${HIKEY960_GIC_SOURCES}