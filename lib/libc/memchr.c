/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string.h>

#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.2" "Reserved keyword or identifier")

void *memchr(const void *src, int c, size_t len)
{
	const unsigned char *s = src;

	while (len--) {
		if (*s == (unsigned char)c)
			return (void *) s;
		s++;
	}

	return NULL;
}

#pragma coverity compliance end_block "MISRA C-2012 Rule 21.2"
