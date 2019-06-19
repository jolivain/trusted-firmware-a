/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#undef memrchr

void *memrchr(const void *s, int c, size_t n)
{
	unsigned char *p = (unsigned char*)s;

	for (p += n - 1; (n != 0UL) && (*p != c); --p) {
		n--;
	}

	return (n == 0UL) ? NULL : (void *)p;
}
