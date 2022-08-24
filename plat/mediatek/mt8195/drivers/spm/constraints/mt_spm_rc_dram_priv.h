/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mt_spm_cond.h>
#include <mt_spm_reg.h>

#define CONSTRAINT_DRAM_PCM_FLAG		\
	(SPM_FLAG_DISABLE_INFRA_PDN |		\
	 SPM_FLAG_DISABLE_VCORE_DVS |		\
	 SPM_FLAG_DISABLE_VCORE_DFS |		\
	 SPM_FLAG_SRAM_SLEEP_CTRL |		\
	 SPM_FLAG_KEEP_CSYSPWRACK_HIGH |	\
	 SPM_FLAG_DISABLE_DRAMC_MCU_SRAM_SLEEP)

#define CONSTRAINT_DRAM_RESOURCE_EXT_OPAND	\
	(MT_SPM_EX_OP_SET_WDT |			\
	 MT_SPM_EX_OP_SET_SUSPEND_MODE |	\
	 MT_SPM_EX_OP_HW_S1_DETECT)

static struct mt_spm_cond_tables cond_dram = {
	.name = "dram",
	.table_cg = {
		0xFFFDD008,	/* MTCMOS1 */
		0x20040802,	/* INFRA0  */
		0x27AF8000,	/* INFRA1  */
		0x86040640,	/* INFRA2  */
		0x00000000,	/* INFRA3  */
		0x80000000,	/* INFRA4  */
		0x00000000,	/* PERI0   */
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
