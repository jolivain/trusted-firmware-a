/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#undef memrchr

void *memrchr(const void *s, int c, size_t n)
{
	const unsigned char *p = s;
	unsigned char delim = (unsigned char)c;

	for (p += n - 1UL; (n != 0UL) && (*p != delim); --p) {
		n--;
	}

	return (n == 0UL) ? NULL : (void *)p;
}
