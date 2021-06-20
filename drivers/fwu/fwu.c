/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/fwu/fwu.h>
#include "fwu_private.h"

#include <plat/common/platform.h>

bool is_fwu_initialized;

/*******************************************************************************
 * fwu_is_trial_run_state:
 *
 * The system runs in trial run state if any of the images in the the active
 * firmware bank has not been accepted yet.
 *
 * Returns true if the system is running in trial state.
 ******************************************************************************/
bool fwu_is_trial_run_state(void)
{
	bool trial_run = false;

	if (is_fwu_initialized == true) {
		for (unsigned int i = 0U; i < NR_OF_IMAGES_IN_FW_BANK; i++) {
			struct fwu_image_entry *entry = &metadata.img_entry[i];
			struct fwu_image_properties *img_props =
				&entry->img_props[metadata.active_index];
			if (img_props->accepted == 0) {
				trial_run = true;
				break;
			}
		}
	} else {
		ERROR("FWU metadata is not initialized\n");
		panic();
	}

	return trial_run;
}

/*******************************************************************************
 * fwu_init:
 *
 * Initialize local FWU metadata structure with verified copy of FWU metadata
 * image present in underlying NV storage, and set the source addresses of the
 * firmware updated images in IO policy.
 ******************************************************************************/
void fwu_init(void)
{
	/* Load FWU metadata which will be used to load proper FIP */
	int result = fwu_metadata_load(FWU_METADATA_IMAGE_ID);

	if (result != 0) {
		WARN("loading of FWU-Metadata failed, "
		     "using Bkup-FWU-Metadata\n");

		result = fwu_metadata_load(BKUP_FWU_METADATA_IMAGE_ID);
		if (result != 0) {
			ERROR("loading of Bkup-FWU-Metadata failed\n");
			panic();
		}
	}

	plat_set_fwu_images_source(metadata.active_index);

	is_fwu_initialized = true;
}
