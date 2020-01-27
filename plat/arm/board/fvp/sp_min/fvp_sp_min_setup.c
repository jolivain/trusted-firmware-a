/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <plat/arm/common/plat_arm.h>

#include "../fvp_private.h"

uintptr_t HW_CONFIG_DTB;

void plat_arm_sp_min_early_platform_setup(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	arm_sp_min_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);

	/* Initialize the platform config for future decision making */
	fvp_config_setup();

	/*
	 * Initialize the correct interconnect for this cluster during cold
	 * boot. No need for locks as no other CPU is active.
	 */
	fvp_interconnect_init();

	/*
	 * Enable coherency in interconnect for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this (e.g. Trusted
	 * Firmware's BL1 does it) but we can't assume so. There is no harm in
	 * executing this code twice anyway.
	 * FVP PSCI code will enable coherency for other clusters.
	 */
	fvp_interconnect_enable();

	HW_CONFIG_DTB = arg2;
}

void sp_min_plat_arch_setup(void)
{
	arm_sp_min_plat_arch_setup();

	assert(HW_CONFIG_DTB != 0U);

	INFO("SP_MIN FCONF: HW_CONFIG address = %p\n", (void *)HW_CONFIG_DTB);
	fconf_populate(HW_CONFIG_DTB, "HW_CONFIG");
}
