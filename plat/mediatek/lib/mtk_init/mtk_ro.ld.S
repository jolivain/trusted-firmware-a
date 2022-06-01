/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

SECTIONS
{
	.mtk_plat_ro . : {
		*(mtk_plat_ro)
	}
}
INSERT BEFORE .rodata;

