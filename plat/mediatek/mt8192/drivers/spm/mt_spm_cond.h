/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_SPM_CONDIT_H__
#define __MT_SPM_CONDIT_H__

#include <mt_lp_rm.h>

enum PLAT_SPM_COND {
	PLAT_SPM_COND_MTCMOS1 = 0,
	PLAT_SPM_COND_CG_INFRA_0,
	PLAT_SPM_COND_CG_INFRA_1,
	PLAT_SPM_COND_CG_INFRA_2,
	PLAT_SPM_COND_CG_INFRA_3,
	PLAT_SPM_COND_CG_INFRA_4,
	PLAT_SPM_COND_CG_INFRA_5,
	PLAT_SPM_COND_CG_MMSYS_0,
	PLAT_SPM_COND_CG_MMSYS_1,
	PLAT_SPM_COND_CG_MMSYS_2,
	PLAT_SPM_COND_MAX,
};

enum PLAT_SPM_COND_PLL {
	PLAT_SPM_COND_PLL_UNIVPLL = 0,
	PLAT_SPM_COND_PLL_MFGPLL,
	PLAT_SPM_COND_PLL_MSDCPLL,
	PLAT_SPM_COND_PLL_TVDPLL,
	PLAT_SPM_COND_PLL_MMPLL,
	PLAT_SPM_COND_PLL_MAX,
};

#define PLL_BIT_MFGPLL	(1 << PLAT_SPM_COND_PLL_MFGPLL)
#define PLL_BIT_MMPLL	(1 << PLAT_SPM_COND_PLL_MMPLL)
#define PLL_BIT_UNIVPLL	(1 << PLAT_SPM_COND_PLL_UNIVPLL)
#define PLL_BIT_MSDCPLL	(1 << PLAT_SPM_COND_PLL_MSDCPLL)
#define PLL_BIT_TVDPLL	(1 << PLAT_SPM_COND_PLL_TVDPLL)

/* Definition about SPM_COND_CHECK_BLOCKED
 * bit [00 ~ 15]: cg blocking index
 * bit [16 ~ 29]: pll blocking index
 * bit [30]     : pll blocking information
 * bit [31]	: idle condition check fail
 */
#define SPM_COND_BLOCKED_CG_IDX		0
#define SPM_COND_BLOCKED_PLL_IDX	16
#define SPM_COND_CHECK_BLOCKED_PLL	(1U << 30)
#define SPM_COND_CHECK_FAIL		(1U << 31)

struct mt_spm_cond_tables {
	char *name;
	unsigned int table_cg[PLAT_SPM_COND_MAX];
	unsigned int table_pll;
	void *priv;
};

enum MT_SPM_VCORE_LP_OP {
	MT_SPM_VCORE_LP_OP6V,
	MT_SPM_VCORE_LP_OP575V,
};

enum MT_SPM_VCORE_LP_MODE {
	MT_SPM_VCORE_LP_MODE_CHECK,
	MT_SPM_VCORE_LP_MODE_FORCE,
	MT_SPM_VCORE_LP_MODE_NORMAL,
	MT_SPM_VCORE_LP_MODE_BYPASS,
};

unsigned int mt_spm_cond_check(int state_id,
			       const struct mt_spm_cond_tables *src,
			       const struct mt_spm_cond_tables *dest,
			       struct mt_spm_cond_tables *res);

int mt_spm_cond_update(struct mt_resource_constraint **con,
		       int num, int stateid, void *priv);
#endif /* __MT_SPM_CONDIT_H__ */
