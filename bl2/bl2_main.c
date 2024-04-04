/*
 * Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <arch_features.h>
#include <bl1/bl1.h>
#include <bl2/bl2.h>
#include <common/bl_common.h>
#include <common/build_message.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/console.h>
#include <drivers/fwu/fwu.h>
#include <lib/bootmarker_capture.h>
#include <lib/extensions/pauth.h>
#if BL2_ENABLE_CONFIG_LOAD
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#endif
#include <lib/pmf/pmf.h>
#include <plat/common/platform.h>

#include "bl2_private.h"

#ifdef __aarch64__
#define NEXT_IMAGE	"BL31"
#else
#define NEXT_IMAGE	"BL32"
#endif

#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_REGISTER_SERVICE(bl_svc, PMF_RT_INSTR_SVC_ID,
		BL_TOTAL_IDS, PMF_DUMP_ENABLE);
#endif

#if RESET_TO_BL2
/*******************************************************************************
 * Setup function for BL2 when RESET_TO_BL2=1
 ******************************************************************************/
void bl2_el3_setup(u_register_t arg0, u_register_t arg1, u_register_t arg2,
		   u_register_t arg3)
{
	/* Enable early console if EARLY_CONSOLE flag is enabled */
	plat_setup_early_console();

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
#else /* RESET_TO_BL2 */

/*******************************************************************************
 * Setup function for BL2 when RESET_TO_BL2=0
 ******************************************************************************/
void bl2_setup(u_register_t arg0, u_register_t arg1, u_register_t arg2,
	       u_register_t arg3)
{
	/* Enable early console if EARLY_CONSOLE flag is enabled */
	plat_setup_early_console();

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
#endif /* RESET_TO_BL2 */

/*********************************************************************************
 * FW CONFIG load function for BL2 when RESET_TO_BL2=1 && BL2_ENABLE_CONFIG_LOAD=1
 *********************************************************************************/
#if BL2_ENABLE_CONFIG_LOAD
static void bl2_el3_config_load(void)
{
	int ret = -1;
	const struct dyn_cfg_dtb_info_t *fw_config_info;

	/* Set global DTB info for fixed fw_config information */
	set_config_info(PLAT_FW_CONFIG_BASE, ~0UL, PLAT_FW_CONFIG_MAX_SIZE, FW_CONFIG_ID);

	/* Fill the device tree information struct with the info from the config dtb */
	ret = fconf_load_config(FW_CONFIG_ID);
	if (ret < 0) {
		ERROR("Loading of FW_CONFIG failed %d\n", ret);
		plat_error_handler(ret);
	}

	/*
	 * FW_CONFIG loaded successfully. Check the FW_CONFIG device tree parsing
	 * is successful.
	 */
	fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, FW_CONFIG_ID);
	if (fw_config_info == NULL) {
		ret = -1;
		ERROR("Invalid FW_CONFIG address\n");
		plat_error_handler(ret);
	}
	ret = fconf_populate_dtb_registry(fw_config_info->config_addr);
	if (ret < 0) {
		ERROR("Parsing of FW_CONFIG failed %d\n", ret);
		plat_error_handler(ret);
	}
}
#endif /* BL2_ENABLE_CONFIG_LOAD */

/*******************************************************************************
 * The only thing to do in BL2 is to load further images and pass control to
 * next BL. The memory occupied by BL2 will be reclaimed by BL3x stages. BL2
 * runs entirely in S-EL1.
 ******************************************************************************/
void bl2_main(void)
{
	entry_point_info_t *next_bl_ep_info;

#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_CAPTURE_TIMESTAMP(bl_svc, BL2_ENTRY, PMF_CACHE_MAINT);
#endif

	NOTICE("BL2: %s\n", build_version_string);
	NOTICE("BL2: %s\n", build_message);

	/* Perform remaining generic architectural setup in S-EL1 */
	bl2_arch_setup();

	/* Initialize authentication module.
	 *
	 * Initialization of the authentication module should occur before
	 * loading the FW_CONFIG to authenticate the complete CoT.
	 */
	auth_mod_init();

#if BL2_ENABLE_CONFIG_LOAD
	/* Load the FW_CONFIG images before the bootloader images*/
	bl2_el3_config_load();
#endif /* BL2_ENABLE_CONFIG_LOAD */

#if PSA_FWU_SUPPORT
	fwu_init();
#endif /* PSA_FWU_SUPPORT */

	crypto_mod_init();

	/* Initialize the Measured Boot backend */
	bl2_plat_mboot_init();

	/* Initialize boot source */
	bl2_plat_preload_setup();

	/* Load the subsequent bootloader images. */
	next_bl_ep_info = bl2_load_images();

	/* Teardown the Measured Boot backend */
	bl2_plat_mboot_finish();

#if !BL2_RUNS_AT_EL3
#ifndef __aarch64__
	/*
	 * For AArch32 state BL1 and BL2 share the MMU setup.
	 * Given that BL2 does not map BL1 regions, MMU needs
	 * to be disabled in order to go back to BL1.
	 */
	disable_mmu_icache_secure();
#endif /* !__aarch64__ */

#if ENABLE_PAUTH
	/*
	 * Disable pointer authentication before running next boot image
	 */
	pauth_disable_el1();
#endif /* ENABLE_PAUTH */

#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_CAPTURE_TIMESTAMP(bl_svc, BL2_EXIT, PMF_CACHE_MAINT);
#endif

	console_flush();

	/*
	 * Run next BL image via an SMC to BL1. Information on how to pass
	 * control to the BL32 (if present) and BL33 software images will
	 * be passed to next BL image as an argument.
	 */
	smc(BL1_SMC_RUN_IMAGE, (unsigned long)next_bl_ep_info, 0, 0, 0, 0, 0, 0);
#else /* if BL2_RUNS_AT_EL3 */

	NOTICE("BL2: Booting " NEXT_IMAGE "\n");
	print_entry_point_info(next_bl_ep_info);
#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_CAPTURE_TIMESTAMP(bl_svc, BL2_EXIT, PMF_CACHE_MAINT);
#endif
	console_flush();

#if ENABLE_PAUTH
	/*
	 * Disable pointer authentication before running next boot image
	 */
	pauth_disable_el3();
#endif /* ENABLE_PAUTH */

	bl2_run_next_image(next_bl_ep_info);
#endif /* BL2_RUNS_AT_EL3 */
}
