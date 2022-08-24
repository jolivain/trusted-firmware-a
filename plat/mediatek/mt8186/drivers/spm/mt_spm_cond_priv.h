/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mt_spm_cond.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <plat_mtk_lpm.h>
#include <plat_pm.h>
#include <platform_def.h>

#define MT_LP_TZ_INFRA_REG(ofs)		(INFRACFG_AO_BASE + ofs)
#define MT_LP_TZ_MM_REG(ofs)		(MMSYS_BASE + ofs)
#define MT_LP_TZ_MDP_REG(ofs)		(MDPSYS_BASE + ofs)
#define MT_LP_TZ_SPM_REG(ofs)		(SPM_BASE + ofs)
#define MT_LP_TZ_TOPCK_REG(ofs)		(TOPCKGEN_BASE + ofs)
#define MT_LP_TZ_APMIXEDSYS(ofs)	(APMIXEDSYS + ofs)

#define SPM_PWR_STATUS			MT_LP_TZ_SPM_REG(0x016C)
#define SPM_PWR_STATUS_2ND		MT_LP_TZ_SPM_REG(0x0170)
#define INFRA_SW_CG0			MT_LP_TZ_INFRA_REG(0x0090)
#define INFRA_SW_CG1			MT_LP_TZ_INFRA_REG(0x0094)
#define INFRA_SW_CG2			MT_LP_TZ_INFRA_REG(0x00AC)
#define INFRA_SW_CG3			MT_LP_TZ_INFRA_REG(0x00C8)
#define INFRA_SW_CG4			MT_LP_TZ_INFRA_REG(0x00E8)
#define INFRA_SW_CG5			MT_LP_TZ_INFRA_REG(0x00D8)
#define MMSYS_CG_CON0			MT_LP_TZ_MM_REG(0x100)
#define MMSYS_CG_CON1			MT_LP_TZ_MM_REG(0x110)
#define MMSYS_CG_CON2			MT_LP_TZ_MM_REG(0x1A0)
#define MMSYS_CG_CON3			MT_LP_TZ_MDP_REG(0x100)

/* Check clkmux registers */
#define CLK_CFG(id)    MT_LP_TZ_TOPCK_REG(0xe0 + id * 0x10)
#define PDN_CHECK	BIT(7)
#define CLK_CHECK	BIT(31)

enum {
	CLKMUX_DISP  = 0,
	CLKMUX_MDP   = 1,
	CLKMUX_IMG1  = 2,
	CLKMUX_IMG2  = 3,
	NF_CLKMUX    = 4,
};

static struct idle_cond_info idle_cg_info[PLAT_SPM_COND_MAX] = {
	IDLE_CG(0xffffffff, SPM_PWR_STATUS, false, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG0, true, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG1, true, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG2, true, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG3, true, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG4, true, 0U),
	IDLE_CG(0x00000200, INFRA_SW_CG5, true, 0U),
	IDLE_CG(0x00200000, MMSYS_CG_CON0, true, (CLK_CHECK | CLKMUX_DISP)),
	IDLE_CG(0x00200000, MMSYS_CG_CON1, true, (CLK_CHECK | CLKMUX_DISP)),
	IDLE_CG(0x00200000, MMSYS_CG_CON2, true, (CLK_CHECK | CLKMUX_DISP)),
	IDLE_CG(0x00200000, MMSYS_CG_CON3, true, (CLK_CHECK | CLKMUX_MDP)),
};

/* Check pll idle condition */
#define PLL_MFGPLL	MT_LP_TZ_APMIXEDSYS(0x314)
#define PLL_MMPLL	MT_LP_TZ_APMIXEDSYS(0x254)
#define PLL_UNIVPLL	MT_LP_TZ_APMIXEDSYS(0x324)
#define PLL_MSDCPLL	MT_LP_TZ_APMIXEDSYS(0x38c)
#define PLL_TVDPLL	MT_LP_TZ_APMIXEDSYS(0x264)

#define PLL_MFGPLL_EN_BIT	(0U)
#define PLL_MMPLL_EN_BIT	(0U)
#define PLL_UNIVPLL_EN_BIT	(0U)
#define PLL_MSDCPLL_EN_BIT	(0U)
#define PLL_TVDPLL_EN_BIT	(0U)
