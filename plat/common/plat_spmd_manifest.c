/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>
#include <libfdt.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <errno.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform_def.h>
#include <services/spm_core_manifest.h>

/*******************************************************************************
 * SPMC attribute node parser
 ******************************************************************************/
static int manifest_parse_attribute(spmc_manifest_attribute_t *attr,
				    const void *fdt,
				    int node)
{
	int rc = 0;

	assert(attr && fdt);

	rc = fdtw_read_cells(fdt, node, "maj_ver", 1, &attr->major_version);
	if (rc) {
		ERROR("Missing SPCI %s version in SPM core manifest.\n", "major");
		return -ENOENT;
	}

	rc = fdtw_read_cells(fdt, node, "min_ver", 1, &attr->minor_version);
	if (rc) {
		ERROR("Missing SPCI %s version in SPM core manifest.\n", "minor");
		return -ENOENT;
	}

	rc = fdtw_read_cells(fdt, node, "spmc_id", 1, &attr->spmc_id);
	if (rc) {
		ERROR("Missing SPMC ID in manifest.\n");
		return -ENOENT;
	}

	rc = fdtw_read_cells(fdt, node, "exec_state", 1, &attr->exec_state);
	if (rc) {
		NOTICE("%s not specified in SPM core manifest.\n", "Execution state");
	}

	rc = fdtw_read_cells(fdt, node, "binary_size", 1, &attr->binary_size);
	if (rc) {
		NOTICE("%s not specified in SPM core manifest.\n", "Binary size");
	}

	rc = fdtw_read_cells(fdt, node, "load_address", 2, &attr->load_address);
	if (rc) {
		NOTICE("%s not specified in SPM core manifest.\n", "Load address");
	}

	rc = fdtw_read_cells(fdt, node, "entrypoint", 2, &attr->entrypoint);
	if (rc) {
		NOTICE("%s not specified in SPM core manifest.\n", "Entry point");
	}

	VERBOSE("SPM core manifest attribute section:\n");
	VERBOSE("  version: %x.%x\n", attr->major_version, attr->minor_version);
	VERBOSE("  spmc_id: %x\n", attr->spmc_id);
	VERBOSE("  binary_size: 0x%x\n", attr->binary_size);
	VERBOSE("  load_address: 0x%llx\n", attr->load_address);
	VERBOSE("  entrypoint: 0x%llx\n", attr->entrypoint);

	return 0;
}

/*******************************************************************************
 * Root node handler
 ******************************************************************************/
static int manifest_parse_root(spmc_manifest_attribute_t *manifest,
			       const void *fdt,
			       int root)
{
	int node;
	char *str;

	str = "attribute";
	node = fdt_subnode_offset_namelen(fdt, root, str, strlen(str));
	if (node < 0) {
		ERROR("Root node doesn't contain subnode '%s'\n", str);
		return -ENOENT;
	}

	return manifest_parse_attribute(manifest, fdt, node);
}

/*******************************************************************************
 * Platform handler to parse a SPM core manifest.
 ******************************************************************************/
int plat_spm_core_manifest_load(spmc_manifest_attribute_t *manifest,
				const void *pm_addr)
{
	int rc;
	int root_node;
	uintptr_t pm_base, pm_base_align;

	assert(manifest != NULL);
	assert(pm_addr != NULL);

	pm_base = (uintptr_t) pm_addr;
	pm_base_align = page_align(pm_base, DOWN);
	if (pm_base - pm_base_align < sizeof(struct fdt_header)) {
		ERROR("Error while mapping SPM core manifest.\n");
		panic();
	}

	/* Map first partition manifest page in the SPMD translation regime */
	VERBOSE("SPM core manifest base : 0x%lx\n", pm_base_align);
	rc = mmap_add_dynamic_region((unsigned long long) pm_base_align,
				     pm_base_align,
				     PAGE_SIZE,
				     MT_RO_DATA);
	if (rc != 0) {
		ERROR("Error while mapping SPM core manifest (%d).\n", rc);
		panic();
	}

	rc = fdt_check_header(pm_addr);
	if (rc != 0) {
		ERROR("Wrong format for SPM core manifest (%d).\n", rc);
		rc = -EINVAL;
		goto exit_unmap;
	}

	VERBOSE("Reading SPM core manifest at address %p\n", pm_addr);

	root_node = fdt_node_offset_by_compatible(pm_addr, -1,
				"arm,spci-core-manifest-1.0");
	if (root_node < 0) {
		ERROR("Unrecognized SPM core manifest\n");
		rc = -ENOENT;
		goto exit_unmap;
	}

	rc = manifest_parse_root(manifest, pm_addr, root_node);

exit_unmap:
	rc = mmap_remove_dynamic_region(pm_base_align, PAGE_SIZE);
	if (rc != 0) {
		ERROR("Error while unmapping SPM core manifest (%d).\n", rc);
		panic();
	}

	return rc;
}
