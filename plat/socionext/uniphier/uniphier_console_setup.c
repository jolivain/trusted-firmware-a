/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/console.h>

#include "uniphier.h"

#define UNIPHIER_UART_BASE		0x54006800
#define UNIPHIER_UART_CLK_RATE		58820000
#define UNIPHIER_UART_DEFAULT_BAUDRATE	115200

typedef struct {
	console_t console;
	uintptr_t base;
} console_uniphier_t;

int console_uniphier_register(uintptr_t baseaddr,
			      uint32_t clock,
			      uint32_t baud,
			      console_uniphier_t *console);

static console_uniphier_t uniphier_runtime_console;

void uniphier_console_setup(void)
{
	(void) console_uniphier_register(UNIPHIER_UART_BASE,
					 UNIPHIER_UART_CLK_RATE,
					 UNIPHIER_UART_DEFAULT_BAUDRATE,
					 &uniphier_runtime_console);

	console_set_scope(&uniphier_runtime_console.console,
			  CONSOLE_FLAG_RUNTIME);
}

void uniphier_console_uninit(void)
{
	console_flush();
	console_unregister(&uniphier_runtime_console.console);
}
