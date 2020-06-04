/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Contains generic routines to fix up the device tree blob passed on to
 * payloads like BL32 and BL33 (and further down the boot chain).
 * This allows to easily add PSCI nodes, when the original DT does not have
 * it or advertises another method.
 * Also it supports to add reserved memory nodes to describe memory that
 * is used by the secure world, so that non-secure software avoids using
 * that.
 */

#include <stdio.h>
#include <string.h>

#include <libfdt.h>

#include <arch.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <lib/psci/psci.h>

#include <common/fdt_fixup.h>

static int append_psci_compatible(void *fdt, int offs, const char *str)
{
	return fdt_appendprop(fdt, offs, "compatible", str, strlen(str) + 1);
}

/*
 * Those defines are for PSCI v0.1 legacy clients, which we expect to use
 * the same execution state (AArch32/AArch64) as TF-A.
 * Kernels running in AArch32 on an AArch64 TF-A should use PSCI v0.2.
 */
#ifdef __aarch64__
#define PSCI_CPU_SUSPEND_FNID	PSCI_CPU_SUSPEND_AARCH64
#define PSCI_CPU_ON_FNID	PSCI_CPU_ON_AARCH64
#else
#define PSCI_CPU_SUSPEND_FNID	PSCI_CPU_SUSPEND_AARCH32
#define PSCI_CPU_ON_FNID	PSCI_CPU_ON_AARCH32
#endif

/*******************************************************************************
 * dt_add_psci_node() - Add a PSCI node into an existing device tree
 * @fdt:	pointer to the device tree blob in memory
 *
 * Add a device tree node describing PSCI into the root level of an existing
 * device tree blob in memory.
 * This will add v0.1, v0.2 and v1.0 compatible strings and the standard
 * function IDs for v0.1 compatibility.
 * An existing PSCI node will not be touched, the function will return success
 * in this case. This function will not touch the /cpus enable methods, use
 * dt_add_psci_cpu_enable_methods() for that.
 *
 * Return: 0 on success, -1 otherwise.
 ******************************************************************************/
int dt_add_psci_node(void *fdt)
{
	int offs;

	if (fdt_path_offset(fdt, "/psci") >= 0) {
		WARN("PSCI Device Tree node already exists!\n");
		return 0;
	}

	offs = fdt_path_offset(fdt, "/");
	if (offs < 0)
		return -1;
	offs = fdt_add_subnode(fdt, offs, "psci");
	if (offs < 0)
		return -1;
	if (append_psci_compatible(fdt, offs, "arm,psci-1.0"))
		return -1;
	if (append_psci_compatible(fdt, offs, "arm,psci-0.2"))
		return -1;
	if (append_psci_compatible(fdt, offs, "arm,psci"))
		return -1;
	if (fdt_setprop_string(fdt, offs, "method", "smc"))
		return -1;
	if (fdt_setprop_u32(fdt, offs, "cpu_suspend", PSCI_CPU_SUSPEND_FNID))
		return -1;
	if (fdt_setprop_u32(fdt, offs, "cpu_off", PSCI_CPU_OFF))
		return -1;
	if (fdt_setprop_u32(fdt, offs, "cpu_on", PSCI_CPU_ON_FNID))
		return -1;
	return 0;
}

/*
 * Find the first subnode that has a "device_type" property with the value
 * "cpu" and which's enable-method is not "psci" (yet).
 * Returns 0 if no such subnode is found, so all have already been patched
 * or none have to be patched in the first place.
 * Returns 1 if *one* such subnode has been found and successfully changed
 * to "psci".
 * Returns negative values on error.
 *
 * Call in a loop until it returns 0. Recalculate the node offset after
 * it has returned 1.
 */
static int dt_update_one_cpu_node(void *fdt, int offset)
{
	int offs;

	/* Iterate over all subnodes to find those with device_type = "cpu". */
	for (offs = fdt_first_subnode(fdt, offset); offs >= 0;
	     offs = fdt_next_subnode(fdt, offs)) {
		const char *prop;
		int len;
		int ret;

		prop = fdt_getprop(fdt, offs, "device_type", &len);
		if (prop == NULL)
			continue;
		if ((strcmp(prop, "cpu") != 0) || (len != 4))
			continue;

		/* Ignore any nodes which already use "psci". */
		prop = fdt_getprop(fdt, offs, "enable-method", &len);
		if ((prop != NULL) &&
		    (strcmp(prop, "psci") == 0) && (len == 5))
			continue;

		ret = fdt_setprop_string(fdt, offs, "enable-method", "psci");
		if (ret < 0)
			return ret;
		/*
		 * Subnode found and patched.
		 * Restart to accommodate potentially changed offsets.
		 */
		return 1;
	}

	if (offs == -FDT_ERR_NOTFOUND)
		return 0;

	return offs;
}

/*******************************************************************************
 * dt_add_psci_cpu_enable_methods() - switch CPU nodes in DT to use PSCI
 * @fdt:	pointer to the device tree blob in memory
 *
 * Iterate over all CPU device tree nodes (/cpus/cpu@x) in memory to change
 * the enable-method to PSCI. This will add the enable-method properties, if
 * required, or will change existing properties to read "psci".
 *
 * Return: 0 on success, or a negative error value otherwise.
 ******************************************************************************/

int dt_add_psci_cpu_enable_methods(void *fdt)
{
	int offs, ret;

	do {
		offs = fdt_path_offset(fdt, "/cpus");
		if (offs < 0)
			return offs;

		ret = dt_update_one_cpu_node(fdt, offs);
	} while (ret > 0);

	return ret;
}

#define HIGH_BITS(x) ((sizeof(x) > 4) ? ((x) >> 32) : (typeof(x))0)

/*******************************************************************************
 * fdt_add_reserved_memory() - reserve (secure) memory regions in DT
 * @dtb:	pointer to the device tree blob in memory
 * @node_name:	name of the subnode to be used
 * @base:	physical base address of the reserved region
 * @size:	size of the reserved region
 *
 * Add a region of memory to the /reserved-memory node in a device tree in
 * memory, creating that node if required. Each region goes into a subnode
 * of that node and has a @node_name, a @base address and a @size.
 * This will prevent any device tree consumer from using that memory. It
 * can be used to announce secure memory regions, as it adds the "no-map"
 * property to prevent mapping and speculative operations on that region.
 *
 * See reserved-memory/reserved-memory.txt in the (Linux kernel) DT binding
 * documentation for details.
 *
 * Return: 0 on success, a negative error value otherwise.
 ******************************************************************************/
int fdt_add_reserved_memory(void *dtb, const char *node_name,
			    uintptr_t base, size_t size)
{
	int offs = fdt_path_offset(dtb, "/reserved-memory");
	uint32_t addresses[3];

	if (offs < 0) {			/* create if not existing yet */
		offs = fdt_add_subnode(dtb, 0, "reserved-memory");
		if (offs < 0)
			return offs;
		fdt_setprop_u32(dtb, offs, "#address-cells", 2);
		fdt_setprop_u32(dtb, offs, "#size-cells", 1);
		fdt_setprop(dtb, offs, "ranges", NULL, 0);
	}

	addresses[0] = cpu_to_fdt32(HIGH_BITS(base));
	addresses[1] = cpu_to_fdt32(base & 0xffffffff);
	addresses[2] = cpu_to_fdt32(size & 0xffffffff);
	offs = fdt_add_subnode(dtb, offs, node_name);
	fdt_setprop(dtb, offs, "no-map", NULL, 0);
	fdt_setprop(dtb, offs, "reg", addresses, 12);

	return 0;
}

/*******************************************************************************
 * fdt_add_cluster_cache() - Creates a cache subnode for a given cluster
 * @dtb:		pointer to the device tree blob in memory
 * @parent:		offset of the parent node
 * @cluster:		cluster ID, starting on 0
 * @nl_phandle:		phandle of the next level cache
 * @compatible:		compatible property for the node
 *
 * Return the offset of the cluster cache node or -1 on error.
 ******************************************************************************/

static uint32_t fdt_add_cluster_cache(void *dtb, int parent,
				      unsigned int cluster,
				      uint32_t nl_phandle, char *compatible)
{
	char node_name[15];
	int node_offs;
	int err;

	if (snprintf(node_name, sizeof(node_name), "l2-cache%u", cluster)
		     >= sizeof(node_name)) {
		WARN("Node name truncated: %s\n", node_name);
	}

	node_offs = fdt_add_subnode(dtb, parent, node_name);
	if (node_offs < 0) {
		return node_offs;
	}

	err = fdt_setprop_string(dtb, node_offs, "compatible", compatible);
	if (err < 0) {
		return node_offs;
	}

	err = fdt_setprop_u32(dtb, node_offs, "next-level-cache", nl_phandle);
	if (err < 0) {
		return err;
	}

	err = fdt_setprop_u32(dtb, node_offs, "phandle",
			      fdt_get_max_phandle(dtb) + 1);
	if (err < 0) {
		return err;
	}

	return node_offs;
}

/*******************************************************************************
 * calculate_mpid() - Calculates the MPIDR value for a given cluster & cpu
 * @cluster:		Cluster ID, starting on 0, where the cpu is located
 * @cpu:		CPU ID starting on 0.
 *
 * Calculate the MPID value for a cluster/cpu combination.
 *
 * Return the expected MPID value or INVALID_MPID on error.
 ******************************************************************************/

static uint32_t calculate_mpid(unsigned int cluster, unsigned int cpu)
{
	if (cluster & ~MPIDR_AFFLVL_MASK || cpu & ~MPIDR_AFFLVL_MASK) {
		return INVALID_MPID;
	}

	return cluster << MPIDR_AFF2_SHIFT | cpu << MPIDR_AFF1_SHIFT;
}

/*******************************************************************************
 * fdt_add_cpu_node() - Add a new CPU node to the DT
 * @dtb:		Pointer to the device tree blob in memory
 * @parent:		Offset of the parent node
 * @cluster:		Cluster ID for the cpu, starting on 0
 * @cpu:		CPU ID for the cpu to add
 * @compatible:		Pointer to the compatible property
 * @nlc_phandle:	phandle for the next level cache
 *
 * Create and add a new cpu node to a DTB.
 *
 * Return the offset of the new node or a negative value in case of error
 ******************************************************************************/

static int fdt_add_cpu_node(void *dtb, int parent, unsigned int cluster,
			    unsigned int cpu, char *compatible,
			    uint32_t nlc_phandle)
{
	char string[15];
	int cpu_offs;
	int err;
	uint32_t reg_prop[2] = {0};

	reg_prop[1] = calculate_mpid(cluster, cpu);
	if (reg_prop[1] == INVALID_MPID) {
		return -1;
	}

	if (snprintf(string, sizeof(string), "cpu@%u", reg_prop[1])
		     >= sizeof(string)) {
		WARN("Subnode name truncated: %s\n", string);
	}

	cpu_offs = fdt_add_subnode(dtb, parent, string);
	if (cpu_offs < 0) {
		return cpu_offs;
	}

	err = fdt_setprop_string(dtb, cpu_offs, "compatible", compatible);
	if (err < 0) {
		return err;
	}

	err = fdt_setprop(dtb, cpu_offs, "reg", reg_prop, sizeof(reg_prop));
	if (err < 0) {
		return err;
	}

	err = fdt_setprop_string(dtb, cpu_offs, "device_type", "cpu");
	if (err < 0) {
		return err;
	}

	err = fdt_setprop_string(dtb, cpu_offs, "enable-method", "psci");
	if (err < 0) {
		return err;
	}

	err = fdt_setprop_u32(dtb, cpu_offs, "next_level_cache", nlc_phandle);
	if (err < 0) {
		return err;
	}

	err = fdt_setprop_u32(dtb, cpu_offs, "phandle",
			      fdt_get_max_phandle(dtb) + 1);
	if (err < 0) {
		return err;
	}

	return cpu_offs;
}

/*******************************************************************************
 * fdt_add_cluster() - Create and populate a cluster with its CPUs.
 * @dtb:		pointer to the device tree blob in memory
 * @cpu:		name of the node containing the topology tree
 * @cpu_map:		name of the node containing the cpu map
 * @ncpus:		number of cpus on this cluster
 * @id:			cluster ID, starting on 0
 * @cache_phandler:	phandler for the highest level cache on the topology
 *
 * Return cluster offset or negative value on error.
 ******************************************************************************/

static int fdt_add_cluster(void *dtb, uint32_t cpus_ph, uint32_t map_ph,
			   unsigned int ncpus, unsigned int id,
			   uint32_t cache_phandler)
{
	int cpus_offs, tmp_offs;
	int err;
	unsigned int i;
	char string[11];
	uint32_t cc_phandle, cpu_phandle, cluster_phandle;

	cpus_offs = fdt_node_offset_by_phandle(dtb, cpus_ph);
	if (cpus_offs < 0) {
		return cpus_offs;
	}

	/*
	 * Add a cache to the /cpus node to be used by all the CPUs
	 * on this cluster and get its phandle.
	 */
	tmp_offs = fdt_add_cluster_cache(dtb, cpus_offs, id,
					cache_phandler, "cache");
	if (tmp_offs < 0) {
		return tmp_offs;
	}
	cc_phandle = fdt_get_phandle(dtb, tmp_offs);
	if (cc_phandle == 0) {
		return -1;
	}

	/* Create an empty cluster, as a subnode of the cpu-map node */
	tmp_offs = fdt_node_offset_by_phandle(dtb, map_ph);
	if (tmp_offs < 0) {
		return tmp_offs;
	}
	if (snprintf(string, sizeof(string), "cluster%u", id) >=
		     sizeof(string)) {
		WARN("Node name truncated: %s\n", string);
	}
	tmp_offs = fdt_add_subnode(dtb, tmp_offs, string);
	if (tmp_offs < 0) {
		return tmp_offs;
	}
	cluster_phandle = fdt_get_max_phandle(dtb);
	err = fdt_setprop_u32(dtb, tmp_offs, "phandle", ++cluster_phandle);
	if (err < 0) {
		return err;
	}

	/* Create the CPU nodes and populate the cluster with them */
	for (i = 0; i < ncpus; i++) {
		/* Add a CPU node and get its phandle */
		tmp_offs = fdt_add_cpu_node(dtb, cpus_offs, id, i,
					    "arm,armv8", cc_phandle);
		if (tmp_offs < 0) {
			return tmp_offs;
		}
		cpu_phandle = fdt_get_phandle(dtb, tmp_offs);
		if (cpu_phandle == 0) {
			return -1;
		}

		/* Create the core subnode on the cluster and add the cpu */
		tmp_offs = fdt_node_offset_by_phandle(dtb, cluster_phandle);
		if (tmp_offs < 0) {
			return tmp_offs;
		}
		if (snprintf(string, sizeof(string), "core%u", i) >=
			     sizeof(string)) {
			WARN("Node truncated %s\n", string);
		}
		tmp_offs = fdt_add_subnode(dtb, tmp_offs, string);
		if (tmp_offs < 0) {
			return tmp_offs;
		}
		err = fdt_setprop_u32(dtb, tmp_offs, "cpu", cpu_phandle);
		if (err < 0) {
			return err;
		}
	}

	return fdt_node_offset_by_phandle(dtb, cluster_phandle);
}

/*******************************************************************************
 * fdt_add_topology_tree() - Add topology tree to the DT.
 * @dtb:		pointer to the device tree blob in memory
 * @tp:			pointer to the topology tree.
 *
 * Analyze the topology tree and add a topology description to the DTB (/cpus).
 * If there is already a /cpus node, exit gracefully
 * A topology tree like [1, 2, 1, 1] would generate a node equivalent to:
 *
 *	cpus {
 *		#address-cells = <2>;
 *		#size-cells = <0>;
 *
 *		cpu-map {
 *			cluster0 {
 *				core0 {
 *					cpu = <&cpu0>;
 *				};
 *			};
 *			cluster1 {
 *				core0 {
 *					cpu = <&cpu1>;
 *				};
 *			};
 *		};
 *
 *		cpu0: cpu@0 {
 *			compatible = "arm,armv8";
 *			reg = <0x0 0x0>;
 *			device_type = "cpu";
 *			enable-method = "psci";
 *			next-level-cache = <&cluster0_l2>;
 *		};
 *		cpu1: cpu@10000 {
 *			compatible = "arm,armv8";
 *			reg = <0x0 0x10000>;
 *			device_type = "cpu";
 *			enable-method = "psci";
 *			next-level-cache = <&cluster1_l2>;
 *		};
 *		cluster0_l2: l2-cache0 {
 *			compatible = "cache";
 *			next-level-cache = <&soc_l3>;
 *		};
 *		cluster1_l2: l2-cache1 {
 *			compatible = "cache";
 *			next-level-cache = <&soc_l3>;
 *		};
 *		soc_l3: l3-cache0 {
 *			compatible = "cache";
 *		};
 *	};
 *
 * Return 0 on success or a negative error value otherwise.
 ******************************************************************************/

int fdt_add_topology_tree(void *dtb, const unsigned char *tt)
{
	int offs;
	int err;
	uint32_t cache_phandle, cpus_phandle, map_phandle;
	unsigned int i;

	if (fdt_path_offset(dtb, "/cpus") >= 0) {
		WARN("Topology Tree node already exists!\n");
		return 0;
	}

	/* Perform some sanity check on the topology tree. */
	if (tt[0] == 0 || tt[1] == 0) {
		return -1;
	}

	offs = fdt_path_offset(dtb, "/");
	if (offs < 0) {
		return offs;
	}

	offs = fdt_add_subnode(dtb, offs, "cpus");
	if (offs < 0) {
		return offs;
	}
	err = fdt_setprop_u32(dtb, offs, "#address-cells", 2);
	if (err < 0) {
		return err;
	}
	err = fdt_setprop_u32(dtb, offs, "#size-cells", 0);
	if (err < 0) {
		return err;
	}
	cpus_phandle = fdt_get_max_phandle(dtb);
	err = fdt_setprop_u32(dtb, offs, "phandle", ++cpus_phandle);
	if (err < 0) {
		return err;
	}

	/* Create the cpu-map node */
	offs = fdt_add_subnode(dtb, offs, "cpu-map");
	if (offs < 0) {
		return offs;
	}
	map_phandle = fdt_get_max_phandle(dtb);
	err = fdt_setprop_u32(dtb, offs, "phandle", ++map_phandle);

	/* Create the cache node for the topology */
	offs = fdt_add_subnode(dtb, offs, "l3-cache0");
	if (offs < 0) {
		return offs;
	}
	err = fdt_setprop_string(dtb, offs, "compatible", "cache");
	if (err < 0) {
		return err;
	}
	cache_phandle = fdt_get_max_phandle(dtb);
	err = fdt_setprop_u32(dtb, offs, "phandle", ++cache_phandle);
	if (err < 0) {
		return err;
	}

	/* Populate the map with the clusters and CPUs */
	for (i = 0; i < tt[1]; i++) {
		err = fdt_add_cluster(dtb, cpus_phandle, map_phandle, tt[i+2],
				      i, cache_phandle);
		if (err < 0) {
			return err;
		}
	}

	return 0;
}
