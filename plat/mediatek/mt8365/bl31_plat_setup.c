/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/desc_image_load.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <mcusys/v1/mcucfg.h>
#include <mt_gic_v3.h>
#include <plat_params.h>

#include <plat_private.h>
#include <platform_def.h>
#include <spm/mt_spm.h>

#define NR_INT_POL_CTL 20

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ?
			  &bl33_image_ep_info : &bl32_image_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;

	return NULL;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	params_early_setup(arg1);
	mtk_console_register();
	bl31_params_parse_helper(arg0, &bl32_image_ep_info, &bl33_image_ep_info);
}

static void clear_sec_pol_ctl_en(void)
{
	unsigned int i;

	for (i = 0; i <= NR_INT_POL_CTL - 1; i++)
		mmio_write_32((SEC_POL_CTL_EN0 + (i * 4)), 0);

	dsb();
}

void bl31_platform_setup(void)
{
	mmio_write_32(MCU_ALL_PWR_ON, 1);
	mmio_write_32(MP0_RW_RSVD0, 1);

	generic_delay_timer_init();

	/* Initialize the GIC driver, CPU and distributor interfaces */
	mt_gic_driver_init();
	mt_gic_init();
	clear_sec_pol_ctl_en();

	spm_boot_init();
}

void bl31_plat_arch_setup(void)
{
	plat_configure_mmu_el3(BL31_BASE,
			       BL31_END - BL31_BASE,
			       BL_CODE_BASE,
			       BL_CODE_END);
}
