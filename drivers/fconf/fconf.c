/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/fconf/fconf.h>

void fconf_populate(void *dtb)
{
	/* go through all registered populate functions */
	extern struct fconf_populator *__FCONF_POPULATOR_START__;
	extern struct fconf_populator *__FCONF_POPULATOR_END__;
	struct fconf_populator *start = (struct fconf_populator *) &__FCONF_POPULATOR_START__;
	struct fconf_populator *end = (struct fconf_populator *) &__FCONF_POPULATOR_END__;
	struct fconf_populator *populator;

	for (populator = start; populator != end; populator++) {
		INFO("FCONF: call %s populator fonction\n", populator->info);
		if (populator->populate(dtb) != 0) {
			/* TODO: handle property miss */
		}
	}
}
