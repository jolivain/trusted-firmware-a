/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <mt_spm.h>
#include <platform_def.h>

/* CLK_SCP_CFG_0 */
#define CLK_SCP_CFG_0		(TOPCKGEN_BASE + 0x264)
#define SPM_CK_CONTROL_EN	0x7FF

void spm_boot_pre_init(void)
{
	/* switch ck_off/axi_26m control to SPM */
	mmio_setbits_32(CLK_SCP_CFG_0, SPM_CK_CONTROL_EN);
}

void spm_boot_post_init(void)
{
}
