/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <string.h>

#include "convto9p.h"

unsigned char *
tostring(unsigned char *p, char *s)
{
	int len = strlen(s);

	p = tobytes(p, len, 2);
	memcpy(p, s, len);
	return p + len;
}
