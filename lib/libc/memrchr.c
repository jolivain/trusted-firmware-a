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
	size_t index = n;

	for (p = s + (index - 1UL); *p != delim; --p) {
		if (index == 0UL) {
			break;
		}
		index--;
	}

	return (index == 0UL) ? NULL : (void *)p;
}
