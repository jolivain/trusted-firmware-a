/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

SECTIONS
{
	.mtk_smc_pool . : {
		__MTK_RODATA_START__ = .;
		__MTK_SMC_POOL_START__ = .;
		KEEP(*(.mtk_smc_descriptor_pool));
		__MTK_SMC_POOL_END_UNALIGNED__ = .;
		. = ALIGN(64);
	}
}
INSERT BEFORE .rodata;

