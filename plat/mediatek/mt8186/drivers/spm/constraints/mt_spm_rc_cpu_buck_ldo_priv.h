/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RC_CPU_BUCK_LDO_PRIV_H
#define MT_SPM_RC_CPU_BUCK_LDO_PRIV_H

#include <mt_spm_reg.h>

#define CONSTRAINT_CPU_BUCK_PCM_FLAG	\
	(SPM_FLAG_DISABLE_INFRA_PDN |	\
	 SPM_FLAG_DISABLE_VCORE_DVS |	\
	 SPM_FLAG_DISABLE_VCORE_DFS |	\
	 SPM_FLAG_SRAM_SLEEP_CTRL |	\
	 SPM_FLAG_KEEP_CSYSPWRACK_HIGH)

#endif /* MT_SPM_RC_CPU_BUCK_LDO_PRIV_H */
