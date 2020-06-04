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
#include <common/fdt_wrappers.h>
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
 * Return the offset of the cluster cache node or negative value on error.
 ******************************************************************************/

static int fdt_add_cluster_cache(void *dtb, int parent,
				      unsigned int cluster,
				      uint32_t nl_phandle, char *compatible)
{
	char node_name[15];
	int node_offs;
	int err;
	uint32_t phandle;

	if (snprintf(node_name, sizeof(node_name), "l2-cache%u", cluster)
		     >= sizeof(node_name)) {
		WARN("Node name truncated: %s\n", node_name);
	}

	node_offs = fdt_add_subnode(dtb, parent, node_name);
	if (node_offs < 0) {
		ERROR ("FDT: add subnode %s to node with offset %i failed\n",
			node_name, parent);
		return node_offs;
	}

	err = fdt_setprop_string(dtb, node_offs, "compatible", compatible);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"compatible", node_offs);
		return err;
	}

	err = fdt_setprop_u32(dtb, node_offs, "next-level-cache", nl_phandle);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"next-level-cache", node_offs);
		return err;
	}

	err = fdt_find_max_phandle(dtb, &phandle);
	if (err < 0) {
		ERROR ("FDT: find max phandle on the DTB failed: %i\n", err);
		return err;
	}
	err = fdt_setprop_u32(dtb, node_offs, "phandle", phandle + 1);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"phandle", node_offs);
		return err;
	}

	return node_offs;
}

/*******************************************************************************
 * fdt_add_cpu_node() - Add a new CPU node to the DT
 * @dtb:		Pointer to the device tree blob in memory
 * @parent:		Offset of the parent node
 * @cluster:		Cluster ID for the cpu, starting on 0
 * @cpu:		CPU ID for the cpu to add
 * @mpidr:		MPIDR for the current CPU
 * @compatible:		Pointer to the compatible property
 * @nlc_phandle:	phandle for the next level cache
 *
 * Create and add a new cpu node to a DTB.
 *
 * Return the offset of the new node or a negative value in case of error
 ******************************************************************************/

static int fdt_add_cpu_node(void *dtb, int parent, unsigned int cluster,
			    unsigned int cpu, uint32_t mpidr, char *compatible,
			    uint32_t nlc_phandle, uint32_t phandle)
{
	int cpu_offs;
	int err;
	char snode_name[15];
	uint64_t reg_prop;

	reg_prop = mpidr;

	snprintf(snode_name, sizeof(snode_name), "cpu@%u",
		(uint32_t)(reg_prop & MPID_MASK));

	cpu_offs = fdt_add_subnode(dtb, parent, snode_name);
	if (cpu_offs < 0) {
		ERROR ("FDT: add subnode \"%s\" failed: %i\n",
							snode_name, cpu_offs);
		return cpu_offs;
	}

	err = fdt_setprop_string(dtb, cpu_offs, "compatible", compatible);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"compatible", cpu_offs);
		return err;
	}

	err = fdt_setprop_u64(dtb, cpu_offs, "reg", reg_prop);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"reg", cpu_offs);
		return err;
	}

	err = fdt_setprop_string(dtb, cpu_offs, "device_type", "cpu");
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"device_type", cpu_offs);
		return err;
	}

	err = fdt_setprop_string(dtb, cpu_offs, "enable-method", "psci");
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"enable-method", cpu_offs);
		return err;
	}

	err = fdt_setprop_u32(dtb, cpu_offs, "next_level_cache", nlc_phandle);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"next_level_cache", cpu_offs);
		return err;
	}

	err = fdt_setprop_u32(dtb, cpu_offs, "phandle", phandle);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"phandle", cpu_offs);
		return err;
	}

	return cpu_offs;
}

/*******************************************************************************
 * fdt_add_cluster() - Create and populate a cluster with its CPUs.
 * @dtb:		pointer to the device tree blob in memory
 * @cpus_ph:		phandler of the cpus node
 * @map_ph:		phandler of the cpu map node
 * @ncpus:		number of cpus on this cluster
 * @id:			cluster ID, starting on 0
 * @cache_ph:		phandler for the highest level cache on the topology
 *
 * Return a negative value on error or zero or a node offset otherwise.
 ******************************************************************************/

static int fdt_add_cluster(void *dtb, uint32_t cpus_ph, uint32_t map_ph,
			   unsigned int ncpus, unsigned int id,
			   uint32_t cache_ph)
{
	int cpus_offs, tmp_offs;
	int err;
	unsigned int i;
	char snode_name[11];
	uint32_t cc_phandle, cpu_phandle, cluster_phandle;
	uint32_t mpidr;

	/*
	 * First cpu id for the current cluster assuming that the topology
	 * tree is based on the maximum affinity levels used at build time,
	 * hence all the clusters are described with the same number of CPUs.
	 */
	i = id * ncpus;

	while ((i < (id + 1) * ncpus) &&
	       (plat_get_mpidr_by_core_pos(i) == INVALID_MPID)) {
		i++;
	}

	if (i == (id + 1) * ncpus) {
		/* Cluster not implemented on the current platform. */
		return 0;
	}

	cpus_offs = fdt_node_offset_by_phandle(dtb, cpus_ph);
	if (cpus_offs < 0) {
		return cpus_offs;
	}

	/*
	 * Add a cache to the /cpus node to be used by all the CPUs
	 * on this cluster and get its phandle.
	 */
	tmp_offs = fdt_add_cluster_cache(dtb, cpus_offs, id,
					 cache_ph, "cache");
	if (tmp_offs < 0) {
		ERROR ("FDT: add new cache node to node at offset %i failed\n",
			cpus_offs);
		return tmp_offs;
	}
	cc_phandle = fdt_get_phandle(dtb, tmp_offs);
	if (cc_phandle == 0) {
		ERROR ("FDT: error getting phandle for node at offset %i\n",
			tmp_offs);
		return -1;
	}

	/* Create an empty cluster, as a subnode of the cpu-map node */
	tmp_offs = fdt_node_offset_by_phandle(dtb, map_ph);
	if (tmp_offs < 0) {
		ERROR ("FDT: error getting offset for phandle %i\n",
			map_ph);
		return tmp_offs;
	}
	snprintf(snode_name, sizeof(snode_name), "cluster%u", id);

	tmp_offs = fdt_add_subnode(dtb, tmp_offs, snode_name);
	if (tmp_offs < 0) {
		ERROR ("FDT: add subnode %s to node at offset %i failed\n",
			snode_name, tmp_offs);
		return tmp_offs;
	}
	err =  fdt_find_max_phandle(dtb, &cluster_phandle);
	if (err < 0) {
		ERROR ("FDT: find max phandle on the DTB failed: %i\n", err);
		return err;
	}
	err = fdt_setprop_u32(dtb, tmp_offs, "phandle", cluster_phandle + 1);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"phandle", tmp_offs);
		return err;
	}

	/* Create the CPU nodes and populate the cluster with them */
	cpu_phandle = cluster_phandle + 2;
	for (; i < (id + 1) * ncpus; i++) {
		mpidr = plat_get_mpidr_by_core_pos(i);
		if (mpidr == INVALID_MPID) {
			continue;
		}

		/* Add a CPU node and get its phandle */
		tmp_offs = fdt_add_cpu_node(dtb, cpus_offs, id, i, mpidr,
					    "arm,armv8", cc_phandle,
					     cpu_phandle);
		if (tmp_offs < 0) {
			ERROR ("FDT: add new cpu node to node at offset %i failed\n",
				cpus_offs);
			return tmp_offs;
		}

		/* Create the core subnode on the cluster and add the cpu */
		tmp_offs = fdt_node_offset_by_phandle(dtb, cluster_phandle);
		if (tmp_offs < 0) {
			ERROR ("FDT: error getting offset for phandle %i\n",
				cluster_phandle);
			return tmp_offs;
		}
		snprintf(snode_name, sizeof(snode_name), "core%u", i);

		tmp_offs = fdt_add_subnode(dtb, tmp_offs, snode_name);
		if (tmp_offs < 0) {
			ERROR ("FDT: add new subnode %s to node at offset %i failed\n",
				snode_name, cpus_offs);
			return tmp_offs;
		}
		err = fdt_setprop_u32(dtb, tmp_offs, "cpu", cpu_phandle);
		if (err < 0) {
			ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
				"cpu", tmp_offs);
			return err;
		}
		cpu_phandle++;
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
 * assuming that all the nodes on the tree are available in the platform.
 *
 * Return 0 on success or a negative error value otherwise.
 *
 * NOTE: This fucntion as well as its acillary ones assume that the device tree
 *       is based on the maximum affinity levels used at build time.
 ******************************************************************************/

int fdt_add_topology_tree(void *dtb, const unsigned char *pd_tree)
{
	int offs;
	int err;
	uint32_t phandle_base;
	unsigned int i;

	if (fdt_path_offset(dtb, "/cpus") >= 0) {
		WARN("Topology Tree node already exists\n");
		return 0;
	}

	/* Perform some sanity check on the topology tree. */
	if (pd_tree[0] == 0 || pd_tree[1] == 0) {
		return -1;
	}

	offs = fdt_add_subnode(dtb, 0, "cpus");
	if (offs < 0) {
		ERROR ("FDT: add subnode \"cpus\" node to parent node failed");
		return offs;
	}
	err = fdt_setprop_u32(dtb, offs, "#address-cells", 2);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"#address-cells", offs);

		return err;
	}
	err = fdt_setprop_u32(dtb, offs, "#size-cells", 0);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"#size-cells", offs);
		return err;
	}

	err = fdt_find_max_phandle(dtb, &phandle_base);
	if (err < 0) {
		ERROR ("FDT: find max phandle on the DTB failed: %i\n", err);
		return err;
	}
	err = fdt_setprop_u32(dtb, offs, "phandle", phandle_base + 1);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"#phandle", offs);
		return err;
	}

	/* Create the cpu-map node */
	offs = fdt_add_subnode(dtb, offs, "cpu-map");
	if (offs < 0) {
		ERROR ("FDT: create subnode \"%s\" at offset %i failed\n",
			"cpu_map", offs);
		return offs;
	}
	err = fdt_setprop_u32(dtb, offs, "phandle", phandle_base + 2);

	/* Create the cache node for the topology */
	offs = fdt_add_subnode(dtb, offs, "l3-cache0");
	if (offs < 0) {
		ERROR ("FDT: create subnode \"%s\" at offset %i failed\n",
			"l3-cache0", offs);
		return offs;
	}
	err = fdt_setprop_string(dtb, offs, "compatible", "cache");
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"compatible", offs);
		return err;
	}

	err = fdt_setprop_u32(dtb, offs, "phandle", phandle_base + 3);
	if (err < 0) {
		ERROR ("FDT: write to \"%s\" property of node at offset %i failed\n",
			"#phanle", offs);
		return err;
	}

	/* Populate the map with the clusters and CPUs */
	for (i = 0; i < pd_tree[1]; i++) {
		err = fdt_add_cluster(dtb, phandle_base + 1, phandle_base + 2,
				      pd_tree[i+2], i, phandle_base + 3);
		if (err < 0) {
			ERROR ("FDT: error adding cluster %u to DTB\n", i);
			return err;
		}
	}

	err = fdt_pack(dtb);
	if (err < 0)
		ERROR("FDT: Failed to pack Device Tree at %p\n", dtb);

	clean_dcache_range((uintptr_t)dtb, fdt_blob_size(dtb));

	return 0;
}
