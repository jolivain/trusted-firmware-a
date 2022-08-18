/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <mt_spm.h>
#include <platform_def.h>

/* CLK_SCP_CFG_0 */
#define CLK_SCP_CFG_0		(TOPCKGEN_BASE + 0x200)
#define SPM_CK_CONTROL_EN	0x3FF

/* CLK_SCP_CFG_1 */
#define CLK_SCP_CFG_1		(TOPCKGEN_BASE + 0x210)
#define CLK_SCP_CFG_1_MASK	0x100C
#define CLK_SCP_CFG_1_SPM	0x3

void spm_boot_pre_init(void)
{
	/* switch ck_off/axi_26m control to SPM */
	mmio_setbits_32(CLK_SCP_CFG_0, SPM_CK_CONTROL_EN);
	mmio_clrsetbits_32(CLK_SCP_CFG_1, CLK_SCP_CFG_1_MASK,
			   CLK_SCP_CFG_1_SPM);
}

void spm_boot_post_init(void)
{
}
