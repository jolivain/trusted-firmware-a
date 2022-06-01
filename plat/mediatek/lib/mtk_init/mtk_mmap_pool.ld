/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

SECTIONS
{
	.mtk_mmap_pointer_pool . : {
		__MTK_MMAP_POINTER_POOL_START__ = .;
		KEEP(*(.mtk_mmap_pool));
		__MTK_MMAP_POINTER_POOL_END_UNALIGNED__ = .;
		. = ALIGN(64);
	}
	.mtk_mmap_pool . : {
		__MTK_MMAP_POOL_START__ = .;
		KEEP(*(.mtk_mmap_lists));
		__MTK_MMAP_POOL_END_UNALIGNED__ = .;
		. = ALIGN(64);
	}
}
INSERT BEFORE .rodata;
