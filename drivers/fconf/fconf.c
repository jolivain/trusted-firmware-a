/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/fconf/fconf.h>
#include <platform_def.h>

void fconf_populate(void *dtb)
{
	/* go through all registered populate functions */
	IMPORT_SYM(struct fconf_populator *, __FCONF_POPULATOR_START__, start);
	IMPORT_SYM(struct fconf_populator *, __FCONF_POPULATOR_END__, end);
	struct fconf_populator *populator;

	for (populator = (struct fconf_populator *) start; populator != end; populator++) {
		assert(populator->info && populator->populate);

		INFO("FCONF: call %s populator fonction\n", populator->info);
		if (populator->populate(dtb) != 0) {
			/* TODO: handle property miss */
		}
	}
}
