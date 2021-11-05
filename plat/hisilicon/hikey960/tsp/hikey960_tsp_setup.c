/*
 * Copyright (c) 2021-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../hikey960_def.h"
#include "../hikey960_private.h"

#include <drivers/arm/gicv2.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>

static console_t console;

/*******************************************************************************
 * Initialize the UART
 ******************************************************************************/
void tsp_early_platform_setup(void)
{
	unsigned int id, uart_base;

	generic_delay_timer_init();
	hikey960_read_boardid(&id);
	if (id == 5300)
		uart_base = PL011_UART5_BASE;
	else
		uart_base = PL011_UART6_BASE;
	/* Initialize the console to provide early debug support */
	console_pl011_register(uart_base, PL011_UART_CLK_IN_HZ,
		PL011_BAUDRATE, &console);
}

/*******************************************************************************
 * Perform platform specific setup placeholder
 ******************************************************************************/
void tsp_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	//gicv2_driver_init(&hikey960_gic_data);
	//gicv2_distif_init();
	//gicv2_pcpu_distif_init();
	//gicv2_cpuif_enable();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the MMU
 ******************************************************************************/
void tsp_plat_arch_setup(void)
{
	hikey960_init_mmu_el3(BL32_DRAM_BASE,
			BL32_DRAM_LIMIT - BL32_DRAM_BASE,
			BL_CODE_BASE,
			BL_CODE_END,
			BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END);
}
