/*
 * Copyright (c) 2017-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/console.h>
#include <drivers/ti/uart/uart_16550.h>
#include <k3_console.h>

#include <platform_def.h>

void k3_console_setup(void)
{
	static console_t console;

	console_16550_register(K3_USART_BASE, K3_USART_CLK_SPEED, K3_USART_BAUD,
			       &console);

	console_set_scope(&console, CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME |
					    CONSOLE_FLAG_CRASH);
}
