/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

void plat_arm_sp_min_early_platform_setup(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
#if RESET_TO_SP_MIN
	/* ---------------------------------------------------------------------
	 * For RESET_TO_SP_MIN systems, BL32 (SP_MIN) is the first bootloader
	 * to run so there's no argument to relay from a previous bootloader.
	 * Zero the arguments passed to the platform layer to reflect that.
	 * ---------------------------------------------------------------------
	 */
	arm_sp_min_early_platform_setup((void *)0, 0, 0, (void *)0);
#else
	arm_sp_min_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);
#endif /* RESET_TO_SP_MIN */
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
