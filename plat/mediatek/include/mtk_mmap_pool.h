/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_MMAP_POOL_H
#define MTK_MMAP_POOL_H

#include <lib/xlat_tables/xlat_tables_compat.h>

struct mtk_mmap_descriptor {
	const char *mmap_name;
	const mmap_region_t *mmap_ptr;
	const uint32_t mmap_size;
};
#define MTK_MMAP_SECTION \
	__attribute__((used)) \
	__aligned(sizeof(void *)) \
	__section(".mtk_mmap_lists")

#define DECLARE_MTK_MMAP_REGIONS(_mmap_array) \
	static const struct mtk_mmap_descriptor _mtk_mmap_descriptor_##_mmap_array \
	__attribute__((used)) \
	__aligned(sizeof(void *)) \
	__section(".mtk_mmap_pool") \
	= { \
		.mmap_name = #_mmap_array, \
		.mmap_ptr = _mmap_array, \
		.mmap_size = ARRAY_SIZE(_mmap_array) \
	};
void mtk_xlat_init(const mmap_region_t *bl_regions);
#endif /* MTK_MMAP_POOL_H */
