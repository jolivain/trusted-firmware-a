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
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <plat_pm.h>

static struct spm_lp_scen *idle_spm_lp;

int mt_spm_idle_generic_enter(int state_id, unsigned int ext_opand,
			      spm_idle_conduct fn)
{
	unsigned int src_req = 0U;

	if (fn != NULL) {
		fn(idle_spm_lp, &src_req);
	}

	return spm_conservation(state_id, ext_opand, idle_spm_lp, src_req);
}

void mt_spm_idle_generic_resume(int state_id, unsigned int ext_opand,
				struct wake_status **status)
{
	ext_opand |= idle_spm_lp->resume_opand;
	spm_conservation_finish(state_id, ext_opand, idle_spm_lp, status);
}

void mt_spm_idle_generic_init(void)
{
	idle_spm_lp = get_lp_scen_spm_idle();
	assert(idle_spm_lp != NULL);

	spm_conservation_pwrctrl_init(idle_spm_lp->pwrctrl);
}
