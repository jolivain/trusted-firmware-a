/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <string.h>
#undef strcpy

char *
strcpy(char * dst, const char * src)
{
	char *ret = dst;

	while ( (*dst++ = *src++) != '\0' )
		;
	return ret;
}
