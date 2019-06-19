/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#undef memrchr

void *memrchr(const void *s, int c, size_t n)
{
	const unsigned char *p;
	unsigned char delim = (unsigned char)c;

	for (p = s + (n - 1UL); *p != delim; --p) {
		n--;
		if (n == 0UL) {
			break;
		}
	}

	return (n == 0UL) ? NULL : (void *)p;
}
