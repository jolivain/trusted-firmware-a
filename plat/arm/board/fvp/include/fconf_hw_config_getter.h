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
#define hw_config__cpu_timer_getter(prop) soc_cpu_timer.prop
#define hw_config__mm_timer_getter(prop) soc_mm_timer.prop

struct gicv3_config_t {
	uint64_t gicd_base;
	uint64_t gicd_offset;
	uint64_t gicr_base;
	uint64_t gicr_offset;
	uint64_t gicc_base;
	uint64_t gicc_offset;
	uint64_t gich_base;
	uint64_t gich_offset;
	uint64_t gicv_base;
	uint64_t gicv_offset;
	uint32_t interrupt_type;
	uint32_t interrupt_num;
	uint32_t interrupt_flags;
};

struct hw_topology_t {
	uint32_t plat_cluster_count;
	uint32_t cluster_cpu_count;
	uint32_t plat_cpu_count;
	uint32_t plat_max_pwr_level;
};

struct cpu_timer_t {
	uint32_t interrupt_stype;
	uint32_t interrupt_snum;
	uint32_t interrupt_sflags;
	uint32_t interrupt_nstype;
	uint32_t interrupt_nsnum;
	uint32_t interrupt_nsflags;
	uint32_t interrupt_vtype;
	uint32_t interrupt_vnum;
	uint32_t interrupt_vflags;
	uint32_t interrupt_htype;
	uint32_t interrupt_hnum;
	uint32_t interrupt_hflags;
	int clock_freq;
};

struct mm_timer_t {
	uint64_t cframe_base;
	uint64_t cframe_offset;
	int clock_freq;
	int frame_num;
	uint32_t pinterrupt_type;
	uint32_t pinterrupt_num;
	uint32_t pinterrupt_flags;
	uint64_t fframe_base;
	uint64_t fframe_offset;
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
