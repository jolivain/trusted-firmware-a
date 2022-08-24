/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RC_SYSPLL_PRIV_H
#define MT_SPM_RC_SYSPLL_PRIV_H
#include <mt_spm_reg.h>

#define CONSTRAINT_SYSPLL_PCM_FLAG		\
	(SPM_FLAG_DISABLE_INFRA_PDN |		\
	 SPM_FLAG_DISABLE_VCORE_DVS |		\
	 SPM_FLAG_DISABLE_VCORE_DFS |		\
	 SPM_FLAG_SRAM_SLEEP_CTRL |		\
	 SPM_FLAG_KEEP_CSYSPWRACK_HIGH |	\
	 SPM_FLAG_ENABLE_6315_CTRL |		\
	 SPM_FLAG_USE_SRCCLKENO2)

static struct mt_spm_cond_tables cond_syspll = {
	.name = "syspll",
	.table_cg = {
		0x078BF1FC,	/* MTCMOS1 */
		0x080D8856,	/* INFRA0  */
		0x03AF9A00,	/* INFRA1  */
		0x86000640,	/* INFRA2  */
		0xC800C000,	/* INFRA3  */
		0x00000000,     /* INFRA4  */
		0x0000007C,     /* INFRA5  */
		0x280E0800,	/* MMSYS0  */
		0x00000001,     /* MMSYS1  */
		0x00000000,	/* MMSYS2  */
	},
	.table_pll = 0U,
};

#endif /* MT_SPM_RC_SYSPLL_PRIV_H */
