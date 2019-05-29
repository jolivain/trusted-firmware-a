/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/console.h>

#include "uniphier.h"

struct uniphier_console {
	struct console console;
	uintptr_t base;
};

int __uniphier_console_putc(int c, uintptr_t base);
int __uniphier_console_getc(uintptr_t base);
int __uniphier_console_flush(uintptr_t base);
uintptr_t uniphier_console_get_base(void);
uintptr_t uniphier_console_init(uintptr_t base);

/*
 * This relies on that 'console' is the first member of uniphier_console.
 * I wish I could use container_of() like Linux...
 */
#define to_uniphier_console(console)	((struct uniphier_console *)console)

static int uniphier_console_putc(int c, struct console *console)
{
	return __uniphier_console_putc(c, to_uniphier_console(console)->base);
}

static int uniphier_console_getc(struct console *console)
{
	return __uniphier_console_getc(to_uniphier_console(console)->base);
}

static int uniphier_console_flush(struct console *console)
{
	return __uniphier_console_flush(to_uniphier_console(console)->base);
}

static struct uniphier_console uniphier_console = {
	.console = {
		.flags = CONSOLE_FLAG_BOOT |
#ifdef DEBUG
			 CONSOLE_FLAG_RUNTIME |
#endif
			 CONSOLE_FLAG_CRASH,
		.putc = uniphier_console_putc,
		.getc = uniphier_console_getc,
		.flush = uniphier_console_flush,
	},
};

void uniphier_console_setup(void)
{
	uniphier_console.base = uniphier_console_get_base();

	uniphier_console_init(uniphier_console.base);

	console_register(&uniphier_console.console);
}
