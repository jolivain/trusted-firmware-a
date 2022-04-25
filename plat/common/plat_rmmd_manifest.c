/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <plat/common/platform.h>
#include <services/rmm_core_manifest.h>

int plat_rmmd_load_manifest(rmm_manifest_t *manifest)
{
	assert(manifest != NULL);

	manifest->version = RMMD_MANIFEST_VERSION;
	manifest->plat_data = (uintptr_t)NULL;

	return 0;
}
