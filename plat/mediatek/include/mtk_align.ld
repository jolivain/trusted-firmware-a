/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

SECTIONS
{
    .mtk_padding . : {
		FILL(0xDEADBEEF);
		. = ALIGN(4096);
		__MTK_RODATA_END__ = .;
	}
}
INSERT BEFORE .rodata;
