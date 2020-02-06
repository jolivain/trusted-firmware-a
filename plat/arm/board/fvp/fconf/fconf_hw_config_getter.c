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
	int err, node, addr[20];

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

	To parse the property we need to understand three things:
		1. Each cell value is a 32-bit unsigned integer
		2. The parent node's #address-cells value defines how many cell
		values constitute a region's base address
		3. The parent node's #size-cells value defines how many cell values
		constitute a region's offset

	In this case, #address-cells = #size-cells = <2>. Therefore combining the
	first 2 cells for a base address and the next 2 cells for a offset, and so
	on, gives the following regions that the interrupt controller will respond
	to:
		GICD: 64-bit address 0x0000_0000_2F00_0000 with 64-bit offset 0x0000_0000_0001_0000
		GICR: 64-bit address 0x0000_0000_2F10_0000 with 64-bit offset 0x0000_0000_0020_0000
		GICC: 64-bit address 0x0000_0000_2C00_0000 with 64-bit offset 0x0000_0000_0000_2000
		GICH: 64-bit address 0x0000_0000_2C01_0000 with 64-bit offset 0x0000_0000_0000_2000
		GICV: 64-bit address 0x0000_0000_2C02_F000 with 64-bit offset 0x0000_0000_0000_2000
	*/
	err = fdtw_read_array(hw_config_dtb, node, "reg", 20, &addr);
	if (err < 0) {
		ERROR("FCONF: Failed to read reg property of GIC node\n");
		return err;
	}

	gicv3_config.gicd_base = ((uint64_t)addr[0] << 32) | addr[1];
	gicv3_config.gicd_offset = ((uint64_t)addr[2] << 32) | addr[3];

	gicv3_config.gicr_base = ((uint64_t)addr[4] << 32) | addr[5];
	gicv3_config.gicr_offset = ((uint64_t)addr[6] << 32) | addr[7];

	gicv3_config.gicc_base = ((uint64_t)addr[8] << 32) | addr[9];
	gicv3_config.gicc_offset = ((uint64_t)addr[10] << 32) | addr[11];

	gicv3_config.gich_base = ((uint64_t)addr[12] << 32) | addr[13];
	gicv3_config.gich_offset = ((uint64_t)addr[14] << 32) | addr[15];

	gicv3_config.gicv_base = ((uint64_t)addr[16] << 32) | addr[17];
	gicv3_config.gicv_offset = ((uint64_t)addr[18] << 32) | addr[19];

	/* Locate the interrupts cell holding cells needed to encode an interrupt source
	A sample interrupts cell array is shown here:
		interrupts = <1 9 4>;

	To parse the property we need to understand four things:
		1. The #interrupt-cells cell specifies the number of cells needed to encode
		an interrupt source. It must be a single cell with a value of at least 3.
		2. The 1st cell is the interrupt type; 0 for SPI interrupts, 1 for PPI
		interrupts.
		3. The 2nd cell contains the interrupt number for the interrupt type. SPI
		interrupts are in the range [0-987]. PPI interrupts are in the range [0-15].
		4. The 3rd cell is the flags, encoded as follows: bits[3:0] trigger type
		and level flags. 1 for edge triggered and 4 for level triggered.

	In this case, #interrupt-cells = 3. The 1st cell is 1 for PPI interrupts. The 2nd
	cell is 9 for the PPI interrupt type. The 3rd cell is 4 for level triggered.
	*/
	err = fdtw_read_array((void *)hw_config_dtb, node, "interrupts", 3, &addr);
	if (err < 0) {
		ERROR("FCONF: Failed to read interrupts property of GIC node\n");
		return err;
	}

	gicv3_config.gicv3_intr_config.interrupt_type = addr[0];
	gicv3_config.gicv3_intr_config.interrupt_num = addr[1];
	gicv3_config.gicv3_intr_config.interrupt_flags = addr[2];

	return 0;
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

	soc_topology.plat_max_pwr_level = (uint32_t)max_pwr_lvl;
	soc_topology.plat_cluster_count = cluster_count;
	soc_topology.cluster_cpu_count = max_cpu_per_cluster;
	soc_topology.plat_cpu_count = total_cpu_count;

	return 0;
}

int fconf_populate_cpu_timer(uintptr_t config)
{
	int err = 0, node, addr[12];

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
	 * virtual and hypervisor timers, in that order.

	A sample interrupts cell array is shown here:
		interrupts = <1 13 0xff01>,	//Secure Timer
			     <1 14 0xff01>,	//Non-Secure Timer
			     <1 11 0xff01>,	//Virtual Timer
			     <1 10 0xff01>;	//Hypervisor Timer
	 */
	err = fdtw_read_array(hw_config_dtb, node, "interrupts", 12, &addr);
	if (err < 0) {
		ERROR("FCONF: Failed to read interrupts property of CPU Timer node\n");
		return err;
	}

	for(int i = 0; i<MAX_CPU_TIMER; i++) {
		cpu_timer.cputimer_intr_config[i].interrupt_type = addr[i*3];
		cpu_timer.cputimer_intr_config[i].interrupt_num = addr[i*3+1];
		cpu_timer.cputimer_intr_config[i].interrupt_flags = addr[i*3+2];
	}

	/* Locate the cell holding the clock-frequency */
	err = fdtw_read_cells(hw_config_dtb, node, "clock-frequency", 1,
		&cpu_timer.clock_freq);
	if (err < 0) {
		ERROR("FCONF failed to read clock-frequency property\n");
		return err;
	}

	return 0;
}

int fconf_populate_mm_timer(uintptr_t config)
{
	int err = 0, node, addr[4], interrupts[3];

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

	/* Locate the reg cell holding the control frame base address and offset */
	err = fdtw_read_array(hw_config_dtb, node, "reg", 4, &addr);
	if (err < 0) {
		ERROR("FCONF: Failed to read reg property of Mem Timer node\n");
		return err;
	}

	mm_timer.cframe_base = ((uint64_t)addr[0] << 32) | addr[1];
	mm_timer.cframe_offset = ((uint64_t)addr[2] << 32) | addr[3];

	/* Locate the cell holding the clock-frequency */
	err = fdtw_read_cells(hw_config_dtb, node, "clock-frequency", 1,
		&mm_timer.clock_freq);
	if (err < 0) {
		ERROR("FCONF failed to read clock-frequency property\n");
		return err;
	}

	/* Assert the node offset point to subnode frame property */
	node = fdt_subnode_offset_namelen(hw_config_dtb, node, "frame", 5);
	if (node < 0) {
		ERROR("FCONF: Unrecognized hardware configuration dtb (%d)\n", node);
		return node;
	}

	/* Locate the cell holding the frame-number */
	err = fdtw_read_cells(hw_config_dtb, node, "frame-number", 1,
		&mm_timer.frame_num);
	if (err < 0) {
		ERROR("FCONF failed to read frame-number property\n");
		return err;
	}

	/* Locate the interrupts cell holding the interrupt list for physical timer*/
	err = fdtw_read_array(hw_config_dtb, node, "interrupts", 3, &interrupts);
	if (err < 0) {
		ERROR("FCONF failed\n");
		return err;
	}

	mm_timer.mmtimer_intr_config.interrupt_type = interrupts[0];
	mm_timer.mmtimer_intr_config.interrupt_num = interrupts[1];
	mm_timer.mmtimer_intr_config.interrupt_flags = interrupts[2];

	/* Locate the reg cell holding the first view base address and offset */
	err = fdtw_read_array(hw_config_dtb, node, "reg", 4, &addr);
	if (err < 0) {
		ERROR("FCONF failed\n");
		return err;
	}

	mm_timer.fframe_base = ((uint64_t)addr[0] << 32) | addr[1];
	mm_timer.fframe_offset = ((uint64_t)addr[2] << 32) | addr[3];

	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, gicv3_config, fconf_populate_gicv3_config);
FCONF_REGISTER_POPULATOR(HW_CONFIG, topology, fconf_populate_topology);
FCONF_REGISTER_POPULATOR(HW_CONFIG, cpu_timer, fconf_populate_cpu_timer);
FCONF_REGISTER_POPULATOR(HW_CONFIG, mm_timer, fconf_populate_mm_timer);
