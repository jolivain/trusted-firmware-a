/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/pal/pal.h>

void pal_populate(void *dtb)
{
	// go through all registered populate functions
	extern struct pal_populator *__PAL_POPULATOR_START__;
	extern struct pal_populator *__PAL_POPULATOR_END__;
	 struct pal_populator *start = (struct pal_populator *) &__PAL_POPULATOR_START__;
	struct pal_populator *end = (struct pal_populator *) &__PAL_POPULATOR_END__;
	struct pal_populator *populator;

	for (populator = start; populator != end; populator++) {
		INFO("PAL: call %s populator fonction\n", populator->info);
		if (populator->populate(dtb) != 0) {
			// TODO: handle property miss
		}
	}
}
