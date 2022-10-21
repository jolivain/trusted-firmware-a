/*
 * Copyright (c) 2013-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <arch_features.h>
#include <bl1/bl1.h>
#include <bl2/bl2.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/console.h>
#include <drivers/fwu/fwu.h>
#include <lib/extensions/pauth.h>
#include <plat/common/platform.h>

#if SPMD_SPM_AT_SEL2
#include <common/desc_image_load.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <lib/fconf/fconf_tbbr_getter.h>
#endif
#include "bl2_private.h"

#ifdef __aarch64__
#define NEXT_IMAGE	"BL31"
#else
#define NEXT_IMAGE	"BL32"
#endif

#if BL2_AT_EL3
/*******************************************************************************
 * Setup function for BL2 when BL2_AT_EL3=1
 ******************************************************************************/
void bl2_el3_setup(u_register_t arg0, u_register_t arg1, u_register_t arg2,
		   u_register_t arg3)
{
	/* Perform early platform-specific setup */
	bl2_el3_early_platform_setup(arg0, arg1, arg2, arg3);

	/* Perform late platform-specific setup */
	bl2_el3_plat_arch_setup();

#if CTX_INCLUDE_PAUTH_REGS
	/*
	 * Assert that the ARMv8.3-PAuth registers are present or an access
	 * fault will be triggered when they are being saved or restored.
	 */
	assert(is_armv8_3_pauth_present());
#endif /* CTX_INCLUDE_PAUTH_REGS */
}
#else /* BL2_AT_EL3 */
/*******************************************************************************
 * Setup function for BL2 when BL2_AT_EL3=0
 ******************************************************************************/
void bl2_setup(u_register_t arg0, u_register_t arg1, u_register_t arg2,
	       u_register_t arg3)
{
	/* Perform early platform-specific setup */
	bl2_early_platform_setup2(arg0, arg1, arg2, arg3);

	/* Perform late platform-specific setup */
	bl2_plat_arch_setup();

#if CTX_INCLUDE_PAUTH_REGS
	/*
	 * Assert that the ARMv8.3-PAuth registers are present or an access
	 * fault will be triggered when they are being saved or restored.
	 */
	assert(is_armv8_3_pauth_present());
#endif /* CTX_INCLUDE_PAUTH_REGS */
}
#endif /* BL2_AT_EL3 */

#if SPMD_SPM_AT_SEL2
/*
 * BL2 utility function to initialize dynamic configuration specified by
 * FW_CONFIG. Populate the bl_mem_params_node_t of other FW_CONFIGs if
 * specified in FW_CONFIG.
 */
void qemu_bl2_dyn_cfg_init(void)
{
	unsigned int i;
	bl_mem_params_node_t *cfg_mem_params = NULL;
	uintptr_t image_base;
	uint32_t image_size;
	const unsigned int config_ids[] = {
			HW_CONFIG_ID,
			SOC_FW_CONFIG_ID,
			NT_FW_CONFIG_ID,
			TOS_FW_CONFIG_ID
	};

	const struct dyn_cfg_dtb_info_t *dtb_info;

	/* Iterate through all the fw config IDs */
	for (i = 0; i < ARRAY_SIZE(config_ids); i++) {
		cfg_mem_params = get_bl_mem_params_node(config_ids[i]);
		if (cfg_mem_params == NULL) {
			INFO("%s config_id %d in bl_mem_params_node\n",
				"Couldn't find", config_ids[i]);
			continue;
		}

		/* Get the config load address and size from FW_CONFIG */
		dtb_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, config_ids[i]);
		if (dtb_info == NULL) {
			INFO("%sconfig_id %d load info in FW_CONFIG\n",
				"Couldn't find ", config_ids[i]);
			continue;
		}

		image_base = dtb_info->config_addr;
		image_size = dtb_info->config_max_size;

		/*
		 * Do some runtime checks on the load addresses of soc_fw_config,
		 * tos_fw_config, nt_fw_config. This is not a comprehensive check
		 * of all invalid addresses but to prevent trivial porting errors.
		 */
		if (config_ids[i] != HW_CONFIG_ID) {

			if (check_uptr_overflow(image_base, image_size)) {
				continue;
			}
#ifdef	BL31_BASE
			/* Ensure the configs don't overlap with BL31 */
			if ((image_base >= BL31_BASE) &&
			    (image_base <= BL31_LIMIT)) {
				continue;
			}
#endif
			/* Ensure the configs are loaded in a valid address */
			if (image_base < BL_RAM_BASE) {
				continue;
			}
#ifdef BL32_BASE
			/*
			 * If BL32 is present, ensure that the configs don't
			 * overlap with it.
			 */
			if ((image_base >= BL32_BASE) &&
			    (image_base <= BL32_LIMIT)) {
				continue;
			}
#endif
		}

		cfg_mem_params->image_info.image_base = image_base;
		cfg_mem_params->image_info.image_max_size = (uint32_t)image_size;

		/*
		 * Remove the IMAGE_ATTRIB_SKIP_LOADING attribute from
		 * HW_CONFIG or FW_CONFIG nodes
		 */
		cfg_mem_params->image_info.h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;
	}
}
#endif

/*******************************************************************************
 * The only thing to do in BL2 is to load further images and pass control to
 * next BL. The memory occupied by BL2 will be reclaimed by BL3x stages. BL2
 * runs entirely in S-EL1.
 ******************************************************************************/
void bl2_main(void)
{
	entry_point_info_t *next_bl_ep_info;

	NOTICE("BL2: %s\n", version_string);
	NOTICE("BL2: %s\n", build_message);

	/* Perform remaining generic architectural setup in S-EL1 */
	bl2_arch_setup();

#if PSA_FWU_SUPPORT
	fwu_init();
#endif /* PSA_FWU_SUPPORT */

	crypto_mod_init();

	/* Initialize authentication module */
	auth_mod_init();

	/* Initialize the Measured Boot backend */
	bl2_plat_mboot_init();

	/* Initialize boot source */
	bl2_plat_preload_setup();

#if SPMD_SPM_AT_SEL2
	qemu_bl2_dyn_cfg_init();
#endif
	/* Load the subsequent bootloader images. */
	next_bl_ep_info = bl2_load_images();

	/* Teardown the Measured Boot backend */
	bl2_plat_mboot_finish();

#if !BL2_AT_EL3 && !ENABLE_RME
#ifndef __aarch64__
	/*
	 * For AArch32 state BL1 and BL2 share the MMU setup.
	 * Given that BL2 does not map BL1 regions, MMU needs
	 * to be disabled in order to go back to BL1.
	 */
	disable_mmu_icache_secure();
#endif /* !__aarch64__ */

	console_flush();

#if ENABLE_PAUTH
	/*
	 * Disable pointer authentication before running next boot image
	 */
	pauth_disable_el1();
#endif /* ENABLE_PAUTH */

	/*
	 * Run next BL image via an SMC to BL1. Information on how to pass
	 * control to the BL32 (if present) and BL33 software images will
	 * be passed to next BL image as an argument.
	 */
	smc(BL1_SMC_RUN_IMAGE, (unsigned long)next_bl_ep_info, 0, 0, 0, 0, 0, 0);
#else /* if BL2_AT_EL3 || ENABLE_RME */
	NOTICE("BL2: Booting " NEXT_IMAGE "\n");
	print_entry_point_info(next_bl_ep_info);
	console_flush();

#if ENABLE_PAUTH
	/*
	 * Disable pointer authentication before running next boot image
	 */
	pauth_disable_el3();
#endif /* ENABLE_PAUTH */

	bl2_run_next_image(next_bl_ep_info);
#endif /* BL2_AT_EL3 && ENABLE_RME */
}
