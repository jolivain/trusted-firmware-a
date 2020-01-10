/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string.h>

#pragma coverity compliance \
		(deviate "MISRA C-2012 Rule 21.2" "Reserved keyword or identifier")
void *memset(void *dst, int val, size_t count)
{
	char *ptr = dst;

	while (count--)
		*ptr++ = val;

	return dst;
}
