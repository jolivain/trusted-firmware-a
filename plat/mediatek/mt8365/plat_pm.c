/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl31/bl31.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <mcdi/mt_mcdi.h>
#include <mt_gic_v3.h>
#include <spmc/mtspmc.h>

unsigned long sec_entrypoint;

static int plat_power_domain_on(unsigned long mpidr)
{
	unsigned int cpu = (unsigned int)plat_core_pos_by_mpidr(mpidr);
	unsigned int cluster = 0U;

	if (cpu >= PLATFORM_CORE_COUNT) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (!spm_get_cluster_powerstate(cluster)) {
		spm_poweron_cluster(cluster);
	}

	/* init cpu reset arch as AARCH64 */
	mcucfg_init_archstate(cluster, cpu, 1);
	mcucfg_set_bootaddr(cluster, cpu, sec_entrypoint);

	mcupm_hotplug_on(cpu);

	return PSCI_E_SUCCESS;
}

static void plat_power_domain_on_finish(const psci_power_state_t *state)
{
	uint16_t mpidr = read_mpidr();
	unsigned int cpu = (unsigned int)plat_core_pos_by_mpidr(mpidr);

	gicv3_rdistif_on(cpu);
	gicv3_cpuif_enable(cpu);
	mt_gic_rdistif_init();
	mt_gic_rdistif_restore();
}

static void plat_power_domain_off(const psci_power_state_t *state)
{
	uint16_t mpidr = read_mpidr();
	unsigned int cpu = (unsigned int)plat_core_pos_by_mpidr(mpidr);

	mt_gic_rdistif_save();
	gicv3_cpuif_disable(cpu);
	gicv3_rdistif_off(cpu);

	mcupm_hotplug_off(cpu);
}

static const plat_psci_ops_t plat_plat_pm_ops = {
	.pwr_domain_on	      = plat_power_domain_on,
	.pwr_domain_on_finish = plat_power_domain_on_finish,
	.pwr_domain_off	      = plat_power_domain_off,
};

int plat_setup_psci_ops(uintptr_t secure_entrypoint,
			const plat_psci_ops_t **plat_ops)
{
	*plat_ops = &plat_plat_pm_ops;
	sec_entrypoint = secure_entrypoint;

	/*
	 * init the warm reset config for boot CPU
	 *  a. reset arch as AARCH64
	 *  b. reset addr as function bl31_warm_entrypoint()
	 */
	mcucfg_init_archstate(0, 0, 1);
	mcucfg_set_bootaddr(0, 0, (uintptr_t) bl31_warm_entrypoint);

	spmc_init();

	return 0;
}
