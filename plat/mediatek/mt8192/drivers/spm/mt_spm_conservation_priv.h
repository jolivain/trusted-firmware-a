/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_CONSERVATATION_PRIV_H
#define SPM_CONSERVATATION_PRIV_H

#include <mt_spm_vcorefs.h>

static void spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl,
					      const struct pwr_ctrl *src_pwr_ctrl)
{
	__spm_sync_vcore_dvfs_power_control(dest_pwr_ctrl, src_pwr_ctrl);
}

static void dump_cpu_pwr_status(void)
{
}
#endif
