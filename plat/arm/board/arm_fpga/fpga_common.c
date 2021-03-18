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

/* Needs to be big enough to contain the fpga cmdline. */
#define FPGA_CMD_LINE_SIZE	PAGE_SIZE
/* The cmdline must be loaded in as device memory. */
#define FPGA_CMD_LINE		MAP_REGION_FLAT(			\
					FPGA_PRELOADED_CMD_LINE,	\
					FPGA_CMD_LINE_SIZE,		\
					MT_DEVICE | MT_RO | MT_SECURE)

#define FPGA_UART_BASE		U(0x7ff80000)
#define FPGA_UART_SIZE		PAGE_SIZE

#define FPGA_UART		MAP_REGION_FLAT(			\
					FPGA_UART_BASE,			\
					FPGA_UART_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define FPGA_GICD_BASE		U(0x30000000)
#define FPGA_GICD_SIZE		U(0x10000)

#define FPGA_GICD		MAP_REGION_FLAT(			\
					FPGA_GICD_BASE,			\
					FPGA_GICD_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define FPGA_GICR_BASE		U(0x30040000)
#define FPGA_GICR_SIZE		(U(0x20000) * PLATFORM_CORE_COUNT)

#define FPGA_GICR		MAP_REGION_FLAT(			\
					FPGA_GICR_BASE,			\
					FPGA_GICR_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	FPGA_UART,
	FPGA_GICD,
	FPGA_GICR,
	FPGA_PRELOADED_DTB,
	FPGA_CMD_LINE,
	{0}
};
