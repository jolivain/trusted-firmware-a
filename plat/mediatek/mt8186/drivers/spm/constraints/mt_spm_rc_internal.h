/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RC_INTERNAL_H
#define MT_SPM_RC_INTERNAL_H

#include <mt_spm_rc_common.h>
#include <sleep_def.h>

#define SPM_FLAG_SRAM_SLEEP_CTRL			\
	(SPM_FLAG_DISABLE_SSPM_SRAM_SLEEP |		\
	 SPM_FLAG_DISABLE_DRAMC_MCU_SRAM_SLEEP |	\
	 SPM_FLAG_DISABLE_SYSRAM_SLEEP |		\
	 SPM_FLAG_DISABLE_MCUPM_SRAM_SLEEP |		\
	 SPM_FLAG_DISABLE_SRAM_EVENT)

#endif /* MT_SPM_RC_INTERNAL_H */
