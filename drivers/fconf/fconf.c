/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/fconf/fconf.h>
#include <platform_def.h>

void fconf_populate(uintptr_t config)
{
	assert(config == 0UL);

	/* Check if the pointer to DTB is correct */
	if (fdt_check_header((void *)config) != 0) {
		WARN("FCONF: Invalid DTB file passed for FW_CONFIG \n");
		panic();
	}

	INFO("FCONF: populate from: 0x%lx\n", config);

	/* go through all registered populate functions */
	IMPORT_SYM(const struct fconf_populator *, __FCONF_POPULATOR_START__, start);
	IMPORT_SYM(const struct fconf_populator *, __FCONF_POPULATOR_END__, end);
	const struct fconf_populator *populator;

	for (populator = start; populator != end; populator++) {
		assert((populator->info != NULL) && (populator->populate != NULL));

		INFO("FCONF: call %s populator function\n", populator->info);
		if (populator->populate(config) != 0) {
			/* TODO: handle property miss */
		}
	}
}
