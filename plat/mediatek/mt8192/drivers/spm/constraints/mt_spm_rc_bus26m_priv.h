/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RC_BUS26M_PRIV_H
#define MT_SPM_RC_BUS26M_PRIV_H

#include <mt_spm.h>
#include <mt_spm_reg.h>

#define CONSTRAINT_BUS26M_PCM_FLAG		\
	(SPM_FLAG_DISABLE_INFRA_PDN |		\
	 SPM_FLAG_DISABLE_VCORE_DVS |		\
	 SPM_FLAG_DISABLE_VCORE_DFS |		\
	 SPM_FLAG_SRAM_SLEEP_CTRL |		\
	 SPM_FLAG_ENABLE_TIA_WORKAROUND |	\
	 SPM_FLAG_ENABLE_LVTS_WORKAROUND |	\
	 SPM_FLAG_KEEP_CSYSPWRACK_HIGH)

#define CONSTRAINT_BUS26M_PCM_FLAG1		\
	(SPM_FLAG1_DISABLE_MD26M_CK_OFF)

#define CONSTRAINT_BUS26M_SUSPEND_EX_PAND	\
	(MT_SPM_EX_OP_SET_WDT |			\
	 MT_SPM_EX_OP_HW_S1_DETECT)

static struct mt_spm_cond_tables cond_bus26m = {
	.name = "bus26m",
	.table_cg = {
		0x07CBF1FC,	/* MTCMOS1 */
		0x0A0D8856,	/* INFRA0  */
		0x03AF9A00,	/* INFRA1  */
		0x86000650,	/* INFRA2  */
		0xC800C000,	/* INFRA3  */
		0x00000000,     /* INFRA4  */
		0x4000007C,     /* INFRA5  */
		0x280E0800,	/* MMSYS0  */
		0x00000001,     /* MMSYS1  */
		0x00000000,	/* MMSYS2  */
	},
	.table_pll = (PLL_BIT_UNIVPLL | PLL_BIT_MFGPLL |
		      PLL_BIT_MSDCPLL | PLL_BIT_TVDPLL |
		      PLL_BIT_MMPLL),
};

#endif /* MT_SPM_RC_BUS26M_PRIV_H */
