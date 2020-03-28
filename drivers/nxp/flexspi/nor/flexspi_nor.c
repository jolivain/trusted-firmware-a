/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <plat_common.h>
#include <platform_def.h>

int flexspi_init(void)
{
	return 0;
}

int flexspi_nor_io_setup(void)
{

	flexspi_init();
	/* Adding NOR Memory Map in XLAT Table */
	mmap_add_region(NXP_FLEXSPI_FLASH_ADDR, NXP_FLEXSPI_FLASH_ADDR,
			NXP_FLEXSPI_FLASH_SIZE, MT_MEMORY | MT_RW);

	return plat_io_memmap_setup(NXP_FLEXSPI_FLASH_ADDR + PLAT_FIP_OFFSET);
}
