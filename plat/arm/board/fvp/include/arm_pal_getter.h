/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PAL_GETTER_H
#define PAL_GETTER_H

#include <drivers/pal/pal.h>

/* ARM io policies */
#define arm__io_policies_getter(id)	&policies[id]

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

extern struct plat_io_policy policies[];
int pal_populate_arm_io_policies(void *dtb);

#endif /* PAL_GETTER_H */
