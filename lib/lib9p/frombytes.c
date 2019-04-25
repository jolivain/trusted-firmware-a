/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "convfr9p.h"

long long
frombytes(unsigned char *bp, int *n, int max, int nbytes)
{
	int i;
	unsigned long long v;

	if (*n == -1)
		return 0;

	if (*n + nbytes > max) {
		*n = -1;
		return -1;
	}

	bp += *n;
	*n += nbytes;
	v = 0;
	for (i = 0; i < nbytes; i++)
		v |= bp[i] << (8 * i);

	return v;
}
