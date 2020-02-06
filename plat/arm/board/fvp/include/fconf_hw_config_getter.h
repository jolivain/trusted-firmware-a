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
#define hw_config__cpu_timer_getter(prop) arch_timer.prop
#define hw_config__mm_timer_getter(prop) memm_timer.prop

struct intr_config_t {
	uint32_t interrupt_type;
	uint32_t interrupt_num;
	uint32_t interrupt_flags;
};

struct gicv3_config_t {
	uintptr_t gicd_base;
	uintptr_t gicr_base;
};

struct hw_topology_t {
	uint32_t plat_cluster_count;
	uint32_t cluster_cpu_count;
	uint32_t plat_cpu_count;
	uint32_t plat_max_pwr_level;
};

struct cpu_timer_t {
	struct intr_config_t cputimer_intr_config;
	uint32_t clock_freq;
};

struct mm_timer_t {
	uint64_t cframe_base;
	uint32_t clock_freq;
	uint32_t frame_num;
	struct intr_config_t mmtimer_intr_config;
	uint64_t fframe_base;
};

int fconf_populate_gicv3_config(uintptr_t config);
int fconf_populate_topology(uintptr_t config);
int fconf_populate_cpu_timer(uintptr_t config);
int fconf_populate_mm_timer(uintptr_t mm_timer);

extern struct gicv3_config_t gicv3_config;
extern struct hw_topology_t soc_topology;
extern struct cpu_timer_t cpu_timer;
extern struct mm_timer_t mm_timer;

#endif /* FCONF_HW_CONFIG_GETTER_H */
