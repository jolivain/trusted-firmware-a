/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mtk_init/mtk_init.h>
SECTIONS
{
    .mtk_plat_init . : {
		INIT_CALL_TABLE(EXPAND_AS_LINK_SECTION);
		__MTK_PLAT_INITCALL_END__ = .;
	}
}
INSERT BEFORE .rodata;

