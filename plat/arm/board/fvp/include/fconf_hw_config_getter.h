/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_HW_CONFIG_GETTER_H
#define FCONF_HW_CONFIG_GETTER_H

#include <lib/fconf/fconf.h>

/* Hardware Config related getter */
#define hw_config__gicv3_config_getter(prop) gicv3_config.prop

#define hw_config__topology_getter(prop) soc_topology.prop

struct gicv3_config_t {
	void *gicd_base;
	void *gicr_base;
};

struct hw_topology_t {
	int plat_cluster_count;
	int cluster_cpu_count;
	int plat_cpu_count;
	int plat_max_pwr_level;
};

int fconf_populate_gicv3_config(uintptr_t config);

extern struct gicv3_config_t gicv3_config;
extern struct hw_topology_t soc_topology;

#endif /* FCONF_HW_CONFIG_GETTER_H */
