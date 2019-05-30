/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/tbbr/tbbr_img_def.h>
#include <lib/coreboot.h>
#include <plat/common/bl31_params.h>

struct bl31_params_v1 {
       param_header_t h;
       image_info_t *bl31_image_info;
       entry_point_info_t *bl32_ep_info;
       image_info_t *bl32_image_info;
       entry_point_info_t *bl33_ep_info;
       image_info_t *bl33_image_info;
};

void bl31_params_parse(u_register_t param, entry_point_info_t *bl32_ep_info_out,
		       entry_point_info_t *bl33_ep_info_out)
{
	struct bl31_params_v1 *v1 = (void *)(uintptr_t)param;
	bl_params_t *v2 = (void *)(uintptr_t)param;

	if (v1->h.version == PARAM_VERSION_1) {
		assert(v1->h.type == PARAM_BL31);
		if (bl32_ep_info_out)
			*bl32_ep_info_out = *v1->bl32_ep_info;
		if (bl33_ep_info_out)
			*bl33_ep_info_out = *v1->bl33_ep_info;
	} else {
		bl_params_node_t *node;
		assert(v2->h.version == PARAM_VERSION_2);
		assert(v2->h.type == PARAM_BL_PARAMS);
		for (node = v2->head; node; node = node->next_params_info) {
			if (node->image_id == BL32_IMAGE_ID)
				if (bl32_ep_info_out)
					*bl32_ep_info_out = *node->ep_info;
			if (node->image_id == BL33_IMAGE_ID)
				if (bl33_ep_info_out)
					*bl33_ep_info_out = *node->ep_info;
		}
	}
}

void bl31_plat_params_parse(u_register_t plat_param,
			    bl31_plat_param_handler_t handler)
{
	bl31_plat_param_header_t *param;

	for (param = (void *)plat_param; param; param = (void *)param->next) {
		if (handler && handler(param))
			continue;

		switch (param->type) {
#if COREBOOT
		case BL31_PLAT_PARAM_COREBOOT_TABLE:
			coreboot_table_setup((void *)
				((bl31_plat_param_uint64_t *)param)->value);
			break;
#endif
		default:
			ERROR("Ignore unknown BL31 platform parameter: 0x%llx",
			      param->type);
			break;
		}
	}
}
