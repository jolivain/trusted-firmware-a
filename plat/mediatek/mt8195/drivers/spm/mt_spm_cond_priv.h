/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
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
#define MT_LP_TZ_SPM_REG(ofs)		(SPM_BASE + ofs)
#define MT_LP_TZ_TOPCK_REG(ofs)		(TOPCKGEN_BASE + ofs)
#define MT_LP_TZ_APMIXEDSYS(ofs)	(APMIXEDSYS + ofs)
#define MT_LP_TZ_VPPSYS0_REG(ofs)	(VPPSYS0_BASE + ofs)
#define MT_LP_TZ_VPPSYS1_REG(ofs)	(VPPSYS1_BASE + ofs)
#define MT_LP_TZ_VDOSYS0_REG(ofs)	(VDOSYS0_BASE + ofs)
#define MT_LP_TZ_VDOSYS1_REG(ofs)	(VDOSYS1_BASE + ofs)
#define MT_LP_TZ_PERI_AO_REG(ofs)	(PERICFG_AO_BASE + ofs)

#define SPM_PWR_STATUS			MT_LP_TZ_SPM_REG(0x016C)
#define SPM_PWR_STATUS_2ND		MT_LP_TZ_SPM_REG(0x0170)
#define INFRA_SW_CG0			MT_LP_TZ_INFRA_REG(0x0094)
#define INFRA_SW_CG1			MT_LP_TZ_INFRA_REG(0x0090)
#define INFRA_SW_CG2			MT_LP_TZ_INFRA_REG(0x00AC)
#define INFRA_SW_CG3			MT_LP_TZ_INFRA_REG(0x00C8)
#define INFRA_SW_CG4			MT_LP_TZ_INFRA_REG(0x00E8)
#define TOP_SW_I2C_CG			MT_LP_TZ_TOPCK_REG(0x00BC)
#define PERI_SW_CG0			MT_LP_TZ_PERI_AO_REG(0x0018)
#define VPPSYS0_SW_CG0			MT_LP_TZ_VPPSYS0_REG(0x0020)
#define VPPSYS0_SW_CG1			MT_LP_TZ_VPPSYS0_REG(0x002C)
#define VPPSYS0_SW_CG2			MT_LP_TZ_VPPSYS0_REG(0x0038)
#define VPPSYS1_SW_CG0			MT_LP_TZ_VPPSYS1_REG(0x0100)
#define VPPSYS1_SW_CG1			MT_LP_TZ_VPPSYS1_REG(0x0110)
#define VDOSYS0_SW_CG0			MT_LP_TZ_VDOSYS0_REG(0x0100)
#define VDOSYS0_SW_CG1			MT_LP_TZ_VDOSYS0_REG(0x0110)
#define VDOSYS1_SW_CG0			MT_LP_TZ_VDOSYS1_REG(0x0100)
#define VDOSYS1_SW_CG1			MT_LP_TZ_VDOSYS1_REG(0x0120)
#define VDOSYS1_SW_CG2			MT_LP_TZ_VDOSYS1_REG(0x0130)

/***********************************************************
 * Check clkmux registers
 ***********************************************************/
#define CLK_CFG(id)	MT_LP_TZ_TOPCK_REG(0x98 + id * 0x10)
#define PDN_CHECK	BIT(7)
#define CLK_CHECK	BIT(31)

enum {
	CLKMUX_DISP = 0,
	NF_CLKMUX,
};

static struct idle_cond_info idle_cg_info[PLAT_SPM_COND_MAX] = {
	IDLE_CG(0xffffffff, SPM_PWR_STATUS, false, 0U),
	IDLE_CG(0xffffffff, INFRA_SW_CG0, true, 0U),
	IDLE_CG(0xffffffff, INFRA_SW_CG1, true, 0U),
	IDLE_CG(0xffffffff, INFRA_SW_CG2, true, 0U),
	IDLE_CG(0xffffffff, INFRA_SW_CG3, true, 0U),
	IDLE_CG(0xffffffff, INFRA_SW_CG4, true, 0U),
	IDLE_CG(0xffffffff, PERI_SW_CG0, true, 0U),
	IDLE_CG(0x00000800, VPPSYS0_SW_CG0, true, (CLK_CHECK|CLKMUX_DISP)),
	IDLE_CG(0x00000800, VPPSYS0_SW_CG1, true, (CLK_CHECK|CLKMUX_DISP)),
	IDLE_CG(0x00000800, VPPSYS0_SW_CG2, true, (CLK_CHECK|CLKMUX_DISP)),
	IDLE_CG(0x00001000, VPPSYS1_SW_CG0, true, (CLK_CHECK|CLKMUX_DISP)),
	IDLE_CG(0x00001000, VPPSYS1_SW_CG1, true, (CLK_CHECK|CLKMUX_DISP)),
	IDLE_CG(0x00002000, VDOSYS0_SW_CG0, true, (CLK_CHECK|CLKMUX_DISP)),
	IDLE_CG(0x00002000, VDOSYS0_SW_CG1, true, (CLK_CHECK|CLKMUX_DISP)),
	IDLE_CG(0x00004000, VDOSYS1_SW_CG0, true, (CLK_CHECK|CLKMUX_DISP)),
	IDLE_CG(0x00004000, VDOSYS1_SW_CG1, true, (CLK_CHECK|CLKMUX_DISP)),
	IDLE_CG(0x00004000, VDOSYS1_SW_CG2, true, (CLK_CHECK|CLKMUX_DISP)),
	IDLE_CG(0x00000080, TOP_SW_I2C_CG, true, (CLK_CHECK|CLKMUX_DISP)),
};

/***********************************************************
 * Check pll idle condition
 ***********************************************************/
#define PLL_MFGPLL	MT_LP_TZ_APMIXEDSYS(0x340)
#define PLL_MMPLL	MT_LP_TZ_APMIXEDSYS(0x0E0)
#define PLL_UNIVPLL	MT_LP_TZ_APMIXEDSYS(0x1F0)
#define PLL_MSDCPLL	MT_LP_TZ_APMIXEDSYS(0x710)
#define PLL_TVDPLL	MT_LP_TZ_APMIXEDSYS(0x380)

#define PLL_MFGPLL_EN_BIT	(9U)
#define PLL_MMPLL_EN_BIT	(9U)
#define PLL_UNIVPLL_EN_BIT	(9U)
#define PLL_MSDCPLL_EN_BIT	(9U)
#define PLL_TVDPLL_EN_BIT	(9U)
