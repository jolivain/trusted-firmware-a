/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

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


void bl1_plat_mboot_init(void)
{
	/* Initialize the communication channel between AP and RSS */
	(void)rss_comms_init(PLAT_RSS_AP_SND_MHU_BASE,
			     PLAT_RSS_AP_RCV_MHU_BASE);

	dpe_init();
}

void bl1_plat_mboot_finish(void)
{
	/* Nothing to do. */
}
