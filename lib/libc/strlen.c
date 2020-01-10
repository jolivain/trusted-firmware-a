/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#pragma coverity compliance \
		(deviate "MISRA C-2012 Rule 21.2" "Reserved keyword or identifier")
size_t strlen(const char *s)
{
	const char *cursor = s;

	while (*cursor)
		cursor++;

	return cursor - s;
}
