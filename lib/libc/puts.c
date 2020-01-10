/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#pragma coverity compliance \
		(deviate "MISRA C-2012 Rule 21.2" "Reserved keyword or identifier")
int puts(const char *s)
{
	int count = 0;

	while (*s != '\0') {
		if (putchar(*s) == EOF)
			return EOF;
		s++;
		count++;
	}

	if (putchar('\n') == EOF)
		return EOF;

	return count + 1;
}
