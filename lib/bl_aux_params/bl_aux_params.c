/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/coreboot.h>
#include <lib/bl_aux_params/bl_aux_params.h>

void bl_aux_params_parse(u_register_t head,
			 bl_aux_param_handler_t handler)
{
	bl_aux_param_header_t *p;

	for (p = (void *)head; p; p = (void *)(uintptr_t)p->next) {
		if (handler && handler(p))
			continue;

		switch (p->type) {
#if COREBOOT
		case BL_AUX_PARAM_COREBOOT_TABLE:
			coreboot_table_setup((void *)(uintptr_t)
				((bl_aux_param_uint64_t *)p)->value);
			break;
#endif
		default:
			ERROR("Ignoring unknown BL aux parameter: 0x%llx",
			      p->type);
			break;
		}
	}
}
