/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <services/spm_core_manifest.h>

/*******************************************************************************
 * Platform handler to parse a SPM Core manifest.
 ******************************************************************************/
int plat_spm_core_manifest_load(spmc_manifest_attribute_t *manifest,
				const void *pm_addr)
{
	manifest->major_version = 1;
	manifest->minor_version = 1;
	manifest->exec_state = 0;
	manifest->load_address = 0xe100000;
	manifest->entrypoint = 0;
	manifest->binary_size = 0x100000;
	manifest->spmc_id = 0x8000;

	return 0;
}
