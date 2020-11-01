/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <mt_lp_rm.h>
#include <mt_spm_resource_req.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_cond.h>
#include <mt_spm_constraint.h>
#include <mt_spm_conservation.h>
#include <plat_pm.h>
#include <plat_mtk_lpm.h>

#include <mt_spm_rc_internal.h>
#include <mt_spm_suspend.h>
#include <mt_spm_idle.h>

#include <mt_spm_notifier.h>
#include <mt_spm_rc_api.h>
#include <mt_lp_api.h>

#define CONSTRAINT_DRAM_ALLOW (MT_RM_CONSTRAINT_ALLOW_DRAM_S0\
			      | MT_RM_CONSTRAINT_ALLOW_DRAM_S1\
			      | MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF)

#define CONSTRAINT_DRAM_PCM_FLAG (SPM_FLAG_DISABLE_INFRA_PDN\
				| SPM_FLAG_DISABLE_VCORE_DVS\
				| SPM_FLAG_DISABLE_VCORE_DFS\
				| SPM_FLAG_SRAM_SLEEP_CTRL\
				| SPM_FLAG_KEEP_CSYSPWRACK_HIGH)


#define CONSTRAINT_DRAM_PCM_FLAG1 (0)

#define CONSTRAINT_DRAM_RESOURCE_REQ (MT_SPM_SYSPLL\
				    | MT_SPM_INFRA\
				    | MT_SPM_26M)


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
	.table_pll = 0,
};

static struct mt_spm_cond_tables cond_dram_res = {
	.table_cg = {0},
	.table_pll = 0,
};

static struct constraint_status status = {
	.id = MT_RM_CONSTRAINT_ID_DRAM,
	.is_valid = (MT_SPM_RC_VALID_SW
		    | MT_SPM_RC_VALID_COND_CHECK
		    | MT_SPM_RC_VALID_COND_LATCH
		    | MT_SPM_RC_VALID_XSOC_BBLPM),
	.is_cond_block = 0,
	.enter_cnt = 0,
	.cond_res = &cond_dram_res,
	.residency = 0,
};

static unsigned short ext_status_dram;

int spm_dram_conduct(int state_id, struct spm_lp_scen *spm_lp,
					unsigned int *resource_req)
{
	unsigned int res_req = CONSTRAINT_DRAM_RESOURCE_REQ;

	if (!spm_lp || !resource_req)
		return -1;

	spm_lp->pwrctrl->pcm_flags =
				(uint32_t)CONSTRAINT_DRAM_PCM_FLAG;

	spm_lp->pwrctrl->pcm_flags1 =
				(uint32_t)CONSTRAINT_DRAM_PCM_FLAG1;

	*resource_req |= res_req;
	return 0;
}

int spm_is_valid_rc_dram(int cpu, int state_id)
{
	return (!(status.is_cond_block &&
		(status.is_valid & MT_SPM_RC_VALID_COND_CHECK))
		&& IS_MT_RM_RC_READY(status.is_valid));
}

int spm_update_rc_dram(int state_id, int type, const void *val)
{
	int res = MT_RM_STATUS_OK;

	if (type == PLAT_RC_UPDATE_CONDITION) {
		const struct mt_spm_cond_tables *tlb =
				(const struct mt_spm_cond_tables *)val;
		const struct mt_spm_cond_tables *tlb_check =
				(const struct mt_spm_cond_tables *)&cond_dram;
		if (!tlb)
			return MT_RM_STATUS_BAD;

		status.is_cond_block =
			mt_spm_cond_check(state_id, tlb, tlb_check,
					  (status.is_valid &
					   MT_SPM_RC_VALID_COND_LATCH) ?
					  &cond_dram_res : NULL);
	} else if (type == PLAT_RC_CLKBUF_STATUS) {
		unsigned int IsFlight = (val) ?
			!!(*((unsigned int *)val) == FLIGHT_MODE_ON) : 0;

		if (IsFlight)
			spm_rc_constraint_valid_set(MT_RM_CONSTRAINT_ID_DRAM,
				    MT_RM_CONSTRAINT_ID_DRAM,
				    MT_SPM_RC_VALID_FLIGHTMODE,
				    (struct constraint_status * const)&status);
		else
			spm_rc_constraint_valid_clr(MT_RM_CONSTRAINT_ID_DRAM,
				    MT_RM_CONSTRAINT_ID_DRAM,
				    MT_SPM_RC_VALID_FLIGHTMODE,
				    (struct constraint_status * const)&status);
	} else if (type == PLAT_RC_UFS_STATUS) {
		unsigned int IsUfsH8 = (val) ?
			!!(*((unsigned int *)val) == UFS_REF_CLK_OFF) : 0;

		if (IsUfsH8)
			spm_rc_constraint_valid_set(MT_RM_CONSTRAINT_ID_DRAM,
				    MT_RM_CONSTRAINT_ID_DRAM,
				    MT_SPM_RC_VALID_UFS_H8,
				    (struct constraint_status * const)&status);
		else
			spm_rc_constraint_valid_clr(MT_RM_CONSTRAINT_ID_DRAM,
				    MT_RM_CONSTRAINT_ID_DRAM,
				    MT_SPM_RC_VALID_UFS_H8,
				    (struct constraint_status * const)&status);
	} else if (type == PLAT_RC_STATUS) {
		const struct rc_common_state *st;

		st = (const struct rc_common_state *)val;

		do {
			if (!st)
				break;
			if (st->type == CONSTRAINT_UPDATE_COND_CHECK) {
				struct mt_spm_cond_tables * const tlb =
								&cond_dram;

				spm_rc_condition_modifier(st->id,
					   st->act, st->value,
					   MT_RM_CONSTRAINT_ID_DRAM, tlb);
			} else if ((st->type == CONSTRAINT_UPDATE_VALID)
				  || (st->type == CONSTRAINT_RESIDNECY))
				spm_rc_constraint_status_set(st->id, st->type,
				    st->act, MT_RM_CONSTRAINT_ID_DRAM,
				    (struct constraint_status * const)st->value,
				    (struct constraint_status * const)&status);
			else
				INFO("[%s:%d] - Unknown type: 0x%x\n",
					__func__, __LINE__, st->type);
		} while (0);
	}

	return res;
}

unsigned int spm_allow_rc_dram(int state_id)
{
	return CONSTRAINT_DRAM_ALLOW;
}

int spm_run_rc_dram(int cpu, int state_id)
{
	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;
	unsigned int allows = CONSTRAINT_DRAM_ALLOW;

	MT_SPM_RC_TAG((unsigned int)cpu, state_id, MT_RM_CONSTRAINT_ID_DRAM);
	MT_SPM_RC_TAG_VALID(status.is_valid);
	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_START);

	ext_status_dram = status.is_valid;

	if (IS_MT_SPM_RC_BBLPM_MODE(ext_status_dram)) {
#ifdef MT_SPM_USING_SRCLKEN_RC
		ext_op |= MT_SPM_EX_OP_SRCLKEN_RC_BBLPM;
#else
		allows |= MT_RM_CONSTRAINT_ALLOW_BBLPM;
#endif
	}

	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_NOTIFY);
#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_ENTER, allows
			       | (IS_PLAT_SUSPEND_ID(state_id) ?
			       MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND : 0));
#else
	(void)allows;
#endif
	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_WAKE_SPM_BEFORE);

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_enter(state_id,
				     (MT_SPM_EX_OP_CLR_26M_RECORD |
				      MT_SPM_EX_OP_SET_WDT |
				      MT_SPM_EX_OP_HW_S1_DETECT),
				     CONSTRAINT_DRAM_RESOURCE_REQ);
	} else {
		mt_spm_idle_generic_enter(state_id, ext_op, spm_dram_conduct);
	}

	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_WAKE_SPM_AFTER);
	return 0;
}

int spm_reset_rc_dram(int cpu, int state_id)
{
	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;
	unsigned int allows = CONSTRAINT_DRAM_ALLOW;

	MT_SPM_RC_FP(MT_SPM_RC_FP_RESUME_START);

	if (IS_MT_SPM_RC_BBLPM_MODE(ext_status_dram)) {
#ifdef MT_SPM_USING_SRCLKEN_RC
		ext_op |= MT_SPM_EX_OP_SRCLKEN_RC_BBLPM;
#else
		allows |= MT_RM_CONSTRAINT_ALLOW_BBLPM;
#endif
	}

	MT_SPM_RC_FP(MT_SPM_RC_FP_RESUME_NOTIFY);

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_LEAVE, allows);
#else
	(void)allows;
#endif
	MT_SPM_RC_FP(MT_SPM_RC_FP_RESUME_RESET_SPM_BEFORE);

	if (IS_PLAT_SUSPEND_ID(state_id))
		mt_spm_suspend_resume(state_id,
				(MT_SPM_EX_OP_SET_WDT
				| MT_SPM_EX_OP_HW_S1_DETECT),
				NULL);
	else {
		struct wake_status *waken = NULL;

		if (spm_unlikely(status.is_valid &
				MT_SPM_RC_VALID_TRACE_EVENT))
			ext_op |= MT_SPM_EX_OP_TRACE_LP;

		mt_spm_idle_generic_resume(state_id, ext_op, &waken);
		status.enter_cnt++;

		if (spm_unlikely(status.is_valid & MT_SPM_RC_VALID_RESIDNECY))
			status.residency += waken ?
					    waken->tr.comm.timer_out : 0;
	}
	MT_SPM_RC_FP(MT_SPM_RC_FP_INIT);

	return 0;
}

int spm_get_status_rc_dram(int type, void *priv)
{
	int ret = MT_RM_STATUS_OK;

	if (type == PLAT_RC_STATUS) {
		int res = 0;
		struct rc_common_state *st =
			(struct rc_common_state *)priv;

		if (!st)
			return MT_RM_STATUS_BAD;

		res = spm_rc_constraint_status_get(st->id, st->type,
			st->act, MT_RM_CONSTRAINT_ID_DRAM,
			(struct constraint_status * const)&status,
			(struct constraint_status * const)st->value);
		if (!res && (st->id != MT_RM_CONSTRAINT_ID_ALL))
			ret = MT_RM_STATUS_STOP;
	}
	return ret;
}

