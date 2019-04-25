/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <string.h>

#include "convfr9p.h"

char *
fromstring(unsigned char *p, int *n, int max, char *s, int size)
{
	int len;

	if (*n == -1)
		goto error;

	len = frombytes(p, n, max, 2);
	if (len == -1)
		goto error;

	if (*n + len > max)
		goto error;

	if (len >= size)
		goto error;

	if (s) {
		memcpy(s, p + *n, len);
		s[len] = '\0';
	}
	*n += len;

	return s;

error:
	*n = -1;
	return NULL;
}
