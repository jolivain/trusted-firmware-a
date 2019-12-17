/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/fconf/fconf.h>
#include <libfdt.h>
#include <plat/common/platform.h>
#include <platform_def.h>

struct fconf_dtb_info_t fconf_dtb_info;

void fconf_load_config(void)
{
	int err;
	uintptr_t config_base = 0UL;
	image_desc_t *desc;

	/* fconf FW_CONFIG and TB_FW_CONFIG are currently the same DTB */
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
	fconf_dtb_info.base_addr = (void *)config_base;
	fconf_dtb_info.size = (size_t)arm_tb_fw_info.image_info.image_max_size;

	/* The BL2 ep_info arg0 is modified to point to TB_FW_CONFIG */
	desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(desc != NULL);
	desc->ep_info.args.arg0 = config_base;

	INFO("FCONF: TB_FW_CONFIG loaded at address = 0x%lx\n", config_base);
}

void fconf_populate(void *dtb)
{
	assert(dtb);

	/* Check if the pointer to DTB is correct */
	if (fdt_check_header(dtb) != 0) {
		WARN("FCONF: Invalid DTB file passed for tbbr.dyn_config\n");
		panic();
	}

	INFO("FCONF: populate from: %p\n", dtb);

	/* go through all registered populate functions */
	IMPORT_SYM(struct fconf_populator *, __FCONF_POPULATOR_START__, start);
	IMPORT_SYM(struct fconf_populator *, __FCONF_POPULATOR_END__, end);
	struct fconf_populator *populator;

	for (populator = (struct fconf_populator *) start; populator != end; populator++) {
		assert(populator->info && populator->populate);

		INFO("FCONF: call %s populator fonction\n", populator->info);
		if (populator->populate(dtb) != 0) {
			/* TODO: handle property miss */
		}
	}

	/* save local pointer to dtb */
	fconf_dtb_info.base_addr = dtb;
}
