/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_HW_CONFIG_GETTER_H
#define FCONF_HW_CONFIG_GETTER_H

#include <lib/fconf/fconf.h>

/* Hardware Config related getter */
#define hw_config__gicv3_config_getter(prop) gicv3_config.prop
#define hw_config__topology_getter(prop) soc_topology.prop

#define INTERRUPT_CELL_CNT		3
#define INTERRUPT_TYPE_CELL		0
#define INTERRUPT_NUM_CELL		1
#define INTERRUPT_FLAGS_CELL		2

struct intr_config_t {
	uint32_t interrupt_type;
	uint32_t interrupt_num;
	uint32_t interrupt_flags;
};

struct gicv3_config_t {
	uint64_t gicd_base;
	uint64_t gicd_size;
	uint64_t gicr_base;
	uint64_t gicr_size;
	uint64_t gicc_base;
	uint64_t gicc_size;
	uint64_t gich_base;
	uint64_t gich_size;
	uint64_t gicv_base;
	uint64_t gicv_size;
	struct intr_config_t gicv3_intr_config;
};

struct hw_topology_t {
	uint32_t plat_cluster_count;
	uint32_t cluster_cpu_count;
	uint32_t plat_cpu_count;
	uint32_t plat_max_pwr_level;
};

int fconf_populate_gicv3_config(uintptr_t config);
int fconf_populate_topology(uintptr_t config);

extern struct gicv3_config_t gicv3_config;
extern struct hw_topology_t soc_topology;

#endif /* FCONF_HW_CONFIG_GETTER_H */
