/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/fconf/fconf.h>
#include <plat/common/platform.h>

static void *tb_fw_cfg_dtb;
static size_t tb_fw_cfg_dtb_size;

void fconf_load_config(void)
{
	int err;
	uintptr_t config_base = 0UL;
	image_desc_t *desc;

	image_desc_t arm_tb_fw_info = {
		.image_id = TB_FW_CONFIG_ID,
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				VERSION_2, image_info_t, 0),
		.image_info.image_base = ARM_TB_FW_CONFIG_BASE,
		.image_info.image_max_size =
			ARM_TB_FW_CONFIG_LIMIT - ARM_TB_FW_CONFIG_BASE
	};

	VERBOSE("FCONF: Loading FW_CONFIG\n");
	err = load_auth_image(TB_FW_CONFIG_ID, &arm_tb_fw_info.image_info);
	if (err != 0) {
		/* Return if TB_FW_CONFIG is not loaded */
		VERBOSE("Failed to load TB_FW_CONFIG\n");
		return;
	}

	/* At this point we know that a DTB is indeed available */
	config_base = arm_tb_fw_info.image_info.image_base;
	tb_fw_cfg_dtb = (void *)config_base;
	tb_fw_cfg_dtb_size = (size_t)arm_tb_fw_info.image_info.image_max_size;

	/* The BL2 ep_info arg0 is modified to point to TB_FW_CONFIG */
	desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(desc != NULL);
	desc->ep_info.args.arg0 = config_base;

	INFO("FCONF: TB_FW_CONFIG loaded at address = 0x%lx\n", config_base);
}

void fconf_populate(void *dtb)
{
	/* go through all registered populate functions */
	extern struct fconf_populator *__FCONF_POPULATOR_START__;
	extern struct fconf_populator *__FCONF_POPULATOR_END__;
	struct fconf_populator *start = (struct fconf_populator *) &__FCONF_POPULATOR_START__;
	struct fconf_populator *end = (struct fconf_populator *) &__FCONF_POPULATOR_END__;
	struct fconf_populator *populator;

	for (populator = start; populator != end; populator++) {
		INFO("FCONF: call %s populator fonction\n", populator->info);
		if (populator->populate(dtb) != 0) {
			/* TODO: handle property miss */
		}
	}
}
