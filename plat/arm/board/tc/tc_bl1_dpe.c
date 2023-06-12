/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/arm/rss_comms.h>
#include <drivers/measured_boot/rss/dice_prot_env.h>
#include <lib/psa/measured_boot.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

struct dpe_metadata tc_dpe_metadata[] = {
	{
		.id = FW_CONFIG_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = FW_CONFIG_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = TB_FW_CONFIG_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = TB_FW_CONFIG_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = BL2_IMAGE_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = BL2_IMAGE_STRING,
		.allow_child_to_derive = true,
		.retain_parent_context = false,
		.create_certificate = false },
	{
		.id = DPE_INVALID_ID }
};

/* Context handle is meant to be used by BL2. Sharing it via TB_FW_CONFIG */
static int child_ctx_handle;

void plat_dpe_share_context_handle(int *ctx_handle)
{
	child_ctx_handle = *ctx_handle;
}

void bl1_plat_mboot_init(void)
{
	/* Initialize the communication channel between AP and RSS */
	(void)rss_comms_init(PLAT_RSS_AP_SND_MHU_BASE,
			     PLAT_RSS_AP_RCV_MHU_BASE);

	dpe_init();
}

void bl1_plat_mboot_finish(void)
{
	int rc;

	VERBOSE("Share DPE context handle with BL2: 0x%x\n", child_ctx_handle);
	rc = arm_set_tb_fw_info(&child_ctx_handle);
	if (rc != 0) {
		ERROR("Unable to set DPE context handle in TB_FW_CONFIG\n");
		/*
		 * It is a fatal error because on TC platform, BL2 software
		 * assumes that a valid DPE context_handle is passed through
		 * the DTB object by BL1.
		 */
		plat_panic_handler();
	}
}
