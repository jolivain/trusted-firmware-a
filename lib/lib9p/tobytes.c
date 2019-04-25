/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "convto9p.h"

unsigned char *
tobytes(unsigned char *p, unsigned long long v, int n)
{
	for ( ; n--; p++)
		*p = v, v >>= 8;
	return p;
}
