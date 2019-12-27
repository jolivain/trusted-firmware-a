/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/fdt_wrappers.h>
#include <fconf_hw_config_getter.h>
#include <libfdt.h>
#include <plat/common/platform.h>

struct gicv3_config_t gicv3_config;
struct hw_topology_t soc_topology;

int fconf_populate_gicv3_config(uintptr_t config)
{
	int err = 0;
	int node;
	int addr[20];

	const void *hw_config_dtb = (void *)config;

	INFO("FDT: HW_CONFIG address = %p\n", hw_config_dtb);

	/* Assert the node offset point to "arm,gic-v3" compatible property */
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,gic-v3");
	if (node < 0) {
		ERROR("Unrecognized hardware configuration dtb (%d)\n", node);
		panic();
	}
	/* Locate the reg cell holding base address of GIC controller modules
	A sample reg cell array is shown here:
		reg = <0x0 0x2f000000 0 0x10000>,	// GICD
		      <0x0 0x2f100000 0 0x200000>,	// GICR
		      <0x0 0x2c000000 0 0x2000>,	// GICC
		      <0x0 0x2c010000 0 0x2000>,	// GICH
		      <0x0 0x2c02f000 0 0x2000>;	// GICV
	*/

	err = fdtw_read_array(hw_config_dtb, node, "reg", 20, &addr);
	if (err < 0) {
		ERROR("FCONF FAILED\n");
	}
	return err;
}

int fconf_populate_topology(uintptr_t config)
{
	int err = 0, cluster_count = 0, cpu_count = 0, total_cpu_count = 0;
	int node, cluster_node, core_node;
	const void *hw_config_dtb = (void *)config;

	/* Assert the node offset point to "arm,psci-1.0" compatible property */
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,psci-1.0");
	if ( node < 0) {
		ERROR("Unrecognized hardware configuration dtb (%d)\n", node);
		panic();
	}

	err = fdtw_read_cells(hw_config_dtb, node, "max-pwr-lvl", 1,
		&soc_topology.plat_max_pwr_level);
	if (err < 0) {
		ERROR("FCONF failed to read cluster-count property\n");
		return err;
	}

	/* Find the offset of the "cpus" node */
	node = fdt_path_offset(hw_config_dtb, "/cpus");
	if ( node < 0) {
		ERROR("Node 'cpus' not found in hardware configuration dtb \n");
		panic();
	}

	/* Locate the cpu-map child node */
	node = fdt_subnode_offset(hw_config_dtb, node, "cpu-map");
	if ( node < 0) {
		ERROR("Node 'cpu-map' not found in hardware configuration dtb \n");
		panic();
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

	int cpus_per_cluster[PLAT_ARM_CLUSTER_COUNT] = {0};

	/* Iterate through cluster nodes */
	fdt_for_each_subnode(cluster_node, hw_config_dtb, node) {
		/* Iterate through core nodes */
		fdt_for_each_subnode(core_node, hw_config_dtb, cluster_node) {
			cpus_per_cluster[cluster_count]++;
		}
		INFO("CLUSTER ID: %d cpu-count: %d\n", cluster_count, cpus_per_cluster[cluster_count]);

		/* Find the maximum number of cpus in any cluster */
		if (cpu_count < cpus_per_cluster[cluster_count]) {
			cpu_count = cpus_per_cluster[cluster_count];
		}
		cluster_count++;
		total_cpu_count += cpus_per_cluster[cluster_count];
	}

	soc_topology.plat_cluster_count = cluster_count;
	soc_topology.cluster_cpu_count = cpu_count;
	soc_topology.plat_cpu_count = total_cpu_count;

	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, gicv3_config, fconf_populate_gicv3_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, topology, fconf_populate_topology);
