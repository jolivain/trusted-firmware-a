/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mt_spm_cond.h>
#include <mt_spm_cond_priv.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <plat_mtk_lpm.h>
#include <plat_pm.h>
#include <platform_def.h>

static bool is_clkmux_pdn(unsigned int clkmux_id)
{
	unsigned int reg, val, idx;
	bool ret = false;

	if (clkmux_id & CLK_CHECK) {
		clkmux_id = (clkmux_id & ~CLK_CHECK);
		reg = clkmux_id / 4U;
		val = mmio_read_32(CLK_CFG(reg));
		idx = clkmux_id % 4U;
		ret = (((val >> (idx * 8U)) & PDN_CHECK) != 0U);
	}

	return ret;
}

static struct mt_spm_cond_tables spm_cond_t;

unsigned int mt_spm_cond_check(int state_id,
			       const struct mt_spm_cond_tables *src,
			       const struct mt_spm_cond_tables *dest,
			       struct mt_spm_cond_tables *res)
{
	unsigned int blocked = 0U;
	unsigned int i;
	bool is_system_suspend = IS_PLAT_SUSPEND_ID(state_id);

	if ((src == NULL) || (dest == NULL)) {
		blocked = SPM_COND_CHECK_FAIL;
	} else {
		for (i = 0U; i < PLAT_SPM_COND_MAX; i++) {
			if (res != NULL) {
				res->table_cg[i] = (src->table_cg[i] & dest->table_cg[i]);
				if (is_system_suspend && ((res->table_cg[i]) != 0U)) {
					INFO("suspend: %s block[%u](0x%lx) = 0x%08x\n",
					     dest->name, i, idle_cg_info[i].addr,
					     res->table_cg[i]);
				}

				if ((res->table_cg[i]) != 0U) {
					blocked |= BIT(i);
				}
			} else if ((src->table_cg[i] & dest->table_cg[i]) != 0U) {
				blocked |= BIT(i);
				break;
			}
		}

		if (res != NULL) {
			res->table_pll = (src->table_pll & dest->table_pll);

			if (res->table_pll != 0U) {
				blocked |= (res->table_pll << SPM_COND_BLOCKED_PLL_IDX) |
					    SPM_COND_CHECK_BLOCKED_PLL;
			}
		} else if ((src->table_pll & dest->table_pll) != 0U) {
			blocked |= SPM_COND_CHECK_BLOCKED_PLL;
		}

		if (is_system_suspend && ((blocked) != 0U)) {
			INFO("suspend: %s total blocked = 0x%08x\n", dest->name, blocked);
		}
	}

	return blocked;
}

#define IS_MT_SPM_PWR_OFF(mask)					\
	(((mmio_read_32(SPM_PWR_STATUS) & mask) == 0U) &&	\
	 ((mmio_read_32(SPM_PWR_STATUS_2ND) & mask) == 0U))

int mt_spm_cond_update(struct mt_resource_constraint **con,
		       int stateid, void *priv)
{
	int res;
	uint32_t i;
	struct mt_resource_constraint *const *rc;

	/* read all cg state */
	for (i = 0U; i < PLAT_SPM_COND_MAX; i++) {
		spm_cond_t.table_cg[i] = 0U;

		/* check mtcmos, if off set idle_value and clk to 0 disable */
		if (IS_MT_SPM_PWR_OFF(idle_cg_info[i].subsys_mask)) {
			continue;
		}

		/* check clkmux */
		if (is_clkmux_pdn(idle_cg_info[i].clkmux_id)) {
			continue;
		}

		spm_cond_t.table_cg[i] = idle_cg_info[i].bit_flip ?
					 ~mmio_read_32(idle_cg_info[i].addr) :
					 mmio_read_32(idle_cg_info[i].addr);
	}

	spm_cond_t.table_pll = 0U;
	if ((mmio_read_32(PLL_MFGPLL) & BIT(PLL_MFGPLL_EN_BIT)) != 0U) {
		spm_cond_t.table_pll |= PLL_BIT_MFGPLL;
	}

	if ((mmio_read_32(PLL_MMPLL) & BIT(PLL_MMPLL_EN_BIT)) != 0U) {
		spm_cond_t.table_pll |= PLL_BIT_MMPLL;
	}

	if ((mmio_read_32(PLL_UNIVPLL) & BIT(PLL_UNIVPLL_EN_BIT)) != 0U) {
		spm_cond_t.table_pll |= PLL_BIT_UNIVPLL;
	}

	if ((mmio_read_32(PLL_MSDCPLL) & BIT(PLL_MSDCPLL_EN_BIT)) != 0U) {
		spm_cond_t.table_pll |= PLL_BIT_MSDCPLL;
	}

	if ((mmio_read_32(PLL_TVDPLL) & BIT(PLL_TVDPLL_EN_BIT)) != 0U) {
		spm_cond_t.table_pll |= PLL_BIT_TVDPLL;
	}

	spm_cond_t.priv = priv;

	for (rc = con; *rc != NULL; rc++) {
		if (((*rc)->update) == NULL) {
			continue;
		}

		res = (*rc)->update(stateid, PLAT_RC_UPDATE_CONDITION,
				    (void const *)&spm_cond_t);
		if (res != MT_RM_STATUS_OK) {
			break;
		}
	}

	return 0;
}
