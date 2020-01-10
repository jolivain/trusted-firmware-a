/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#pragma coverity compliance block \
		(deviate "MISRA C-2012 Rule 21.2" "Reserved keyword or identifier")

static void (*exitfun)(void);

void exit(int status)
{
	if (exitfun != NULL)
		(*exitfun)();
	for (;;)
		;
}

int atexit(void (*fun)(void))
{
	if (exitfun != NULL)
		return -1;
	exitfun = fun;

	return 0;
}

#pragma coverity compliance end_block "MISRA C-2012 Rule 21.2"
