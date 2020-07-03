/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <mcucfg.h>
#include <mtspmc.h>
#include <platform_def.h>

#include "mtspmc_private.h"

void mcucfg_disable_gic_wakeup(int cluster, int cpu)
{
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
}

void mcucfg_enable_gic_wakeup(int cluster, int cpu)
{
	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(cpu));
}

void mcucfg_set_bootaddr(int cluster, int cpu, uintptr_t bootaddr)
{
	assert(cluster == 0);

	mmio_write_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR), bootaddr);
}

uintptr_t mcucfg_get_bootaddr(int cluster, int cpu)
{
	assert(cluster == 0);

	return mmio_read_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR));
}

void mcucfg_init_archstate(int cluster, int cpu, int arm64)
{
	uint32_t reg;

	assert(cluster == 0);

	reg = per_cluster(cluster, MCUCFG_INITARCH);

	if (arm64)
		mmio_setbits_32(reg, 1 << (16 + cpu));
	else
		mmio_clrbits_32(reg, 1 << (16 + cpu));
}

/**
 * Return the someone subsystem's power state.
 *
 * @mask: mask to SPM_CPU_PWR_STATUS to query the power state
 *        of one subsystem.
 * RETURNS:
 * 0 (the subsys was powered off)
 * 1 (the subsys was powered on)
 */
int spm_get_powerstate(uint32_t mask)
{
	return (mmio_read_32(SPM_CPU_PWR_STATUS) & mask);
}

int spm_get_cluster_powerstate(int cluster)
{
	assert(cluster == 0);

	return spm_get_powerstate(MP0_CPUTOP);
}

int spm_get_cpu_powerstate(int cluster, int cpu)
{
	uint32_t mask = 1 << cpu;

	assert(cluster == 0);

	return spm_get_powerstate(mask);
}

int spmc_init(void)
{
	INFO("SPM: enable CPC mode\n");

	mmio_write_32(SPM_POWERON_CONFIG_EN, PROJECT_CODE | BCLK_CG_EN);

	mmio_setbits_32(per_cpu(0, 1, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 2, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 3, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 4, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 5, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 6, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 7, SPM_CPU_PWR), PWR_RST_B);

	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(1));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(2));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(3));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(4));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(5));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(6));
	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, GIC_WAKEUP_IGNORE(7));

	mmio_clrbits_32(SPM_MCUSYS_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(SPM_MP0_CPUTOP_PWR_CON, RESETPWRON_CONFIG);
	mmio_clrbits_32(per_cpu(0, 0, SPM_CPU_PWR), RESETPWRON_CONFIG);

	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, CPC_CTRL_ENABLE);

	return 0;
}

/**
 * Power on a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered on
 * @cpu: the CPU ID of the CPU which to be powered on
 */
void spm_poweron_cpu(int cluster, int cpu)
{
	/* set to 0 after BIG VPROC bulk on & before B-core power on seq. */
	if (cpu >= 4)
		mmio_write_32(DREQ20_BIG_VPROC_ISO, 0);

	mmio_setbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, SSPM_ALL_PWR_CTRL_EN);
	mmio_setbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);

	while (!spm_get_cpu_powerstate(cluster, cpu))
		;

	mmio_clrbits_32(MCUCFG_CPC_FLOW_CTRL_CFG, SSPM_ALL_PWR_CTRL_EN);

	/* Enable Big CPU Last PC */
	if (cpu >= 4)
		mmio_clrbits_32((MCUCFG_BASE + (cpu * 0x800)) + 0x308, BIT(3));
}

/**
 * Power off a core with specified cluster and core index
 *
 * @cluster: the cluster ID of the CPU which to be powered off
 * @cpu: the CPU ID of the CPU which to be powered off
 */
void spm_poweroff_cpu(int cluster, int cpu)
{
	/* Set mp0_spmc_pwr_on_cpuX = 0 */
	mmio_clrbits_32(per_cpu(cluster, cpu, SPM_CPU_PWR), PWR_ON);
}

/**
 * Power off a cluster with specified index
 *
 * @cluster: the cluster index which to be powered off
 */
void spm_poweroff_cluster(int cluster)
{
	/* CPU hotplug do not power off cluster for ARMv8.2 */
}

/**
 * Power on a cluster with specified index
 *
 * @cluster: the cluster index which to be powered on
 */
void spm_poweron_cluster(int cluster)
{
	/* CPU hotplug do not power on cluster for ARMv8.2 */
}
