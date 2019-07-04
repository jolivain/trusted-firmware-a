/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <plat/arm/common/plat_arm.h>

#pragma weak plat_arm_error_handler

/*
 * ARM common implementation for error handler
 */
void __dead2 arm_error_handler()
{
	for (;;)
		wfi();
}

void __dead2 plat_error_handler(int err)
{
	plat_arm_error_handler(err);
}
