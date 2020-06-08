/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#if MEASURED_BOOT
#include <common/desc_image_load.h>
#endif

#include <drivers/arm/sp804_delay_timer.h>

#if MEASURED_BOOT
#include <drivers/measured_boot/measured_boot.h>
#endif

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include "fvp_private.h"

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
	arm_bl2_early_platform_setup((uintptr_t)arg0, (meminfo_t *)arg1);

	/* Initialize the platform config for future decision making */
	fvp_config_setup();
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();

	/* Initialize System level generic or SP804 timer */
	fvp_timer_init();
}

#if MEASURED_BOOT
int fvp_bl2_plat_handle_post_image_load(unsigned int image_id)
{
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
	image_info_t image_info = bl_mem_params->image_info;
	int err;

	if ((image_info.h.attr & IMAGE_ATTRIB_SKIP_LOADING) == 0) {
		/* Calculate image hash and record data in Event Log */
		err = tpm_record_measurement(image_info.image_base,
					     image_info.image_size, image_id);
		if (err != 0) {
			ERROR("BL2: Failed to record image id %u (%i)\n",
							image_id, err);
			return err;
		}
	}

	return arm_bl2_handle_post_image_load(image_id);
}

int arm_bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return fvp_bl2_plat_handle_post_image_load(image_id);
}
#endif	/* MEASURED_BOOT */
