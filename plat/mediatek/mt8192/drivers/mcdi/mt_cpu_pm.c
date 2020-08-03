/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <sleep_def.h>
#include <stdint.h>

#include <mt_cpu_pm.h>
#include <mt_cpu_pm_cpc.h>
#include <mt_lp_irqremain.h>
#include <mt_lp_rm.h>
#include <mt_lpm_dispatch.h>
#include <mt_lpm_smc.h>
#include <mt_mcdi.h>
#include <plat_mtk_lpm.h>
#include <plat_pm.h>

/* ARM v8.2, the cache will turn off automatically when cpu
 * power down. So, there is no doubt to use the spin_lock here
 */
#if !HW_ASSISTED_COHERENCY
#define MT_CPU_PM_USING_BAKERY_LOCK
#endif

/* The locker must use the bakery locker when cache turn off.
 * Using spin_lock will has better performance.
 */
#ifdef MT_CPU_PM_USING_BAKERY_LOCK
DEFINE_BAKERY_LOCK(mt_cpu_pm_lock);

#define plat_cpu_pm_lock_init()\
	bakery_lock_init(&mt_cpu_pm_lock)

#define plat_cpu_pm_lock()\
	bakery_lock_get(&mt_cpu_pm_lock)

#define plat_cpu_pm_unlock()\
	bakery_lock_release(&mt_cpu_pm_lock)
#else
spinlock_t mt_cpu_pm_lock;

#define plat_cpu_pm_lock_init()
#define plat_cpu_pm_lock()\
	spin_lock(&mt_cpu_pm_lock)

#define plat_cpu_pm_unlock()\
	spin_unlock(&mt_cpu_pm_lock)
#endif

DEFINE_SYSREG_RW_FUNCS(dbgprcr_el1);

static int plat_mt_lp_cpu_rc;

static int pwr_state_prompt(unsigned int cpu, const psci_power_state_t *state)
{
	return 0;
}

static int pwr_state_reflect(unsigned int cpu, const psci_power_state_t *state)
{
	mtk_cpc_core_on_hint_clr(cpu);

	if (IS_SYSTEM_SUSPEND_STATE(state)) {
		mtk_cpc_time_sync();
	}

	return 0;
}

static int pwr_cpu_pwron(unsigned int cpu, const psci_power_state_t *state)
{
	return 0;
}

static int pwr_cpu_pwrdwn(unsigned int cpu, const psci_power_state_t *state)
{
	/* clear DBGPRCR.CORENPDRQ to allow CPU power down  */
	write_dbgprcr_el1(0ULL);

	return 0;
}

static int pwr_cluster_pwron(unsigned int cpu, const psci_power_state_t *state)
{
	return 0;
}

static int pwr_cluster_pwrdwn(unsigned int cpu, const psci_power_state_t *state)
{
	return 0;
}

static int pwr_mcusys_pwron(unsigned int cpu, const psci_power_state_t *state)
{
	if ((!IS_MCUSYS_OFF_STATE(state) && !IS_SYSTEM_SUSPEND_STATE(state)) ||
	    (plat_mt_lp_cpu_rc < 0))
		return -1;

	mtk_cpc_mcusys_off_reflect();

	return 0;
}

static int pwr_mcusys_pwron_finished(unsigned int cpu,
					const psci_power_state_t *state)
{
	int state_id = state->pwr_domain_state[MTK_AFFLVL_MCUSYS];

	if ((!IS_MCUSYS_OFF_STATE(state) && !IS_SYSTEM_SUSPEND_STATE(state)) ||
	    (plat_mt_lp_cpu_rc < 0))
		return -1;

	mt_lp_rm_reset_constraint(plat_mt_lp_cpu_rc, cpu, state_id);
	mt_lp_irqremain_release();

	return 0;
}

static int pwr_mcusys_pwrdwn(unsigned int cpu, const psci_power_state_t *state)
{
	int state_id = state->pwr_domain_state[MTK_AFFLVL_MCUSYS];

	if (!IS_MCUSYS_OFF_STATE(state) && !IS_SYSTEM_SUSPEND_STATE(state))
		goto mt_pwr_mcusysoff_break;

	if (mcdi_try_init() != 0)
		goto mt_pwr_mcusysoff_break;

	if (mtk_cpc_mcusys_off_prepare() != CPC_SUCCESS)
		goto mt_pwr_mcusysoff_break;

	plat_mt_lp_cpu_rc =
		mt_lp_rm_find_and_run_constraint(1, 0, cpu, state_id, NULL);

	if (plat_mt_lp_cpu_rc < 0)
		goto mt_pwr_mcusysoff_reflect;

	mt_lp_irqremain_aquire();

	return 0;

mt_pwr_mcusysoff_reflect:
	mtk_cpc_mcusys_off_reflect();

mt_pwr_mcusysoff_break:
	plat_mt_lp_cpu_rc = -1;

	return -1;
}

static uint64_t mt_cpu_pm_compatible_smc_id(uint64_t lp_id)
{
	switch (lp_id) {
	case MT_LPM_SPMC_COMPAT_LK_MCDI_WDT_DUMP:
		lp_id = CPC_COMMAND;
		break;
	default:
		break;
	}
	return lp_id;
}

static uint64_t mt_cpu_pm_compatible_smc_act(uint64_t lp_id, uint64_t act)
{
	switch (lp_id) {
	case MT_LPM_SPMC_COMPAT_LK_MCDI_WDT_DUMP:
		act = (act & MT_LPM_SMC_ACT_SET) ?
		      CPC_SMC_EVENT_DUMP_TRACE_DATA : act;
		break;
	default:
		break;
	}
	return act;
}

uint64_t mt_cpu_pm_dispatcher(uint64_t lp_id, uint64_t act,
				     uint64_t arg1, uint64_t arg2,
				     void *cookie,
				     void *handle,
				     uint64_t flags)
{
	uint64_t res = 0;

	if (act & MT_LPM_SMC_ACT_COMPAT) {
		plat_cpu_pm_lock();
		lp_id = mt_cpu_pm_compatible_smc_id(lp_id);
		act &= ~(MT_LPM_SMC_ACT_COMPAT);
		act = mt_cpu_pm_compatible_smc_act(lp_id, act);
		plat_cpu_pm_unlock();
	}

	switch (lp_id) {
	case CPC_COMMAND:
		res = mtk_cpc_handler(act, arg1, arg2);
		break;
	default:
		break;
	}

	return res;
}

uint64_t mt_cpu_pm_lp_dispatcher(uint64_t lp_id, uint64_t act,
				     uint64_t arg1, uint64_t arg2,
				     void *cookie,
				     void *handle,
				     uint64_t flags)
{
	uint64_t res = 0;
	int ret;

	switch (lp_id) {
	case LP_CPC_COMMAND:
		res = mtk_cpc_handler(act, arg1, arg2);
		break;
	case IRQS_REMAIN_ALLOC:
		if (act & MT_LPM_SMC_ACT_GET)
			res = (uint64_t)mt_lp_irqremain_count();
		break;
	case IRQS_REMAIN_CTRL:
		plat_cpu_pm_lock();
		if (act & MT_LPM_SMC_ACT_SUBMIT)
			ret = mt_lp_irqremain_submit();
		else if (act & MT_LPM_SMC_ACT_PUSH) {
			ret = mt_lp_irqremain_push();
			if (ret)
				INFO("Irqs remain push fail\n");
		} else
			INFO("Irqs remain control not support! (0x%llx)\n", act);
		plat_cpu_pm_unlock();
		break;
	case IRQS_REMAIN_IRQ:
	case IRQS_REMAIN_WAKEUP_CAT:
	case IRQS_REMAIN_WAKEUP_SRC:
		plat_cpu_pm_lock();
		if (act & MT_LPM_SMC_ACT_SET) {
			const struct mt_lp_irqinfo info = {
				.val = (unsigned int)arg1,
			};

			ret = mt_lp_irqremain_set((unsigned int)lp_id, &info);
			if (ret)
				INFO("Irqs remain command: %llu, set fail\n", lp_id);
		} else if (act & MT_LPM_SMC_ACT_GET) {
			struct mt_lp_irqinfo info;

			ret = mt_lp_irqremain_get((int)arg1,
					(unsigned int)lp_id, &info);
			if (ret) {
				INFO("Irqs remain command: %llu, get fail\n", lp_id);
				res = 0;
			} else
				res = (uint64_t)info.val;
		} else
			INFO("Irqs remain command not support! (0x%llx)\n", act);
		plat_cpu_pm_unlock();
		break;
	default:
		break;
	}

	return res;
}

static struct mt_lpm_tz plat_pm = {
	.pwr_prompt			= pwr_state_prompt,
	.pwr_reflect			= pwr_state_reflect,
	.pwr_cpu_on			= pwr_cpu_pwron,
	.pwr_cpu_dwn			= pwr_cpu_pwrdwn,
	.pwr_cluster_on			= pwr_cluster_pwron,
	.pwr_cluster_dwn		= pwr_cluster_pwrdwn,
	.pwr_mcusys_dwn			= pwr_mcusys_pwrdwn,
	.pwr_mcusys_on			= pwr_mcusys_pwron,
	.pwr_mcusys_on_finished		= pwr_mcusys_pwron_finished
};

const struct mt_lpm_tz *mt_plat_cpu_pm_init(void)
{
	mtk_cpc_init();

	if (mcdi_try_init() == 0) {
		INFO("MCDI init done.\n");
	}

	mt_lp_irqremain_init();
	mt_lpm_dispatcher_registry(mt_lpm_smc_user_cpu_pm,
				   mt_cpu_pm_dispatcher);
	mt_lpm_dispatcher_registry(mt_lpm_smc_user_cpu_pm_lp,
				   mt_cpu_pm_lp_dispatcher);

	return &plat_pm;
}
