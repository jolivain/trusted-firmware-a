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

int fconf_populate_gicv3_config(uintptr_t config)
{
	int err, node;
	uint32_t interrupt[INTERRUPT_CELL_CNT];
	uintptr_t addr, size;

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

	/*
	 * The GICv3 DT binding holds at least two address/size pairs,
	 * the first describing the distributor, the second the redistributors.
	 * See: bindings/interrupt-controller/arm,gic-v3.yaml
	 */
	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 0, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICD reg property of GIC node\n");
		return err;
	}
	gicv3_config.gicd_base = addr;

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 0, NULL, &size);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICD reg property of GIC node\n");
	} else {
		gicv3_config.gicd_size = size;
	}

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 1, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICR reg property of GIC node\n");
	} else {
		gicv3_config.gicr_base = addr;
	}

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 1, NULL, &size);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICR reg property of GIC node\n");
	} else {
		gicv3_config.gicr_size = size;
	}

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 2, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICC reg property of GIC node\n");
	} else {
		gicv3_config.gicc_base = addr;
	}

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 2, NULL, &size);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICC reg property of GIC node\n");
	} else {
		gicv3_config.gicc_size = size;
	}

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 3, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICH reg property of GIC node\n");
	} else {
		gicv3_config.gich_base = addr;
	}

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 3, NULL, &size);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICH reg property of GIC node\n");
	} else {
		gicv3_config.gich_size = size;
	}

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 4, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICV reg property of GIC node\n");
	} else {
		gicv3_config.gicv_base = addr;
	}

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 4, NULL, &size);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICV reg property of GIC node\n");
	} else {
		gicv3_config.gicv_size = size;
	}

	/*
	 * Locate the interrupts cell holding cells needed to encode an interrupt source.
	 *
	 * A sample interrupts cell array is shown here: interrupts = <1 9 4>;
	 *
	 * The 1st cell is the interrupt type; 0 for SPI interrupts, 1 for PPI interrupts.
	 * The 2nd cell contains the interrupt number for the interrupt type. SPI interrupts
	 * are in the range [0-987]. PPI interrupts are in the range [0-15]. The 3rd cell is
	 * the flags, encoded as follows: bits[3:0] trigger type and level flags. 1 for
	 * edge triggered and 4 for level triggered.
	 *
	 * In this case, #interrupt-cells = 3. The 1st cell is 1 for PPI interrupts. The 2nd
	 * cell is 9 for the PPI interrupt type. The 3rd cell is 4 for level triggered.
	 */
	err = fdt_read_uint32_array((void *)hw_config_dtb, node, "interrupts",
			INTERRUPT_CELL_CNT, interrupt);
	if (err < 0) {
		ERROR("FCONF: Failed to read interrupts property of GIC node\n");
		return err;
	}

	gicv3_config.gicv3_intr_config.interrupt_type = interrupt[INTERRUPT_TYPE_CELL];
	gicv3_config.gicv3_intr_config.interrupt_num = interrupt[INTERRUPT_NUM_CELL];
	gicv3_config.gicv3_intr_config.interrupt_flags = interrupt[INTERRUPT_FLAGS_CELL];

	return 0;
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

FCONF_REGISTER_POPULATOR(HW_CONFIG, gicv3_config, fconf_populate_gicv3_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, topology, fconf_populate_topology);
