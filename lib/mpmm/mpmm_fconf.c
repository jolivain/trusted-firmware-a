/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <libfdt.h>
#include <lib/fconf/fconf.h>
#include "mpmm_private.h"

#include <plat/common/platform.h>

static bool mpmm_topology_populated_; /* Whether the topology is valid */
static struct mpmm_fconf_topology mpmm_topology_; /* Populated topology cache */

const struct mpmm_fconf_topology *mpmm_topology(void)
{
	if (!mpmm_topology_populated_) {
		return NULL;
	}

	return &mpmm_topology_;
}

/*
 * Within a `cpu` node, determine support for MPMM via the `supports-mpmm`
 * property.
 *
 * Returns `0` on success, or a negative integer representing an error code.
 */
static int mpmm_fconf_populate_cpu(const void *fdt, int off, uintptr_t mpidr)
{
	struct mpmm_fconf_core *core;
	int ret, len;

	core = &mpmm_topology_.cores[plat_core_pos_by_mpidr(mpidr)];

	fdt_getprop(fdt, off, "supports-mpmm", &len);
	if (len >= 0) {
		core->supported = true;
		ret = 0;
	} else {
		core->supported = false;
		ret = len;
	}

	return ret;
}

/*
 * For every CPU node (`/cpus/cpu@n`) in an FDT, executes a callback passing a
 * pointer to the FDT and the offset of the CPU node. If the return value of the
 * callback is negative, it is treated as an error and the loop is aborted. In
 * this situation, the value of the callback is returned from the function.
 *
 * Returns `0` on success, or a negative integer representing an error code.
 */
static int mpmm_fconf_foreach_cpu(
	const void *fdt,
	int (*callback)(const void *, int, uintptr_t))
{
	int ret = 0;
	int parent, node = 0;

	parent = fdt_path_offset(fdt, "/cpus");
	if (parent < 0) {
		if (parent == -FDT_ERR_NOTFOUND) {
			parent = 0;
		}

		return parent;
	}

	fdt_for_each_subnode(node, fdt, parent) {
		const char *name;
		int len;

		uintptr_t mpidr = 0;

		name = fdt_get_name(fdt, node, &len);
		if (strncmp(name, "cpu@", 4) != 0) {
			continue;
		}

		ret = fdt_get_reg_props_by_index(fdt, node, 0, &mpidr, NULL);
		if (ret < 0) {
			break;
		}

		ret = callback(fdt, node, mpidr);
		if (ret < 0) {
			break;
		}
	}

	if ((node < 0) && (node != -FDT_ERR_NOTFOUND)) {
		return node;
	}

	return ret;
}

/*
 * Populates the global `mpmm_topology` structure based on what's described by
 * the hardware configuration device tree blob.
 *
 * The device tree is expected to provide a `supports-mpmm` property for each
 * `cpu` node, like so:
 *
 *     cpu@0 {
 *       supports-mpmm;
 *     };
 *
 * This property indicates whether the core implements MPMM.
 */
static int mpmm_fconf_populate(uintptr_t config)
{
	int ret = mpmm_fconf_foreach_cpu(
		(const void *)config, mpmm_fconf_populate_cpu);
	if (ret >= 0) {
		mpmm_topology_populated_ = true;
	} else {
		ERROR("MPMM-FCONF: Failed to configure MPMM: %d\n", ret);
	}

	return ret;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, mpmm, mpmm_fconf_populate);
