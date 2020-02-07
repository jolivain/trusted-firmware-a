/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <libfdt.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
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
	int rc;

	assert(attr && fdt);

	rc = fdtw_read_cells(fdt, node, "maj_ver", 1, &attr->major_version);
	if (rc != 0) {
		ERROR("Missing SPCI %s version in SPM Core manifest.\n",
			"major");
		return rc;
	}

	rc = fdtw_read_cells(fdt, node, "min_ver", 1, &attr->minor_version);
	if (rc != 0) {
		ERROR("Missing SPCI %s version in SPM Core manifest.\n",
			"minor");
		return rc;
	}

	rc = fdtw_read_cells(fdt, node, "spmc_id", 1, &attr->spmc_id);
	if (rc != 0) {
		ERROR("Missing SPMC ID in manifest.\n");
		return rc;
	}

	rc = fdtw_read_cells(fdt, node, "exec_state", 1, &attr->exec_state);
	if (rc != 0) {
		NOTICE("%s not specified in SPM Core manifest.\n",
			"Execution state");
	}

	rc = fdtw_read_cells(fdt, node, "binary_size", 1, &attr->binary_size);
	if (rc != 0) {
		NOTICE("%s not specified in SPM Core manifest.\n",
			"Binary size");
	}

	rc = fdtw_read_cells(fdt, node, "load_address", 2, &attr->load_address);
	if (rc != 0) {
		NOTICE("%s not specified in SPM Core manifest.\n",
			"Load address");
	}

	rc = fdtw_read_cells(fdt, node, "entrypoint", 2, &attr->entrypoint);
	if (rc != 0) {
		NOTICE("%s not specified in SPM Core manifest.\n",
			"Entry point");
	}

	VERBOSE("SPM Core manifest attribute section:\n");
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
	const char *str = "attribute";

	assert(manifest != NULL);

	node = fdt_subnode_offset_namelen(fdt, root, str, strlen(str));
	if (node < 0) {
		ERROR("Root node doesn't contain subnode '%s'\n", str);
		return node;
	}

	return manifest_parse_attribute(manifest, fdt, node);
}

/*******************************************************************************
 * Platform handler to parse a SPM Core manifest.
 ******************************************************************************/
int plat_spm_core_manifest_load(spmc_manifest_attribute_t *manifest,
				const void *pm_addr)
{
	int rc, unmap_ret;
	uintptr_t pm_base, pm_base_align;
	size_t mapped_size;

	assert(manifest != NULL);
	assert(pm_addr != NULL);

	/*
	 * Assume TOS_FW_CONFIG is not necessarily aligned to a page
	 * boundary, thus calculate the remaining space between SPMC
	 * manifest start address and upper page limit.
	 *
	 */
	pm_base = (uintptr_t)pm_addr;
	pm_base_align = page_align(pm_base, UP);
	mapped_size = pm_base_align - pm_base;

	/* Check space within the page at least maps the FDT header */
	if (mapped_size < sizeof(struct fdt_header)) {
		ERROR("Error while mapping SPM core manifest.\n");
		return -EINVAL;
	}

	/* Map first SPMC manifest page in the SPMD translation regime */
	pm_base_align = page_align(pm_base, DOWN);
	rc = mmap_add_dynamic_region((unsigned long long) pm_base_align,
				     pm_base_align,
				     PAGE_SIZE,
				     MT_RO_DATA);
	if (rc != 0) {
		ERROR("Error while mapping SPM core manifest (%d).\n", rc);
		return rc;
	}

	rc = fdt_check_header(pm_addr);
	if (rc != 0) {
		ERROR("Wrong format for SPM Core manifest (%d).\n", rc);
		goto exit_unmap;
	}

	/* Check SPMC manifest fits within the upper mapped page boundary */
	if (mapped_size < fdt_totalsize(pm_addr)) {
		ERROR("SPM Core manifest too large.\n");
		rc = -EINVAL;
		goto exit_unmap;
	}

	VERBOSE("Reading SPM core manifest at address %p\n", pm_addr);

	rc = fdt_node_offset_by_compatible(pm_addr, -1,
				"arm,spci-core-manifest-1.0");
	if (rc < 0) {
		ERROR("Unrecognized SPM Core manifest\n");
		goto exit_unmap;
	}

	rc = manifest_parse_root(manifest, pm_addr, rc);

exit_unmap:
	unmap_ret = mmap_remove_dynamic_region(pm_base_align, PAGE_SIZE);
	if (unmap_ret != 0) {
		ERROR("Error while unmapping SPM core manifest (%d).\n",
			unmap_ret);
	}

	return rc;
}
