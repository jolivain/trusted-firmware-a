/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <sunxi_cpucfg.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>
#include <drivers/sunxi_cpuidle.h>

#define CPU_PWR_LVL MPIDR_AFFLVL0

#define CPU_PWR_STATE(state) \
	((state)->pwr_domain_state[CPU_PWR_LVL])

#define CPUIDLE_DELAY 24 /* microseconds */

void sunxi_cpu_power_off_self(void)
{
	unsigned int core = MPIDR_AFFLVL0_VAL(read_mpidr());

	mmio_write_32(SUNXI_CLOSE_FLAG_REG, BIT_32(core));
}

void sunxi_cpuidle_on_finish(void)
{
	unsigned int core = MPIDR_AFFLVL0_VAL(read_mpidr());

	mmio_setbits_32(SUNXI_CPUIDLE_WAKE_REG, BIT_32(core));
}

void sunxi_cpuidle_suspend(const psci_power_state_t *target_state)
{
	unsigned int core = MPIDR_AFFLVL0_VAL(read_mpidr());

	gicv2_cpuif_disable();

	if (is_local_state_off(CPU_PWR_STATE(target_state))) {
		mmio_write_32(SUNXI_CLOSE_FLAG_REG, BIT_32(core));
	}
}

void sunxi_cpuidle_suspend_finish(const psci_power_state_t *target_state)
{
	gicv2_cpuif_enable();
}

void sunxi_cpuidle_off(void)
{
	unsigned int core = MPIDR_AFFLVL0_VAL(read_mpidr());

	mmio_clrbits_32(SUNXI_CPUIDLE_WAKE_REG, BIT_32(core));
}

void sunxi_cpuidle_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	CPU_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;

	for (unsigned int i = CPU_PWR_LVL + 1; i <= PLAT_MAX_PWR_LVL; ++i) {
		req_state->pwr_domain_state[i] = PLAT_MAX_RET_STATE;
	}
}

int sunxi_cpuidle_validate_power_state(unsigned int power_state,
				       psci_power_state_t *req_state)
{
	unsigned int power_level = psci_get_pstate_pwrlvl(power_state);
	unsigned int state_id = psci_get_pstate_id(power_state);
	unsigned int type = psci_get_pstate_type(power_state);
	unsigned int cpu_state;

	cpu_state = type == PSTATE_TYPE_STANDBY ? PLAT_MAX_RET_STATE
						: PLAT_MAX_OFF_STATE;

	if (power_level != CPU_PWR_LVL || state_id != cpu_state) {
		return PSCI_E_INVALID_PARAMS;
	}

	/* The CPU state is encoded in the passed-in state ID. */
	CPU_PWR_STATE(req_state) = cpu_state;

	/* Higher power domain levels should all remain running. */
	for (unsigned int i = CPU_PWR_LVL + 1; i <= PLAT_MAX_PWR_LVL; ++i) {
		req_state->pwr_domain_state[i] = PSCI_LOCAL_STATE_RUN;
	}

	return PSCI_E_SUCCESS;
}

void sunxi_cpuidle_init(void)
{
	/* Enable the CPUIDLE hardware. */
	mmio_write_32(SUNXI_CPUIDLE_EN_REG, 0x16aa0000);
	mmio_write_32(SUNXI_CPUIDLE_EN_REG, 0xaa160001);
	/* Set CPUIDLE state transition delays. */
	mmio_write_32(SUNXI_PWR_SW_DELAY_REG, CPUIDLE_DELAY);
	mmio_write_32(SUNXI_CONFIG_DELAY_REG, CPUIDLE_DELAY);
	/* Enable wakeup for core 0, which is already running. */
	sunxi_cpuidle_on_finish();
}
