/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* C lib */
#include <string.h>

/* TF-A header */

/* MTK header */
#include <lib/boot_tag/boot_tag.h>

/* GZ header */
#include "gz_boot_tag.h"

/* GZ variables */
#define EL2_BOOT_DISABLE (1 << 0)
static uint32_t gz_configs = EL2_BOOT_DISABLE;
static uint64_t gz_exec_start_offset;
static uint64_t gz_reserved_mem_size;

/* gz info */
int init_gz_info(void *tag_entry)
{
	struct boot_tag_gz_info *gz_info = (struct boot_tag_gz_info *)tag_entry;

	gz_configs = gz_info->gz_configs;

	return 0;
}

uint32_t is_el2_enabled(void)
{
	if (gz_configs & EL2_BOOT_DISABLE)
		return 0; /* el2 is disabled */

	return 1; /* el2 is enabled */
}

/* gz platform */
int init_gz_plat(void *tag_entry)
{
	struct boot_tag_gz_platform *gz_plat = (struct boot_tag_gz_platform *)tag_entry;

	/* do copy here to prevent un-aligned access for 64-bit memory */
	memcpy(&gz_exec_start_offset, &gz_plat->exec_start_offset, sizeof(uint64_t));

	/* 32-bit, access directly */
	gz_reserved_mem_size = gz_plat->reserve_mem_size & 0xFFFFFFFFUL;

	return 0;
}

uint64_t get_el2_exec_start_offset(void)
{
	return gz_exec_start_offset;
}

uint64_t get_el2_reserved_mem_size(void)
{
	return gz_reserved_mem_size;
}
