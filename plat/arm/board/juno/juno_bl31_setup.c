/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <plat/arm/common/plat_arm.h>

static uintptr_t hw_config_dtb;

void __init bl31_early_platform_setup2(u_register_t arg0,
		u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
	hw_config_dtb = arg2;

	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_arm_interconnect_init();

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this (e.g. Trusted
	 * Firmware's BL1 does it) but we can't assume so. There is no harm in
	 * executing this code twice anyway.
	 * Platform specific PSCI code will enable coherency for other
	 * clusters.
	 */
	plat_arm_interconnect_enter_coherency();
}

void __init bl31_plat_arch_setup(void)
{
	arm_bl31_plat_arch_setup();

	fconf_populate("HW_CONFIG", hw_config_dtb);
}
