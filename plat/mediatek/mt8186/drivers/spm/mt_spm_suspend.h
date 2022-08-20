/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_SUSPEND_H
#define MT_SPM_SUSPEND_H

#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_suspend_common.h>

#define MCUPM_MBOX_OFFSET_PDN	(0x0C55FDA8)
#define MCUPM_POWER_DOWN	(0x4D50444E)

#define SPM_SUSPEND_SLEEP_PCM_FLAG		\
	(SPM_FLAG_DISABLE_INFRA_PDN |		\
	 SPM_FLAG_DISABLE_VCORE_DVS |		\
	 SPM_FLAG_DISABLE_VCORE_DFS |		\
	 SPM_FLAG_USE_SRCCLKENO2)

#define SPM_SUSPEND_SLEEP_PCM_FLAG1		(0U)

#define SPM_SUSPEND_PCM_FLAG			\
	(SPM_FLAG_DISABLE_VCORE_DVS |		\
	 SPM_FLAG_DISABLE_VCORE_DFS)

#define SPM_SUSPEND_PCM_FLAG1			(0U)

#endif /* MT_SPM_SUSPEND_H */
