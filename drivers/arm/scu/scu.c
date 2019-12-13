/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <drivers/arm/scu.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <stdint.h>

#define ACP_PRESENT_BIT 30
#define L2_CACHE_CTRL_PRESENT_BIT 31

#define CHECK_ACP_PRESENT 0x1
#define CHECK_L2_CACHE_CTRL_PRESENT 0x1

static scu_conf_desc_t scu_config_desc;

/*******************************************************************************
 * Turn ON snoop control unit. This is needed to synchronize the data between
 * CPU's.
 ******************************************************************************/
void enable_snoop_ctrl_unit(uintptr_t base)
{
	uint32_t scu_ctrl;

	INFO("[SCU]: enabling snoop control unit ... \n");

	assert(base != 0U);
	scu_ctrl = mmio_read_32(base + SCU_CTRL_REG);

	/* already enabled? */
	if (scu_ctrl & SCU_ENABLE_BIT) {
		return;
	}

	scu_ctrl |= SCU_ENABLE_BIT;
	mmio_write_32(base + SCU_CTRL_REG, scu_ctrl);
}

/*******************************************************************************
 * Snoop Control Unit configuration register. This is read-only register and
 * contatins information such as
 * - number of CPUs present
 * - is a particular CPU operating in SMP mode or AMP mode
 * - data cache size of a particular CPU
 * - does SCU has ACP port
 * - is L2CPRESENT
 ******************************************************************************/
scu_conf_desc_t *config_snoop_ctrl_unit(uintptr_t base)
{
	uint32_t scu_cfg;

	assert(base != 0U);
	scu_cfg = mmio_read_32(base + SCU_CFG_REG);

	/* Number of CPUs present */
	scu_config_desc.num_cpus = scu_cfg & 0x3;

	/* Core numbering starts from 0, hence increment by 1 */
	scu_config_desc.num_cpus++;
	INFO("[SCU]: number of CPUs present: %u\n", scu_config_desc.num_cpus);

	/* Cores that are in Symmetric Multiprocessing (SMP)
	 * or Asymmetric Multiprocessing (AMP) mode
	 */
	scu_config_desc.cpu_smp = (scu_cfg >> 4) & 0xF;

	for (unsigned int i = 0; i < scu_config_desc.num_cpus; i++)
		INFO("[SCU]: cpu%u is in %sMP mode\n", i,
			((scu_config_desc.cpu_smp >> i) & 0x1) ? "S":"A");

	/* Data cache size of each core */
	scu_config_desc.cpu0_data_cache_size = (scu_cfg >> 8) & 0xF;
	scu_config_desc.cpu1_data_cache_size = (scu_cfg >> 12) & 0xF;
	scu_config_desc.cpu2_data_cache_size = (scu_cfg >> 16) & 0xF;
	scu_config_desc.cpu3_data_cache_size = (scu_cfg >> 20) & 0xF;

	/* Reflects the Accelerator Coherency Port (ACP) configuration */
	scu_config_desc.acp_present = (scu_cfg >> ACP_PRESENT_BIT) & CHECK_ACP_PRESENT;

	/* This enables software to determine the presence
	 * of L2 cache controller in the system
	 */
	scu_config_desc.l2_cache_ctrl_present = (scu_cfg >> L2_CACHE_CTRL_PRESENT_BIT) &
						CHECK_L2_CACHE_CTRL_PRESENT;

	return &scu_config_desc;
}
