/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mt_spm.h>
#include <mt_spm_conservation.h>
#include <mt_spm_internal.h>
#include <mt_spm_rc_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <mt_spm_suspend.h>
#include <plat_pm.h>
#include <uart.h>

static struct spm_lp_scen *__spm_suspend;

int mt_spm_suspend_mode_set(int mode)
{
	struct pwr_ctrl *pwrctrl = __spm_suspend->pwrctrl;

	if (mode == MT_SPM_SUSPEND_SLEEP) {
		pwrctrl->pcm_flags = SPM_SUSPEND_SLEEP_PCM_FLAG;
		pwrctrl->pcm_flags1 = SPM_SUSPEND_SLEEP_PCM_FLAG1;
	} else {
		pwrctrl->pcm_flags = SPM_SUSPEND_PCM_FLAG;
		pwrctrl->pcm_flags1 = SPM_SUSPEND_PCM_FLAG1;
	}

	return 0;
}

int mt_spm_suspend_enter(int state_id, unsigned int ext_opand,
			 unsigned int resource_req)
{
	/* If FMAudio / ADSP is active, change to sleep suspend mode */
	if ((ext_opand & MT_SPM_EX_OP_SET_SUSPEND_MODE) != 0U) {
		mt_spm_suspend_mode_set(MT_SPM_SUSPEND_SLEEP);
	}

	/* Notify MCUPM that device is going suspend flow */
	mmio_write_32(MCUPM_MBOX_OFFSET_PDN, MCUPM_POWER_DOWN);

	/* Notify UART to sleep */
	mt_uart_save();

	return spm_conservation(state_id, ext_opand,
				__spm_suspend, resource_req);
}

void mt_spm_suspend_resume(int state_id, unsigned int ext_opand,
			   struct wake_status **status)
{
	spm_conservation_finish(state_id, ext_opand, __spm_suspend, status);

	/* Notify UART to wakeup */
	mt_uart_restore();

	/* Notify MCUPM that device leave suspend */
	mmio_write_32(MCUPM_MBOX_OFFSET_PDN, 0);

	/* If FMAudio / ADSP is active, change back to suspend mode */
	if ((ext_opand & MT_SPM_EX_OP_SET_SUSPEND_MODE) != 0U) {
		mt_spm_suspend_mode_set(MT_SPM_SUSPEND_SYSTEM_PDN);
	}
}

void mt_spm_suspend_init(void)
{
	__spm_suspend = get_lp_scen_spm_suspend();
	assert(__spm_suspend != NULL);

	spm_conservation_pwrctrl_init(__spm_suspend->pwrctrl);
}
