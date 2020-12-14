/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <mt_lp_rm.h>
#include <mt_spm.h>
#include <mt_spm_cond.h>
#include <mt_spm_constraint.h>
#include <mt_spm_conservation.h>
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_notifier.h>
#include <mt_spm_rc_api.h>
#include <mt_spm_rc_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <mt_spm_suspend.h>
#include <plat_pm.h>
#include <plat_mtk_lpm.h>

#define CONSTRAINT_SYSPLL_ALLOW			\
	(MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF |	\
	 MT_RM_CONSTRAINT_ALLOW_DRAM_S0 |	\
	 MT_RM_CONSTRAINT_ALLOW_DRAM_S1 | 	\
	 MT_RM_CONSTRAINT_ALLOW_VCORE_LP)

#define CONSTRAINT_SYSPLL_PCM_FLAG		\
	(SPM_FLAG_DISABLE_INFRA_PDN |		\
	 SPM_FLAG_DISABLE_VCORE_DVS |		\
	 SPM_FLAG_DISABLE_VCORE_DFS |		\
	 SPM_FLAG_SRAM_SLEEP_CTRL |		\
	 SPM_FLAG_KEEP_CSYSPWRACK_HIGH |	\
	 SPM_FLAG_ENABLE_6315_CTRL |		\
	 SPM_FLAG_USE_SRCCLKENO2)

#define CONSTRAINT_SYSPLL_PCM_FLAG1 (0)

/*
 * If sspm sram won't enter sleep voltage
 * then vcore couldn't enter low power mode
 */
#define CONSTRAINT_SYSPLL_RESOURCE_REQ	(MT_SPM_26M)

static unsigned short ext_status_syspll;

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
	.table_pll = 0,
};

static struct mt_spm_cond_tables cond_syspll_res = {
	.table_cg = {0},
	.table_pll = 0,
};

static struct constraint_status status = {
	.id = MT_RM_CONSTRAINT_ID_SYSPLL,
	.is_valid = (MT_SPM_RC_VALID_SW |
		     MT_SPM_RC_VALID_COND_CHECK |
		     MT_SPM_RC_VALID_COND_LATCH |
		     MT_SPM_RC_VALID_XSOC_BBLPM),
	.is_cond_block = 0,
	.enter_cnt = 0,
	.cond_res = &cond_syspll_res,
	.residency = 0,
};

int spm_syspll_conduct(int state_id, struct spm_lp_scen *spm_lp,
		       unsigned int *resource_req)
{
	unsigned int res_req = CONSTRAINT_SYSPLL_RESOURCE_REQ;

	spm_lp->pwrctrl->pcm_flags = (uint32_t)CONSTRAINT_SYSPLL_PCM_FLAG;
	spm_lp->pwrctrl->pcm_flags1 = (uint32_t)CONSTRAINT_SYSPLL_PCM_FLAG1;
	*resource_req |= res_req;

	return 0;
}

int spm_is_valid_rc_syspll(int cpu, int state_id)
{
	return (!(status.is_cond_block &&
		(status.is_valid & MT_SPM_RC_VALID_COND_CHECK)) &&
		IS_MT_RM_RC_READY(status.is_valid));
}

int spm_update_rc_syspll(int state_id, int type, const void *val)
{
	int res = MT_RM_STATUS_OK;

	if (type == PLAT_RC_UPDATE_CONDITION) {
		const struct mt_spm_cond_tables * const tlb =
			(const struct mt_spm_cond_tables * const)val;
		const struct mt_spm_cond_tables *tlb_check =
			(const struct mt_spm_cond_tables *)&cond_syspll;

		if (!tlb) {
			return MT_RM_STATUS_BAD;
		}

		status.is_cond_block =
			mt_spm_cond_check(state_id, tlb, tlb_check,
					  (status.is_valid &
					   MT_SPM_RC_VALID_COND_LATCH) ?
					  &cond_syspll_res : NULL);
	} else if (type == PLAT_RC_STATUS) {
		const struct rc_common_state *st;
		struct constraint_status *src, *dest;

		st = (const struct rc_common_state *)val;
		if (!st) {
			return res;
		}

		src = (struct constraint_status *)st->value;
		dest = (struct constraint_status *)&status;

		if (st->type == CONSTRAINT_UPDATE_COND_CHECK) {
			struct mt_spm_cond_tables * const tlb =	&cond_syspll;

			spm_rc_condition_modifier(st->id, st->act, st->value,
						  MT_RM_CONSTRAINT_ID_SYSPLL,
						  tlb);
		} else if ((st->type == CONSTRAINT_UPDATE_VALID) ||
			   (st->type == CONSTRAINT_RESIDNECY)) {
			spm_rc_constraint_status_set(st->id, st->type, st->act,
						     MT_RM_CONSTRAINT_ID_SYSPLL,
						     src, dest);
		} else {
			INFO("[%s:%d] - Unknown type: 0x%x\n",
			     __func__, __LINE__, st->type);
		}
	}

	return res;
}

unsigned int spm_allow_rc_syspll(int state_id)
{
	return CONSTRAINT_SYSPLL_ALLOW;
}

int spm_run_rc_syspll(int cpu, int state_id)
{
	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;
	unsigned int allows = CONSTRAINT_SYSPLL_ALLOW;

	ext_status_syspll = status.is_valid;

	if (IS_MT_SPM_RC_BBLPM_MODE(ext_status_syspll)) {
#ifdef MT_SPM_USING_SRCLKEN_RC
		ext_op |= MT_SPM_EX_OP_SRCLKEN_RC_BBLPM;
#else
		allows |= MT_RM_CONSTRAINT_ALLOW_BBLPM;
#endif
	}

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_ENTER, allows |
			       (IS_PLAT_SUSPEND_ID(state_id) ?
				MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND : 0));
#else
	(void)allows;
#endif

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_enter(state_id,
				     (MT_SPM_EX_OP_SET_WDT |
				      MT_SPM_EX_OP_HW_S1_DETECT |
				      MT_SPM_EX_OP_SET_SUSPEND_MODE),
				     CONSTRAINT_SYSPLL_RESOURCE_REQ);
	} else {
		mt_spm_idle_generic_enter(state_id, ext_op, spm_syspll_conduct);
	}

	return 0;
}

int spm_reset_rc_syspll(int cpu, int state_id)
{
	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;
	unsigned int allows = CONSTRAINT_SYSPLL_ALLOW;

	if (IS_MT_SPM_RC_BBLPM_MODE(ext_status_syspll)) {
#ifdef MT_SPM_USING_SRCLKEN_RC
		ext_op |= MT_SPM_EX_OP_SRCLKEN_RC_BBLPM;
#else
		allows |= MT_RM_CONSTRAINT_ALLOW_BBLPM;
#endif
	}

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_LEAVE, allows);
#else
	(void)allows;
#endif

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_resume(state_id,
				      (MT_SPM_EX_OP_SET_SUSPEND_MODE |
				       MT_SPM_EX_OP_SET_WDT |
				       MT_SPM_EX_OP_HW_S1_DETECT),
				      NULL);
	} else {
		struct wake_status *waken = NULL;

		if (spm_unlikely(status.is_valid &
				 MT_SPM_RC_VALID_TRACE_EVENT)) {
			ext_op |= MT_SPM_EX_OP_TRACE_LP;
		}

		mt_spm_idle_generic_resume(state_id, ext_op, &waken);
		status.enter_cnt++;

		if (spm_unlikely(status.is_valid & MT_SPM_RC_VALID_RESIDNECY)) {
			status.residency +=
				waken ? waken->tr.comm.timer_out : 0;
		}
	}

	return 0;
}

int spm_get_status_rc_syspll(int type, void *priv)
{
	int ret = MT_RM_STATUS_OK;

	if (type == PLAT_RC_STATUS) {
		int res = 0;
		struct rc_common_state *st = (struct rc_common_state *)priv;
		struct constraint_status *src, *dest;

		if (!st) {
			return MT_RM_STATUS_BAD;
		}

		src = (struct constraint_status *)&status;
		dest = (struct constraint_status *)st->value;
		res = spm_rc_constraint_status_get(st->id, st->type, st->act,
						   MT_RM_CONSTRAINT_ID_SYSPLL,
						   src, dest);
		if (!res && (st->id != MT_RM_CONSTRAINT_ID_ALL)) {
			ret = MT_RM_STATUS_STOP;
		}
	}

	return ret;
}
