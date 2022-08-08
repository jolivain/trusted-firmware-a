/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <mcucfg.h>
#include <mtspmc.h>
#include <mtspmc_private.h>

void mcucfg_disable_gic_wakeup(unsigned int cluster, unsigned int cpu)
{
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
}

void mcucfg_set_bootaddr(unsigned int cluster, unsigned int cpu, uintptr_t bootaddr)
{
	assert(cluster == 0U);

	mmio_write_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR), bootaddr);
}

uintptr_t mcucfg_get_bootaddr(unsigned int cluster, unsigned int cpu)
{
	assert(cluster == 0U);

	return (uintptr_t)mmio_read_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR));
}

void mcucfg_init_archstate(unsigned int cluster, unsigned int cpu, bool arm64)
{
	uint32_t reg;

	assert(cluster == 0U);

	reg = per_cluster(cluster, MCUCFG_INITARCH);

	if (arm64) {
		mmio_setbits_32(reg, MCUCFG_INITARCH_CPU_BIT(cpu));
	} else {
		mmio_clrbits_32(reg, MCUCFG_INITARCH_CPU_BIT(cpu));
	}
}

bool spm_get_cpu_powerstate(unsigned int cluster, unsigned int cpu)
{
	uint32_t mask = BIT(cpu);

	assert(cluster == 0U);

	return spm_get_powerstate(mask);
}

/*
 * Power off a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered off
 * @cpu: the CPU ID of the CPU which to be powered off
 */
void spm_poweroff_cpu(unsigned int cluster, unsigned int cpu)
{
	/* Set mp0_spmc_pwr_on_cpuX = 0 */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);
}

/*
 * Power off a cluster with specified index
 *
 * @cluster: the cluster index which to be powered off
 */
void spm_poweroff_cluster(unsigned int cluster)
{
	/* No need to power on/off cluster on single cluster platform */
	assert(false);
}

/*
 * Power on a cluster with specified index
 *
 * @cluster: the cluster index which to be powered on
 */
void spm_poweron_cluster(unsigned int cluster)
{
	/* No need to power on/off cluster on single cluster platform */
	assert(false);
}
