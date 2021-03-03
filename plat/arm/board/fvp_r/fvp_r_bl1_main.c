#include <assert.h>

#include <platform_def.h>

#include <arch.h>
/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <arch_helpers.h>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/console.h>
#include <lib/cpus/errata_report.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <smccc_helpers.h>
#include <tools_share/uuid.h>

#include "../../../../bl1/bl1_private.h"

void bl1_run_next_image(const struct entry_point_info *bl_ep_info);

/*******************************************************************************
 * Function to perform late architectural and platform specific initialization.
 * It also queries the platform to load and run next BL image. Only called
 * by the primary cpu after a cold boot.
 ******************************************************************************/
void bl1_transfer_bl33 (void)
{
	unsigned int image_id;

	/* Get the image id of next image to load and run. */
	image_id = bl1_plat_get_next_image_id();

#if ENABLE_PAUTH
	/*
	 * Disable pointer authentication before running next boot image
	 */
	pauth_disable_el2();
#endif /* ENABLE_PAUTH */

#if !ARM_DISABLE_TRUSTED_WDOG
	/* Disable watchdog before leaving BL1 */
	plat_arm_secure_wdt_stop();
#endif

	bl1_run_next_image(&bl1_plat_get_image_desc(image_id)->ep_info);
}

/*******************************************************************************
 * This function locates and loads the BL33 raw binary image in the trusted SRAM.
 * Called by the primary cpu after a cold boot.
 * TODO: Add support for alternative image load mechanism e.g using virtio/elf
 * loader etc.
 ******************************************************************************/
void bl1_load_bl33(void)
{
	image_desc_t *desc;
	image_info_t *info;
	int err;

	/* Get the image descriptor */
	desc = bl1_plat_get_image_desc(BL33_IMAGE_ID);
	assert(desc != NULL);

	/* Get the image info */
	info = &desc->image_info;
	INFO("BL1: Loading BL33\n");

	err = bl1_plat_handle_pre_image_load(BL33_IMAGE_ID);
	if (err != 0) {
		ERROR("Failure in pre image load handling of BL33 (%d)\n", err);
		plat_error_handler(err);
	}

	err = load_auth_image(BL33_IMAGE_ID, info);
	if (err != 0) {
		ERROR("Failed to load BL33 firmware.\n");
		plat_error_handler(err);
	}

	/* Allow platform to handle image information. */
	err = bl1_plat_handle_post_image_load(BL33_IMAGE_ID);
	if (err != 0) {
		ERROR("Failure in post image load handling of BL33 (%d)\n", err);
		plat_error_handler(err);
	}

	NOTICE("BL1: Booting BL33\n");
}
