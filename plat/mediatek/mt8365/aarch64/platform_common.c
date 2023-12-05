/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <platform_def.h>

/* Platform memory map regions */
#define MAP_BL32		MAP_REGION_FLAT(BL32_BASE,		       \
						BL32_SIZE,		       \
						MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_DEVICE0		MAP_REGION_FLAT(MTK_DEV_RNG0_BASE,	       \
						MTK_DEV_RNG0_SIZE,	       \
						MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE1		MAP_REGION_FLAT(MTK_DEV_RNG1_BASE,	       \
						MTK_DEV_RNG1_SIZE,	       \
						MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE2		MAP_REGION_FLAT(MTK_DEV_RNG2_BASE,	       \
						MTK_DEV_RNG2_SIZE,	       \
						MT_DEVICE | MT_RW | MT_SECURE)

static const mmap_region_t plat_mmap[] = {
	MAP_BL32,
	MAP_DEVICE0,
	MAP_DEVICE1,
	MAP_DEVICE2,
	{ 0 }
};

void plat_configure_mmu_el3(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long ro_start,
			    unsigned long ro_limit)
{
	mmap_add_region(total_base, total_base, total_size, MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(ro_start, ro_start, ro_limit - ro_start, MT_CODE | MT_SECURE);

	mmap_add(plat_mmap);
	init_xlat_tables();
	enable_mmu_el3(0);
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}
