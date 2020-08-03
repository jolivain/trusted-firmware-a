/*
 * Copyright (c) since 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef __MT_SPM_SUSPEDN_H__
#define __MT_SPM_SUSPEDN_H__

#include <mt_spm_internal.h>

#define MCUPM_MBOX_OFFSET_PDN		0x0C55FDA8
#define MCUPM_POWER_DOWN		0x4D50444E

struct suspend_dbg_ctrl {
	uint32_t sleep_suspend_cnt;
};

enum MT_SPM_SUSPEND_MODE {
	MT_SPM_SUSPEND_SYSTEM_PDN,
	MT_SPM_SUSPEND_SLEEP,
};

int mt_spm_suspend_mode_set(int mode, void *prv);

int mt_spm_suspend_enter(int state_id, unsigned int ext_opand,
				unsigned int reosuce_req);

void mt_spm_suspend_resume(int state_id, unsigned int ext_opand,
				    struct wake_status **status);

void mt_spm_suspend_init(void);

int mt_spm_suspend_get_spm_lp(struct spm_lp_scen **lp);

#endif

