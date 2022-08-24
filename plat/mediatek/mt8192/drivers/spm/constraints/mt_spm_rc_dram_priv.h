/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mt_spm_cond.h>
#include <mt_spm_reg.h>

#define CONSTRAINT_DRAM_PCM_FLAG	\
	(SPM_FLAG_DISABLE_INFRA_PDN |	\
	 SPM_FLAG_DISABLE_VCORE_DVS |	\
	 SPM_FLAG_DISABLE_VCORE_DFS |	\
	 SPM_FLAG_SRAM_SLEEP_CTRL |	\
	 SPM_FLAG_KEEP_CSYSPWRACK_HIGH)

#define CONSTRAINT_DRAM_RESOURCE_EXT_OPAND \
	(MT_SPM_EX_OP_SET_WDT | MT_SPM_EX_OP_HW_S1_DETECT)

static struct mt_spm_cond_tables cond_dram = {
	.name = "dram",
	.table_cg = {
		0x078BF1FC,	/* MTCMOS1 */
		0x080D8856,	/* INFRA0  */
		0x03AF9A00,	/* INFRA1  */
		0x86000640,	/* INFRA2  */
		0xC800C000,	/* INFRA3  */
		0x00000000,     /* INFRA4  */
		0x00000000,     /* INFRA5  */
		0x200C0000,	/* MMSYS0  */
		0x00000000,     /* MMSYS1  */
		0x00000000,	/* MMSYS2  */
	},
	.table_pll = 0U,
};
