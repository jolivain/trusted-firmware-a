/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <libfdt.h>
#include <plat/common/platform.h>
#include <platform_def.h>

void fconf_load_config(unsigned int image_id)
{
	int err;
	struct dyn_cfg_dtb_info_t *config_info;

	image_info_t image_info = {
		.h.type = (uint8_t)PARAM_IMAGE_BINARY,
		.h.version = (uint8_t)VERSION_2,
		.h.size = (uint16_t)sizeof(image_info_t),
		.h.attr = 0
	};

	config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, image_id);
	image_info.image_base = config_info->config_addr;
	image_info.image_max_size = config_info->config_max_size;

	VERBOSE("FCONF: Loading config %d\n", image_id);
	err = load_auth_image(image_id, &image_info);
	if (err != 0) {
		/* Return if FW_CONFIG is not loaded */
		VERBOSE("Failed to load config %d, continuing without it\n",
			image_id);
		return;
	}

#ifdef IMAGE_BL1
	if (image_id == FW_CONFIG_ID) {
		image_desc_t *desc;

		/* The BL2 ep_info arg0 is modified to point to FW_CONFIG */
		desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
		assert(desc != NULL);
		desc->ep_info.args.arg0 = image_info.image_base;
	}
#endif

}

void fconf_populate(const char *config_type, uintptr_t config)
{
	assert(config != 0UL);

	/* Check if the pointer to DTB is correct */
	if (fdt_check_header((void *)config) != 0) {
		ERROR("FCONF: Invalid DTB file passed for %s\n", config_type);
		panic();
	}

	INFO("FCONF: Reading %s firmware configuration file from: 0x%lx\n", config_type, config);

	/* Go through all registered populate functions */
	IMPORT_SYM(struct fconf_populator *, __FCONF_POPULATOR_START__, start);
	IMPORT_SYM(struct fconf_populator *, __FCONF_POPULATOR_END__, end);
	const struct fconf_populator *populator;

	for (populator = start; populator != end; populator++) {
		assert((populator->info != NULL) && (populator->populate != NULL));

		if (strcmp(populator->config_type, config_type) == 0) {
			INFO("FCONF: Reading firmware configuration information for: %s\n", populator->info);
			if (populator->populate(config) != 0) {
				/* TODO: handle property miss */
				panic();
			}
		}
	}
}
