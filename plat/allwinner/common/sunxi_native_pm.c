/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <sunxi_mmap.h>
#include <sunxi_private.h>

#define SUNXI_WDOG0_CTRL_REG		(SUNXI_R_WDOG_BASE + 0x0010)
#define SUNXI_WDOG0_CFG_REG		(SUNXI_R_WDOG_BASE + 0x0014)
#define SUNXI_WDOG0_MODE_REG		(SUNXI_R_WDOG_BASE + 0x0018)

static int sunxi_pwr_domain_on(u_register_t mpidr)
{
	sunxi_cpu_on(mpidr);

	return PSCI_E_SUCCESS;
}

static void sunxi_pwr_domain_off(const psci_power_state_t *target_state)
{
	gicv2_cpuif_disable();

	sunxi_cpu_power_off_self();
}

static void sunxi_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

static void __dead2 sunxi_system_off(void)
{
	gicv2_cpuif_disable();

	/* Attempt to power down the board (may not return) */
	sunxi_power_down();

	/* Turn off all CPUs */
	sunxi_cpu_power_off_others();
	sunxi_cpu_power_off_self();
	psci_power_down_wfi();
}

static void __dead2 sunxi_system_reset(void)
{
	gicv2_cpuif_disable();

	/* Reset the whole system when the watchdog times out */
	mmio_write_32(SUNXI_WDOG0_CFG_REG, 1);
	/* Enable the watchdog with the shortest timeout (0.5 seconds) */
	mmio_write_32(SUNXI_WDOG0_MODE_REG, (0 << 4) | 1);
	/* Wait for twice the watchdog timeout before panicking */
	mdelay(1000);

	ERROR("PSCI: System reset failed\n");
	panic();
}

static const plat_psci_ops_t sunxi_native_psci_ops = {
	.pwr_domain_on			= sunxi_pwr_domain_on,
	.pwr_domain_off			= sunxi_pwr_domain_off,
	.pwr_domain_on_finish		= sunxi_pwr_domain_on_finish,
	.system_off			= sunxi_system_off,
	.system_reset			= sunxi_system_reset,
	.validate_ns_entrypoint		= sunxi_validate_ns_entrypoint,
};

void sunxi_set_native_psci_ops(const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &sunxi_native_psci_ops;

	/*
	 * On the A64 U-Boot's SPL sets the bus clocks to some conservative
	 * values, to work around FEL mode instabilities with SRAM C accesses.
	 * FEL mode is gone when we reach ATF, so bring the AHB1 bus
	 * (the "main" bus) clock frequency back to the recommended 200MHz,
	 * for improved performance.
	 */
	if (soc_id == SUNXI_SOC_A64)
		mmio_write_32(SUNXI_CCU_BASE + 0x54, 0x00003180);

	/*
	 * U-Boot or the kernel don't setup AHB2, which leaves it at the
	 * AHB1 frequency (200 MHz, see above). However Allwinner recommends
	 * 300 MHz, for improved Ethernet and USB performance. Switch the
	 * clock to use "PLL_PERIPH0 / 2".
	 */
	if (soc_id == SUNXI_SOC_A64 || soc_id == SUNXI_SOC_H5)
		mmio_write_32(SUNXI_CCU_BASE + 0x5c, 0x1);
}
