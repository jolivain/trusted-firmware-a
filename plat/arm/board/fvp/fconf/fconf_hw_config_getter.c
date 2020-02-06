/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <fconf_hw_config_getter.h>
#include <libfdt.h>
#include <plat/common/platform.h>

struct gicv3_config_t gicv3_config;
struct hw_topology_t soc_topology;
struct cpu_timer_t cpu_timer;
struct mm_timer_t mm_timer;

int fconf_populate_gicv3_config(uintptr_t config)
{
	int err;
	int node;
	uintptr_t addr;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/*
	 * Find the offset of the node containing "arm,gic-v3" compatible property.
	 * Populating fconf strucutures dynamically is not supported for legacy
	 * systems which use GICv2 IP. Simply skip extracting GIC properties.
	 */
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,gic-v3");
	if (node < 0) {
		WARN("FCONF: Unable to locate node with arm,gic-v3 compatible property\n");
		return 0;
	}
	/* The GICv3 DT binding holds at least two address/size pairs,
	 * the first describing the distributor, the second the redistributors.
	 * See: bindings/interrupt-controller/arm,gic-v3.yaml
	 */
	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 0, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICD reg property of GIC node\n");
		return err;
	}
	gicv3_config.gicd_base = addr;

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 1, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICR reg property of GIC node\n");
	} else {
		gicv3_config.gicr_base = addr;
	}

	return err;
}

int fconf_populate_topology(uintptr_t config)
{
	int err, node, cluster_node, core_node, thread_node;
	uint32_t cluster_count = 0, max_cpu_per_cluster = 0, total_cpu_count = 0;
	uint32_t max_pwr_lvl = 0;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/* Find the offset of the node containing "arm,psci-1.0" compatible property */
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,psci-1.0");
	if (node < 0) {
		ERROR("FCONF: Unable to locate node with arm,psci-1.0 compatible property\n");
		return node;
	}

	err = fdt_read_uint32(hw_config_dtb, node, "max-pwr-lvl", &max_pwr_lvl);
	if (err < 0) {
		/*
		 * Some legacy FVP dts may not have this property. Assign the default
		 * value.
		 */
		WARN("FCONF: Could not locate max-pwr-lvl property\n");
		max_pwr_lvl = 2;
	}

	assert(max_pwr_lvl <= MPIDR_AFFLVL2);

	/* Find the offset of the "cpus" node */
	node = fdt_path_offset(hw_config_dtb, "/cpus");
	if (node < 0) {
		ERROR("FCONF: Node '%s' not found in hardware configuration dtb\n", "cpus");
		return node;
	}

	/* A typical cpu-map node in a device tree is shown here for reference
	cpu-map {
		cluster0 {
			core0 {
				cpu = <&CPU0>;
			};
			core1 {
				cpu = <&CPU1>;
			};
		};

		cluster1 {
			core0 {
				cpu = <&CPU2>;
			};
			core1 {
				cpu = <&CPU3>;
			};
		};
	};
	*/

	/* Locate the cpu-map child node */
	node = fdt_subnode_offset(hw_config_dtb, node, "cpu-map");
	if (node < 0) {
		ERROR("FCONF: Node '%s' not found in hardware configuration dtb\n", "cpu-map");
		return node;
	}

	uint32_t cpus_per_cluster[PLAT_ARM_CLUSTER_COUNT] = {0};

	/* Iterate through cluster nodes */
	fdt_for_each_subnode(cluster_node, hw_config_dtb, node) {
		assert(cluster_count < PLAT_ARM_CLUSTER_COUNT);

		/* Iterate through core nodes */
		fdt_for_each_subnode(core_node, hw_config_dtb, cluster_node) {
			/* core nodes may have child nodes i.e., "thread" nodes */
			if (fdt_first_subnode(hw_config_dtb, core_node) < 0) {
				cpus_per_cluster[cluster_count]++;
			} else {
				/* Multi-threaded CPU description is found in dtb */
				fdt_for_each_subnode(thread_node, hw_config_dtb, core_node) {
					cpus_per_cluster[cluster_count]++;
				}

				/* Since in some dtbs, core nodes may not have thread node,
				 * no need to error if even one child node is not found.
				 */
			}
		}

		/* Ensure every cluster node has at least 1 child node */
		if (cpus_per_cluster[cluster_count] < 1U) {
			ERROR("FCONF: Unable to locate the core node in cluster %d\n", cluster_count);
			return -1;
		}

		INFO("CLUSTER ID: %d cpu-count: %d\n", cluster_count, cpus_per_cluster[cluster_count]);

		/* Find the maximum number of cpus in any cluster */
		max_cpu_per_cluster = MAX(max_cpu_per_cluster, cpus_per_cluster[cluster_count]);
		total_cpu_count += cpus_per_cluster[cluster_count];
		cluster_count++;
	}


	/* At least one cluster node is expected in hardware configuration dtb */
	if (cluster_count < 1U) {
		ERROR("FCONF: Unable to locate the cluster node in cpu-map node\n");
		return -1;
	}

	soc_topology.plat_max_pwr_level = max_pwr_lvl;
	soc_topology.plat_cluster_count = cluster_count;
	soc_topology.cluster_cpu_count = max_cpu_per_cluster;
	soc_topology.plat_cpu_count = total_cpu_count;

	return 0;
}

int fconf_populate_cpu_timer(uintptr_t config)
{
	int err = 0, node;
	uint32_t interrupts[3];

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/* Find the node offset point to "arm,armv8-timer" compatible property,
	 * a per-core architected timer attached to a GIC to deliver its per-processor
	 * interrupts via PPIs */
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,armv8-timer");
	if (node < 0) {
		ERROR("FCONF: Unrecognized hardware configuration dtb (%d)\n", node);
		return node;
	}

	/* Locate the interrupts cell holding the interrupt list for secure, non-secure,
	 * virtual and hypervisor timers, in that order. Find the secure timer interrupt.

	A sample interrupts cell array is shown here:
		interrupts = <1 13 0xff01>,	//Secure Timer
			     <1 14 0xff01>,	//Non-Secure Timer
			     <1 11 0xff01>,	//Virtual Timer
			     <1 10 0xff01>;	//Hypervisor Timer
	 */
	err = fdt_read_uint32_array(hw_config_dtb, node, "interrupts", 3, interrupts);
	if (err < 0) {
		ERROR("FCONF: Failed to read interrupts property of CPU Timer node\n");
		return err;
	}
	cpu_timer.cputimer_intr_config.interrupt_type = interrupts[0];
	cpu_timer.cputimer_intr_config.interrupt_num = interrupts[1];
	cpu_timer.cputimer_intr_config.interrupt_flags = interrupts[2];

	/* Locate the cell holding the clock-frequency, an optional field */
	err = fdt_read_uint32(hw_config_dtb, node, "clock-frequency", &cpu_timer.clock_freq);
	if (err < 0) {
		WARN("FCONF failed to read clock-frequency property\n");
	}

	return 0;
}

int fconf_populate_mm_timer(uintptr_t config)
{
	int err = 0, node;
	uintptr_t addr;
	uint32_t interrupts[3];

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/* Assert the node offset point to "arm,armv7-timer-mem" compatible property,
	 * a memory mapped architected time attached to a GIC to deliver its interrupts
	 * via SPIs*/
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,armv7-timer-mem");
	if (node < 0) {
		ERROR("FCONF: Unrecognized hardware configuration dtb (%d)\n", node);
		return node;
	}

	/* Locate the reg cell holding the control frame base address */
	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 0, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read reg property of Mem Timer node\n");
		return err;
	}
	mm_timer.cframe_base = addr;
	INFO("FCONF TEST mm_timer.cframe_base=%llx\n", mm_timer.cframe_base);

	/* Locate the cell holding the clock-frequency, an optional field */
	err = fdt_read_uint32(hw_config_dtb, node, "clock-frequency", &mm_timer.clock_freq);
	if (err < 0) {
		WARN("FCONF failed to read clock-frequency property\n");
	}

	/* Assert the node offset point to subnode frame property */
	node = fdt_subnode_offset_namelen(hw_config_dtb, node, "frame", 5);
	if (node < 0) {
		ERROR("FCONF: Unrecognized hardware configuration dtb (%d)\n", node);
		return node;
	}

	/* Locate the cell holding the frame-number */
	err = fdt_read_uint32(hw_config_dtb, node, "frame-number", &mm_timer.frame_num);
	if (err < 0) {
		ERROR("FCONF failed to read frame-number property\n");
		return err;
	}

	/* Locate the interrupts cell holding the interrupt list for physical timer*/
	err = fdt_read_uint32_array(hw_config_dtb, node, "interrupts", 3, interrupts);
	if (err < 0) {
		ERROR("FCONF failed\n");
		return err;
	}
	mm_timer.mmtimer_intr_config.interrupt_type = interrupts[0];
	mm_timer.mmtimer_intr_config.interrupt_num = interrupts[1];
	mm_timer.mmtimer_intr_config.interrupt_flags = interrupts[2];

	/* Locate the reg cell holding the first view base address */
	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 0, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF failed\n");
		return err;
	}
	mm_timer.fframe_base = addr;
	INFO("FCONF TEST mm_timer.fframe_base=%llx\n", mm_timer.fframe_base);

	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, gicv3_config, fconf_populate_gicv3_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, topology, fconf_populate_topology);
FCONF_REGISTER_POPULATOR(HW_CONFIG, cpu_timer, fconf_populate_cpu_timer);
FCONF_REGISTER_POPULATOR(HW_CONFIG, mm_timer, fconf_populate_mm_timer);
