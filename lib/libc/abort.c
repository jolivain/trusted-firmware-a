/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include <common/debug.h>

#pragma coverity compliance \
	(deviate "MISRA C-2012 Rule 21.2" "abort reserved") \
	(deviate "MISRA C-2012 Rule 21.8" "abort reserved")
void abort(void)
{
	ERROR("ABORT\n");
	panic();
}
