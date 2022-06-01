/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <stdint.h>
#include <mtk_mmap_pool.h>
#include <platform_def.h>

static const mmap_region_t plat_mmap[] MTK_MMAP_SECTION = {
	MAP_REGION_FLAT(EFUSE_BASE, EFUSE_REG_SIZE,
		MT_DEVICE | MT_RO | MT_SECURE),
	{0}
};
DECLARE_MTK_MMAP_REGIONS(plat_mmap);


