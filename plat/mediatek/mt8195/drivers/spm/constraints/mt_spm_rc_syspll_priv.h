/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
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
	 SPM_FLAG_DISABLE_DRAMC_MCU_SRAM_SLEEP |\
	 SPM_FLAG_USE_SRCCLKENO2)

static struct mt_spm_cond_tables cond_syspll = {
	.name = "syspll",
	.table_cg = {
		0xFFFFD008,	/* MTCMOS1 */
		0x20844802,	/* INFRA0  */
		0x27AF8000,	/* INFRA1  */
		0x86040640,	/* INFRA2  */
		0x30038020,	/* INFRA3  */
		0x80000000,	/* INFRA4  */
		0x00080A8B,	/* PERI0   */
		0x00004000,	/* VPPSYS0_0  */
		0x08803000,	/* VPPSYS0_1  */
		0x00000000,	/* VPPSYS0_2  */
		0x80005555,	/* VPPSYS1_0  */
		0x00009008,	/* VPPSYS1_1  */
		0x60060000,	/* VDOSYS0_0  */
		0x00000000,	/* VDOSYS0_1  */
		0x201E01F8,	/* VDOSYS1_0  */
		0x00800000,	/* VDOSYS1_1  */
		0x00000000,	/* VDOSYS1_2  */
		0x00000080,	/* I2C */
	},
	.table_pll = 0U,
};

#endif /* MT_SPM_RC_SYSPLL_PRIV_H */
