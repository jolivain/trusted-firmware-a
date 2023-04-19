/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <platform_def.h>

#include "mtspmc.h"
#include "mtspmc_private.h"

void mcucfg_set_bootaddr(uint32_t cluster, uint32_t cpu, uintptr_t bootaddr)
{
	mmio_write_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR), bootaddr);
}

uintptr_t mcucfg_get_bootaddr(uint32_t cluster, uint32_t cpu)
{
	return mmio_read_32(per_cpu(cluster, cpu, MCUCFG_BOOTADDR));
}

void mcucfg_init_archstate(uint32_t cluster, uint32_t cpu, bool arm64)
{
	uintptr_t reg;

	reg = per_cluster(cluster, MCUCFG_INITARCH);

	if (arm64)
		mmio_setbits_32(reg, 1 << (12 + cpu));
	else
		mmio_clrbits_32(reg, 1 << (12 + cpu));
}

bool spm_get_powerstate(uint32_t mask)
{
	return !!(mmio_read_32(SPM_PWR_STATUS) & mask);
}

bool spm_get_cluster_powerstate(uint32_t cluster)
{
	uint32_t mask;

	mask = (cluster) ? MP1_CPUTOP : MP0_CPUTOP;

	return spm_get_powerstate(mask);
}

void spm_poweroff_cluster(uint32_t cluster)
{
	uint32_t mask;

	mmio_setbits_32(per_cluster(cluster, MCUCFG_CLUSTER_SPMC),
			SW_COQ_DIS | SW_NO_WAIT_Q);

	mask = (cluster) ? MP1_CPUTOP_PROT_BIT_MASK : MP0_CPUTOP_PROT_BIT_MASK;
	mmio_write_32(INFRA_TOPAXI_PROTECTEN_1_SET, mask);

	while ((mmio_read_32(INFRA_TOPAXI_PROTECTEN_STA1_1) & mask) != mask)
		;

	mmio_clrbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), PWR_ON);

	while (spm_get_cluster_powerstate(cluster))
		;
}

void spm_poweron_cluster(uint32_t cluster)
{
	uint32_t mask;

	mmio_setbits_32(per_cluster(cluster, SPM_CLUSTER_PWR), PWR_ON);

	while (!spm_get_cluster_powerstate(cluster))
		;

	mask = (cluster) ? MP1_CPUTOP_PROT_BIT_MASK : MP0_CPUTOP_PROT_BIT_MASK;
	mmio_write_32(INFRA_TOPAXI_PROTECTEN_1_CLR, mask);
}

int spmc_init(void)
{
	mmio_write_32(SPM_POWERON_CONFIG_EN,
		      PROJECT_CODE | MD_BCLK_CG_EN | BCLK_CG_EN |
		      AUDIO_MTCMOS_CG_EN | DISP_MTCMOS_CG_EN);

	/* 0: SPMC mode	 1: Legacy mode */
	mmio_write_32(SPM_BYPASS_SPMC, 0);

	mmio_clrbits_32(per_cluster(0, SPM_CLUSTER_PWR), PWR_ON_2ND);

	mmio_clrbits_32(per_cpu(0, 0, SPM_CPU_PWR), PWR_ON_2ND);
	mmio_clrbits_32(per_cpu(0, 1, SPM_CPU_PWR), PWR_ON_2ND);
	mmio_clrbits_32(per_cpu(0, 2, SPM_CPU_PWR), PWR_ON_2ND);
	mmio_clrbits_32(per_cpu(0, 3, SPM_CPU_PWR), PWR_ON_2ND);

	mmio_setbits_32(per_cpu(0, 1, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 2, SPM_CPU_PWR), PWR_RST_B);
	mmio_setbits_32(per_cpu(0, 3, SPM_CPU_PWR), PWR_RST_B);

	return 0;
}
