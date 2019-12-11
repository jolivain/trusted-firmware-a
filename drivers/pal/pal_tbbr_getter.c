/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <common/debug.h>

#include <common/fdt_wrappers.h>
#include <drivers/pal/pal_tbbr_getter.h>
#include <libfdt.h>

int pal_populate_tbbr_dyn_config(void *dtb)
{
	int err;
	int node;

	/* Assert the node offset point to "arm,tb_fw" compatible property */
	node = fdt_node_offset_by_compatible(dtb, -1, "arm,tb_fw");

	/* Locate the disable_auth cell and read the value */
	err = fdtw_read_cells(dtb, node, "disable_auth", 1, &tbbr_dyn_config.disable_auth);
	if (err < 0) {
		WARN("PAL: Read cell failed for `disable_auth`\n");
		return -1;
	}

	/* Check if the value is boolean */
	if ((tbbr_dyn_config.disable_auth != 0U) && (tbbr_dyn_config.disable_auth != 1U)) {
		WARN("Invalid value for `disable_auth` cell %d\n", tbbr_dyn_config.disable_auth);
		return -1;
	}

	/* Retrieve the Mbed TLS heap details from the DTB */
	err = fdtw_read_cells(dtb, node,
		"mbedtls_heap_addr", 2, &tbbr_dyn_config.mbedtls_heap_addr);
	if (err < 0) {
		ERROR("PAL: Read cell failed for mbedtls_heap_addr\n");
		return -1;
	}
	err = fdtw_read_cells(dtb, node,
		"mbedtls_heap_size", 1, &tbbr_dyn_config.mbedtls_heap_size);
	if (err < 0) {
		ERROR("PAL: Read cell failed for mbedtls_heap_size\n");
		return -1;
	}

	VERBOSE("PAL:tbbr.disable_auth cell found with value = %d\n",
					tbbr_dyn_config.disable_auth);
	VERBOSE("PAL:tbbr.mbedtls_heap_addr cell found with value = %p\n",
					tbbr_dyn_config.mbedtls_heap_addr);
	VERBOSE("PAL:tbbr.mbedtls_heap_size cell found with value = %lu\n",
					tbbr_dyn_config.mbedtls_heap_size);
	return 0;
}

PAL_REGISTER_POPULATOR(tbbr, pal_populate_tbbr_dyn_config);
