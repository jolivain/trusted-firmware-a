/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PAL_H
#define PAL_H

/* Public API */
#define PAL_GET_PROPERTY(a, b, c)	a##__##b##_getter(c)

#define PAL_REGISTER_POPULATOR(name, callback)					\
	__attribute__((used, section(".pal_populator")))				\
	const struct pal_populator name##__populator = {			\
		.info = #name,							\
		.populate = callback						\
	};

struct pal_populator {
	const char *info;
	int (*populate)(void *dtb);
};

void pal_load_config(void);
void pal_populate(void *dtb);

#endif /* PAL_H */
