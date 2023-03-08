/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/pl011.h>
#include <drivers/console.h>

#include <platform_def.h>

static console_t console;

void qemu_console_init(void)
{
	(void)console_pl011_register(PLAT_QEMU_BOOT_UART_BASE,
				     PLAT_QEMU_BOOT_UART_CLK_IN_HZ,
				     PLAT_QEMU_CONSOLE_BAUDRATE, &console);

	console_set_scope(&console, CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);
}
