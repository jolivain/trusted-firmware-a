/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_H
#define FCONF_H

/* Public API */
#define FCONF_GET_PROPERTY(a, b, c)	a##__##b##_getter(c)

#define FCONF_REGISTER_POPULATOR(name, callback)					\
	__attribute__((used, section(".fconf_populator")))				\
	const struct fconf_populator name##__populator = {			\
		.info = #name,							\
		.populate = callback						\
	};

struct fconf_populator {
	const char *info;
	int (*populate)(void *dtb);
};

void fconf_load_config(void);
void fconf_populate(void *dtb);

/* FCONF specific getter */
#define fconf__dtb_getter(prop)	fconf_dtb_info.prop

struct fconf_dtb_info_t {
	void *base_addr;
	size_t size;
};

extern struct fconf_dtb_info_t fconf_dtb_info;

#endif /* FCONF_H */
