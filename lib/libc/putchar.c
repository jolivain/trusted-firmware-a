/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include <drivers/console.h>

#pragma coverity compliance \
		(deviate "MISRA C-2012 Rule 21.2" "Reserved keyword or identifier")
int putchar(int c)
{
	int res;
	if (console_putc((unsigned char)c) >= 0)
		res = c;
	else
		res = EOF;

	return res;
}
