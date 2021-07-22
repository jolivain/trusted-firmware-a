/*
 * Copyright (c) 2021, Sipeed
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <lib/mmio.h>

#include <sunxi_mmap.h>
#include <sunxi_cpucfg.h>
#include <sunxi_private.h>

int sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	/* Currently known hardware has no PMIC */

	return 0;
}

void sunxi_power_down(void)
{
}

void sunxi_cpu_power_off_self(void)
{
	u_register_t mpidr = read_mpidr();
	unsigned int core  = MPIDR_AFFLVL0_VAL(mpidr);

	/* Enable the CPUIDLE hardware (only really needs to be done once). */
	mmio_write_32(SUNXI_CPUIDLE_EN_REG, 0x16aa0000);
	mmio_write_32(SUNXI_CPUIDLE_EN_REG, 0xaa160001);

	/* Trigger power off for this core. */
	mmio_write_32(SUNXI_CORE_CLOSE_REG, BIT_32(core));
}
