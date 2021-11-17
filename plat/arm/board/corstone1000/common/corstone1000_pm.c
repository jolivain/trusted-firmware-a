/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
/*******************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/

#define SECURE_WATCHDOG_ADDR_CTRL_REG     0x1A320000
#define SECURE_WATCHDOG_ADDR_VAL_REG      0x1A320008
#define SECURE_WATCHDOG_MASK_ENABLE       0x01
#define SECURE_WATCHDOG_COUNTDOWN_VAL     0x1000

static void __dead2 corstone1000_system_reset(void)
{

	uint32_t volatile * const watchdog_ctrl_reg = (int *) SECURE_WATCHDOG_ADDR_CTRL_REG;
	uint32_t volatile * const watchdog_val_reg = (int *) SECURE_WATCHDOG_ADDR_VAL_REG;

	*(watchdog_val_reg) = SECURE_WATCHDOG_COUNTDOWN_VAL;
	*watchdog_ctrl_reg = SECURE_WATCHDOG_MASK_ENABLE;
	while (1) {
		 wfi();
	 }
}

plat_psci_ops_t plat_arm_psci_pm_ops = {
	.system_reset = corstone1000_system_reset,
	.validate_ns_entrypoint = NULL
};

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	ops = &plat_arm_psci_pm_ops;
	return ops;
}
