/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/xlat_tables/xlat_tables_compat.h>

#include "fpga_private.h"

#define FPGA_PRELOADED_DTB						\
				MAP_REGION_FLAT(			\
					FPGA_PRELOADED_DTB_BASE,	\
					FPGA_MAX_DTB_SIZE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

#define FPGA_CMD_LINE_SIZE	PAGE_SIZE
/* The cmdline must be loaded in as device memory. */
#define FPGA_CMD_LINE		MAP_REGION_FLAT(			\
					FPGA_PRELOADED_CMD_LINE,	\
					FPGA_CMD_LINE_SIZE,		\
					MT_DEVICE | MT_RO | MT_SECURE)

const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	FPGA_PRELOADED_DTB,
	FPGA_CMD_LINE,
	{0}
};

