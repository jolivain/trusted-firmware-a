/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <platform_def.h>
#include <mt_lpm_dispatch.h>
#include <mt_lpm_smc.h>
#include <mt_spm_dispatcher.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_smc.h>
#include <mt_spm_conservation.h>

#include <mt_spm_suspend.h>
#include <pcm_def.h>

static void mt_spm_pcm_wdt(uint64_t enable, uint64_t time)
{
	mmio_write_32(PCM_TIMER_VAL, time);
	__spm_set_pcm_wdt(enable);
}

static unsigned int mt_spm_phypll_mode_check(void)
{
	unsigned int val = mmio_read_32(SPM_POWER_ON_VAL0);

	if ((val & (R0_SC_PHYPLL_MODE_SW
		    | R0_SC_PHYPLL2_MODE_SW))
		    == R0_SC_PHYPLL_MODE_SW)
		val = 0;

	return val;
}

uint64_t mt_spm_dispatcher(uint64_t lp_id, uint64_t act,
				 uint64_t arg1, uint64_t arg2,
				 void *cookie, void *handle,
				 uint64_t flags)
{
	uint64_t ret = 0;

	switch (lp_id) {
	case MT_SPM_SMC_UID_STATUS:
		if (act & MT_LPM_SMC_ACT_SET) {
			if (arg2 & MT_SPM_STATUS_SUSPEND_SLEEP)
				/* legacy audio check from kernel */
				mt_spm_suspend_mode_set(
					MT_SPM_SUSPEND_SLEEP, NULL);
		} else if (act & MT_LPM_SMC_ACT_CLR)
			if (arg2 & MT_SPM_STATUS_SUSPEND_SLEEP)
				mt_spm_suspend_mode_set(
					MT_SPM_SUSPEND_SYSTEM_PDN, NULL);
		break;
	case MT_SPM_SMC_UID_PCM_WDT:
		if (act & MT_LPM_SMC_ACT_SET)
			mt_spm_pcm_wdt(1, arg2);
		else if (act & MT_LPM_SMC_ACT_CLR)
			mt_spm_pcm_wdt(0, arg2);
		break;
	case MT_SPM_SMC_UID_PHYPLL_MODE:
		if (act & MT_LPM_SMC_ACT_GET)
			ret = mt_spm_phypll_mode_check();
		break;
	default:
		break;
	}
	return ret;
}

int mt_spm_dispatcher_init(void)
{
	mt_lpm_dispatcher_registry(mt_lpm_smc_user_spm,
				   mt_spm_dispatcher);
	return 0;
}

