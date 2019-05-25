/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/coreboot.h>
#include <plat/common/bl31_params.h>

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
