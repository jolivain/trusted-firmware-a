/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <plat_common.h>
#include <utils.h>
#include <common/debug.h>

/*
 * Perform arm specific early platform setup. At this moment we only initialize
 * the console and the memory layout.
 */
void plat_console_init(void)
{
	static console_t nxp_boot_console;
	struct sysinfo sys;

	zeromem(&sys, sizeof(sys));
	get_clocks(&sys);

	console_pl011_register(NXP_CONSOLE_ADDR,
			       (sys.freq_platform/NXP_UART_CLK_DIVIDER),
			       NXP_CONSOLE_BAUDRATE, &nxp_boot_console);
}
