/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <plat/common/platform.h>
#include <services/ffa_svc.h>
#include <services/spm_core_manifest.h>

#include <tegra_private.h>

/*
 * These parameters should be extracted from SPM core manifest file. However,
 * we do not have a core manifest available for cactus image running as SPMC;
 * ergo the following macros.
 */
#define CACTUS_SPMC_ID		(0x8000U)

/*******************************************************************************
 * Platform handler to parse a SPM Core manifest.
 ******************************************************************************/
int plat_spm_core_manifest_load(spmc_manifest_attribute_t *manifest,
				const void *pm_addr)
{
	entry_point_info_t *ep = bl31_plat_get_next_image_ep_info(SECURE);
	assert(ep != NULL);

	/* update manifest */
	manifest->major_version = FFA_VERSION_MAJOR;
	manifest->minor_version = FFA_VERSION_MINOR;
	manifest->spmc_id = CACTUS_SPMC_ID;
	manifest->exec_state = SPSR_M_AARCH64;
	manifest->load_address = ep->pc;
	manifest->entrypoint = ep->pc;
	manifest->binary_size = 0U;

	VERBOSE("SPM Core manifest attribute section:\n");
	VERBOSE("  version: %u.%u\n", manifest->major_version,
			manifest->minor_version);
	VERBOSE("  spmc_id: 0x%x\n", manifest->spmc_id);
	VERBOSE("  binary_size: 0x%x\n", manifest->binary_size);
	VERBOSE("  load_address: 0x%llx\n", manifest->load_address);
	VERBOSE("  entrypoint: 0x%llx\n", manifest->entrypoint);

	return 0;
}
