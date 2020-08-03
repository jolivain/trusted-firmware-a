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

#ifndef ATF_PLAT_CIRQ_UNSUPPORT
#include <plat_mt_cirq.h>
#include <mt_gic_v3.h>
#endif

#include <mt_spm_notifier.h>
#include <mt_spm_rc_api.h>

#define CONSTRAINT_BUS26M_ALLOW (MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF\
				| MT_RM_CONSTRAINT_ALLOW_DRAM_S0\
				| MT_RM_CONSTRAINT_ALLOW_DRAM_S1\
				| MT_RM_CONSTRAINT_ALLOW_VCORE_LP\
				| MT_RM_CONSTRAINT_ALLOW_LVTS_STATE\
				| MT_RM_CONSTRAINT_ALLOW_BUS26M_OFF)

#define CONSTRAINT_BUS26M_PCM_FLAG (\
				SPM_FLAG_DISABLE_INFRA_PDN\
				| SPM_FLAG_DISABLE_VCORE_DVS\
				| SPM_FLAG_DISABLE_VCORE_DFS\
				| SPM_FLAG_SRAM_SLEEP_CTRL\
				| SPM_FLAG_ENABLE_TIA_WORKAROUND\
				| SPM_FLAG_ENABLE_LVTS_WORKAROUND\
				| SPM_FLAG_KEEP_CSYSPWRACK_HIGH)

#define CONSTRAINT_BUS26M_PCM_FLAG1 (SPM_FLAG1_DISABLE_MD26M_CK_OFF)

/*
 * If sspm sram won't enter sleep voltage
 * then vcore couldn't enter low power mode
 */

#if defined(ATF_PLAT_SPM_SRAM_SLP_UNSUPPORT) && SPM_SRAM_SLEEP_RC_RES_RESTRICT
#define CONSTRAINT_BUS26M_RESOURCE_REQ	(MT_SPM_26M)
#else
#define CONSTRAINT_BUS26M_RESOURCE_REQ	(0)
#endif
static unsigned int bus26m_ext_opand;

static struct mt_irqremain *refer2remain_irq;

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
	.table_pll = (PLL_BIT_UNIVPLL
		    | PLL_BIT_MFGPLL
		    | PLL_BIT_MSDCPLL
		    | PLL_BIT_TVDPLL
		    | PLL_BIT_MMPLL),
};

static struct mt_spm_cond_tables cond_bus26m_res = {
	.table_cg = {0},
	.table_pll = 0,
};

static struct constraint_status status = {
	.id = MT_RM_CONSTRAINT_ID_BUS26M,
	.is_valid = (MT_SPM_RC_VALID_SW |
		     MT_SPM_RC_VALID_COND_CHECK |
		     MT_SPM_RC_VALID_COND_LATCH),
	.is_cond_block = 0,
	.enter_cnt = 0,
	.cond_res = &cond_bus26m_res,
	.residency = 0,
};

/*
 * Cirq will respresent interrupt which type is edge when gic mask.
 * But if the 26 clk have turned off before then cirq won't work normally.
 * So we need to set irq pending for specific wakeup source.
 */
#ifdef ATF_PLAT_CIRQ_UNSUPPORT
#define do_irqs_delivery()
#else
static void mt_spm_irq_remain_dump(struct mt_irqremain *irqs,
				   unsigned int irq_index,
				   struct wake_status *wakeup)
{
	INFO("[SPM] r12 = 0x%08x(0x%08x), flag = 0x%08x 0x%08x 0x%08x\n",
	     wakeup->tr.comm.r12, wakeup->md32pcm_wakeup_sta,
	     wakeup->tr.comm.debug_flag, wakeup->tr.comm.b_sw_flag0,
	     wakeup->tr.comm.b_sw_flag1);

	INFO("irq:%u(0x%08x) set pending\n",
	     irqs->wakeupsrc[irq_index], irqs->irqs[irq_index]);
}

static void do_irqs_delivery(void)
{
	unsigned int idx;
	int res = 0;
	struct wake_status *wakeup = NULL;
	struct mt_irqremain *irqs = refer2remain_irq;

	if (!irqs)
		return;

	res = spm_conservation_get_result(&wakeup);

	if ((res == 0) && irqs) {
		for (idx = 0; idx < irqs->count; ++idx) {
			if ((wakeup->tr.comm.r12 & irqs->wakeupsrc[idx]) ||
			    (wakeup->raw_sta & irqs->wakeupsrc[idx])) {
				if ((irqs->wakeupsrc_cat[idx] &
				    MT_IRQ_REMAIN_CAT_LOG))
					mt_spm_irq_remain_dump(irqs,
							idx, wakeup);
				mt_irq_set_pending(irqs->irqs[idx]);
			}
		}
	}
}
#endif

int spm_bus26m_conduct(int state_id, struct spm_lp_scen *spm_lp,
		       unsigned int *resource_req)
{
	unsigned int res_req = CONSTRAINT_BUS26M_RESOURCE_REQ;

	spm_lp->pwrctrl->pcm_flags = (uint32_t)CONSTRAINT_BUS26M_PCM_FLAG;
	spm_lp->pwrctrl->pcm_flags1 = (uint32_t)CONSTRAINT_BUS26M_PCM_FLAG1;
	*resource_req |= res_req;

	return 0;
}

int spm_is_valid_rc_bus26m(int cpu, int state_id)
{
	return (!(status.is_cond_block &&
		(status.is_valid & MT_SPM_RC_VALID_COND_CHECK))
		&& IS_MT_RM_RC_READY(status.is_valid));
}

int spm_update_rc_bus26m(int state_id, int type, const void *val)
{
	int res = MT_RM_STATUS_OK;

	if (type == PLAT_RC_UPDATE_CONDITION) {
		const struct mt_spm_cond_tables *tlb =
				(const struct mt_spm_cond_tables *)val;
		const struct mt_spm_cond_tables *tlb_check =
				(const struct mt_spm_cond_tables *)&cond_bus26m;

		if (!tlb)
			return MT_RM_STATUS_BAD;

		status.is_cond_block =
			mt_spm_cond_check(state_id, tlb, tlb_check,
					  (status.is_valid &
					   MT_SPM_RC_VALID_COND_LATCH) ?
					  &cond_bus26m_res : NULL);
	} else if (type == PLAT_RC_UPDATE_REMAIN_IRQS) {
		refer2remain_irq = (struct mt_irqremain *)val;
	} else if ((type == PLAT_RC_IS_FMAUDIO)
		   || (type == PLAT_RC_IS_ADSP)) {
		int *flag = (int *)val;

		if (*flag)
			bus26m_ext_opand |= (type == PLAT_RC_IS_ADSP) ?
				(MT_SPM_EX_OP_SET_IS_ADSP
				 | MT_SPM_EX_OP_SET_SUSPEND_MODE) :
				MT_SPM_EX_OP_SET_SUSPEND_MODE;
		else
			bus26m_ext_opand &= (type == PLAT_RC_IS_ADSP) ?
				~(MT_SPM_EX_OP_SET_IS_ADSP
				  | MT_SPM_EX_OP_SET_SUSPEND_MODE) :
				~MT_SPM_EX_OP_SET_SUSPEND_MODE;
	} else if (type == PLAT_RC_STATUS) {
		const struct rc_common_state *st;

		st = (const struct rc_common_state *)val;

		do {
			if (!st)
				break;
			if (st->type == CONSTRAINT_UPDATE_COND_CHECK) {
				struct mt_spm_cond_tables * const tlb =
								&cond_bus26m;

				spm_rc_condition_modifier(st->id,
					     st->act, st->value,
					     MT_RM_CONSTRAINT_ID_BUS26M, tlb);
			} else if ((st->type == CONSTRAINT_UPDATE_VALID)
				  || (st->type == CONSTRAINT_RESIDNECY))
				spm_rc_constraint_status_set(st->id,
				     st->type, st->act,
				     MT_RM_CONSTRAINT_ID_BUS26M,
				    (struct constraint_status * const)st->value,
				    (struct constraint_status * const)&status);
			else
				INFO("[%s:%d] - Unknown type: 0x%x\n",
					__func__, __LINE__, st->type);
		} while (0);
	}

	return res;
}

unsigned int spm_allow_rc_bus26m(int state_id)
{
	return CONSTRAINT_BUS26M_ALLOW;
}

int spm_run_rc_bus26m(int cpu, int state_id)
{
	MT_SPM_RC_TAG((unsigned int)cpu, state_id, MT_RM_CONSTRAINT_ID_BUS26M);
	MT_SPM_RC_TAG_VALID(status.is_valid);
	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_NOTIFY);

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_ENTER, CONSTRAINT_BUS26M_ALLOW
			       | (IS_PLAT_SUSPEND_ID(state_id) ?
			       MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND : 0));
#endif
	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_WAKE_SPM_BEFORE);

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_enter(state_id,
				     (MT_SPM_EX_OP_CLR_26M_RECORD |
				      MT_SPM_EX_OP_SET_WDT |
				      MT_SPM_EX_OP_HW_S1_DETECT |
				      bus26m_ext_opand),
				     CONSTRAINT_BUS26M_RESOURCE_REQ);
	} else {
		mt_spm_idle_generic_enter(state_id, MT_SPM_EX_OP_HW_S1_DETECT,
					  spm_bus26m_conduct);
	}

	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_WAKE_SPM_AFTER);
	return 0;
}

int spm_reset_rc_bus26m(int cpu, int state_id)
{
	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;

	MT_SPM_RC_FP(MT_SPM_RC_FP_RESUME_NOTIFY);
#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_LEAVE, 0);
#endif
	MT_SPM_RC_FP(MT_SPM_RC_FP_RESUME_RESET_SPM_BEFORE);

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_resume(state_id, (bus26m_ext_opand
						| MT_SPM_EX_OP_SET_WDT
						| ext_op),
					NULL);
		bus26m_ext_opand = 0;
	} else {
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
	MT_SPM_RC_FP(MT_SPM_RC_FP_RESUME_BACKUP_EDGE_INT);
	do_irqs_delivery();
	MT_SPM_RC_FP(MT_SPM_RC_FP_INIT);
	return 0;
}

int spm_get_status_rc_bus26m(int type, void *priv)
{
	int ret = MT_RM_STATUS_OK;

	if (type == PLAT_RC_STATUS) {
		int res = 0;
		struct rc_common_state *st =
			(struct rc_common_state *)priv;

		if (!st)
			return MT_RM_STATUS_BAD;

		res = spm_rc_constraint_status_get(st->id, st->type,
			st->act, MT_RM_CONSTRAINT_ID_BUS26M,
			(struct constraint_status * const)&status,
			(struct constraint_status * const)st->value);
		if (!res && (st->id != MT_RM_CONSTRAINT_ID_ALL))
			ret = MT_RM_STATUS_STOP;
	}
	return ret;
}

