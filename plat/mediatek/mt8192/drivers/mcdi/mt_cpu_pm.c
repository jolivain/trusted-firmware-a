/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <arch_helpers.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <stdint.h>

#include <mt_cpu_pm_cpc.h>
#include <mt_mcdi.h>
#include <plat_mtk_lpm.h>
#include <plat_pm.h>

#if SPM_READY
#include <sleep_def.h>
#include <mt_lp_irqremain.h>
#include <mt_lp_rm.h>
#include <mt_lpm_dispatch.h>
#include <mt_lpm_smc.h>
#endif

#if defined(MTK_CM_MGR)
#include <mtk_cm_mgr.h>
#endif

DEFINE_SYSREG_RW_FUNCS(dbgprcr_el1);

static int plat_mt_lp_cpu_rc;

static int pwr_state_prompt(int cpu, const psci_power_state_t *state)
{
	return 0;
}

static int pwr_state_reflect(int cpu, const psci_power_state_t *state)
{
	mtk_cpc_core_on_hint_clr(cpu);

	if (IS_SYSTEM_SUSPEND_STATE(state))
		mtk_cpc_time_sync();

	return 0;
}

static int pwr_cpu_pwron(int cpu, const psci_power_state_t *state)
{
	return 0;
}

static int pwr_cpu_pwrdwn(int cpu, const psci_power_state_t *state)
{
	/* clear DBGPRCR.CORENPDRQ to allow CPU power down  */
	write_dbgprcr_el1(0);

	return 0;
}

static int pwr_mcusys_pwron(int cpu, const psci_power_state_t *state)
{
	if (!IS_MCUSYS_OFF_STATE(state) || (plat_mt_lp_cpu_rc < 0))
		goto mt_pwr_mcusyson_break;

	mtk_cpc_mcusys_off_reflect();

	return 0;

mt_pwr_mcusyson_break:

	return -1;
}

static int pwr_mcusys_pwron_finished(int cpu, const psci_power_state_t *state)
{
#if SPM_READY
	int state_id = state->pwr_domain_state[MTK_AFFLVL_MCUSYS];
#endif

	if (!IS_MCUSYS_OFF_STATE(state) || (plat_mt_lp_cpu_rc < 0))
		goto mt_pwr_mcusyson_finished_break;

#if SPM_READY
	mt_lp_rm_reset_constraint(plat_mt_lp_cpu_rc, cpu, state_id);
	mt_lp_irqremain_release();
#endif

	return 0;

mt_pwr_mcusyson_finished_break:

	return -1;
}

static int pwr_mcusys_pwrdwn(int cpu, const psci_power_state_t *state)
{
#if SPM_READY
	int state_id = state->pwr_domain_state[MTK_AFFLVL_MCUSYS];
#endif

	if (!IS_MCUSYS_OFF_STATE(state))
		goto mt_pwr_mcusysoff_break;

	if (mcdi_try_init() != 0) /* not ready to process mcusys-off */
		goto mt_pwr_mcusysoff_break;

#if SPM_READY
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

#endif

mt_pwr_mcusysoff_break:
	plat_mt_lp_cpu_rc = -1;

	return -1;
}

static struct mt_lpm_tz plat_pm = {
	.pwr_prompt = pwr_state_prompt,
	.pwr_reflect = pwr_state_reflect,
	.pwr_cpu_on = pwr_cpu_pwron,
	.pwr_cpu_dwn = pwr_cpu_pwrdwn,
	.pwr_mcusys_dwn = pwr_mcusys_pwrdwn,
	.pwr_mcusys_on = pwr_mcusys_pwron,
	.pwr_mcusys_on_finished = pwr_mcusys_pwron_finished,
};

void mt_plat_cpu_pm_init(void)
{
	plat_mt_pm_register(&plat_pm);
	mtk_cpc_init();

#if SPM_READY
	mt_lp_irqremain_init();
#endif

	mcdi_try_init();

#if defined(MTK_CM_MGR)
	/* init cpu stall counter */
	init_cpu_stall_counter_all();
#endif
}
