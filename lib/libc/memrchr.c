/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.2" "Reserved keyword/identifier")

#pragma coverity compliance \
		(deviate "MISRA C-2012 Rule 20.5" "undef") \
		(deviate "MISRA C-2012 Rule 21.1" "undef")
#undef memrchr

void *memrchr(const void *src, int c, size_t len)
{
	const unsigned char *s = src + (len - 1);

	while (len--) {
		if (*s == (unsigned char)c) {
			return (void*) s;
		}

		s--;
	}

	return NULL;
}
