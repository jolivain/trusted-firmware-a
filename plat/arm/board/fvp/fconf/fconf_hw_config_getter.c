/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <fconf_hw_config_getter.h>
#include <fconf_hw_config_helpers.h>
#include <libfdt.h>
#include <plat/common/platform.h>

struct gicv3_config_t gicv3_config;
struct hw_topology_t soc_topology;
struct uart_serial_config_t uart_serial_config;

int fconf_populate_gicv3_config(uintptr_t config)
{
	int err;
	int node;
	int addr[20];

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
	/* Read the reg cell holding base address of GIC controller modules
	A sample reg cell array is shown here:
		reg = <0x0 0x2f000000 0 0x10000>,	// GICD
		      <0x0 0x2f100000 0 0x200000>,	// GICR
		      <0x0 0x2c000000 0 0x2000>,	// GICC
		      <0x0 0x2c010000 0 0x2000>,	// GICH
		      <0x0 0x2c02f000 0 0x2000>;	// GICV
	*/

	err = fdtw_read_array(hw_config_dtb, node, "reg", 20, &addr);
	if (err < 0) {
		ERROR("FCONF: Failed to read reg property of GIC node\n");
	}
	return err;
}

int fconf_populate_topology(uintptr_t config)
{
	int err, node, cluster_node, core_node, thread_node, max_pwr_lvl = 0;
	uint32_t cluster_count = 0, max_cpu_per_cluster = 0, total_cpu_count = 0;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/* Find the offset of the node containing "arm,psci-1.0" compatible property */
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,psci-1.0");
	if (node < 0) {
		ERROR("FCONF: Unable to locate node with arm,psci-1.0 compatible property\n");
		return node;
	}

	err = fdtw_read_cells(hw_config_dtb, node, "max-pwr-lvl", 1, &max_pwr_lvl);
	if (err < 0) {
		/*
		 * Some legacy FVP dts may not have this property. Assign the default
		 * value.
		 */
		WARN("FCONF: Could not locate max-pwr-lvl property\n");
		max_pwr_lvl = 2;
	}

	assert((uint32_t)max_pwr_lvl <= MPIDR_AFFLVL2);

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

		VERBOSE("CLUSTER ID: %d cpu-count: %d\n", cluster_count, cpus_per_cluster[cluster_count]);

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

	soc_topology.plat_max_pwr_level = (uint32_t)max_pwr_lvl;
	soc_topology.plat_cluster_count = cluster_count;
	soc_topology.cluster_cpu_count = max_cpu_per_cluster;
	soc_topology.plat_cpu_count = total_cpu_count;

	return 0;
}

int fconf_populate_uart_config(uintptr_t config)
{
	int node, err, parent_node, addr[2];
	const char *path;
	const uint32_t *phandle, *clk_freq;
	uint64_t base_addr = 0, offset_addr;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/*
	 * uart child node is indirectly referenced through its path which is specified
	 * in the `serial0` property of the "aliases" node.
	 */

	/* Find the offset of the "aliases" node */
	node = fdt_path_offset(hw_config_dtb, "/aliases");
	if (node < 0) {
		ERROR("FCONF: Node '%s' not found in hardware configuration dtb\n", "aliases");
		return node;
	}

	path = fdt_getprop(hw_config_dtb, node, "serial0", NULL);
	if (path == NULL) {
		ERROR("FCONF: Could not read serial0 property in aliases node\n");
		return -1;
	}

	node = fdt_path_offset(hw_config_dtb, path);
	if (node < 0) {
		ERROR("FCONF: Failed to locate uart serial node using its path");
		return -1;
	}

	err = fdtw_read_array(hw_config_dtb, node, "reg", 2, &addr);
	if (err < 0) {
		ERROR("FCONF: Failed to read reg property of '%s' node\n", "uart serial");
		return err;
	}
	VERBOSE("FCONF: UART node address offset: %d size: %d\n", addr[0], addr[1]);

	/* uart serial node has its offset and size of address in reg property */
	offset_addr = (uint64_t)addr[0];

	/* Only the devices which are direct children of root node use CPU address domain. All
	 * other devices use addresses that are local to the device node and cannot be directly
	 * used by CPU. Device tree provides an address translation mechanism through "ranges"
	 * property which provides mappings from local address space to parent address space.
	 * Since a device could be a child of a child node to the root node, there can be more
	 * than one level of address translation needed to map the device local address space
	 * to CPU address space.
	 */
	parent_node = fdt_parent_offset(hw_config_dtb, node);
	base_addr = translate_address(hw_config_dtb, parent_node, base_addr);
	uart_serial_config.uart_base = base_addr + offset_addr;
	VERBOSE("FCONF: UART serial device base address: %llx\n", uart_serial_config.uart_base);

	/* The device tree node which captures the clock information of uart serial node is
	 * specified in the "clocks" property.
	 */
	phandle = fdt_getprop(hw_config_dtb, node, "clocks", NULL);
	if (phandle == NULL) {
		ERROR("FCONF: Could not read clocks property in uart serial node\n");
	}

	node = fdt_node_offset_by_phandle(hw_config_dtb, fdt32_to_cpu(*phandle));
	if (node < 0) {
		ERROR("FCONF: Failed to locate clk node using its path\n");
		return node;
	}

	clk_freq = fdt_getprop(hw_config_dtb, node, "clock-frequency", NULL);
	if (clk_freq == NULL) {
		ERROR("FCONF: Could not read clock-frequency property in clk node\n");
		return -1;
	}

	uart_serial_config.uart_clk = fdt32_to_cpu(*clk_freq);
	VERBOSE("FCONF: UART serial device clk frequency: %x\n", uart_serial_config.uart_clk);
	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, gicv3_config, fconf_populate_gicv3_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, topology, fconf_populate_topology);
FCONF_REGISTER_POPULATOR(HW_CONFIG, uart_config, fconf_populate_uart_config);
