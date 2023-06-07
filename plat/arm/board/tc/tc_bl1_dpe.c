/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/arm/css/sds.h>
#include <drivers/arm/rss_comms.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
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

/* Effective timeout of 10000 ms */
#define RSS_DPE_BOOT_10US_RETRIES		1000000
#define TC2_SDS_DPE_CTX_HANDLE_STRUCT_ID	0x0000000A

/* Context handle is meant to be used by BL2. Sharing it via TB_FW_CONFIG */
static int child_ctx_handle;

void plat_dpe_share_context_handle(int *ctx_handle)
{
	child_ctx_handle = *ctx_handle;
}

void plat_dpe_get_context_handle(int *ctx_handle)
{
	int retry = RSS_DPE_BOOT_10US_RETRIES;
	int ret;

	/* Initialize System level generic or SP804 timer */
	generic_delay_timer_init();

	/* Check the initialization of the Shared Data Storage area between RSS
	 * and AP. Since AP_BL1 is executed first then a bit later the RSS
	 * runtime, which initialize this area, therefore AP needs to check it
	 * in a loop until it gets written by RSS Secure Runtime.
	 */
	VERBOSE("Waiting for DPE service initialization in RSS Secure Runtime\n");
	while (retry > 0) {
		ret = sds_init(SDS_RSS_AP_REGION_ID);
		if (ret != SDS_OK) {
			udelay(10);
			retry--;
		} else {
			break;
		}
	}

	if (retry == 0) {
		ERROR("DPE init timeout\n");
		plat_panic_handler();
	} else {
		VERBOSE("DPE init succeeded in %dms.\n",
			(RSS_DPE_BOOT_10US_RETRIES - retry) / 100);
	}

	/* TODO: call this in a loop to avoid reading unfinished data */
	ret = sds_struct_read(SDS_RSS_AP_REGION_ID,
			      TC2_SDS_DPE_CTX_HANDLE_STRUCT_ID,
			      0,
			      ctx_handle,
			      sizeof(*ctx_handle),
			      SDS_ACCESS_MODE_NON_CACHED);
	if (ret != SDS_OK) {
		ERROR("Unable to get DPE context handle from SDS area\n");
		plat_panic_handler();
	}

	VERBOSE("Get DPE context handle, client_id: 0x%x nonce: 0x%x\n",
		((struct dpe_context_handle *)ctx_handle)->client_id,
		((struct dpe_context_handle *)ctx_handle)->nonce);
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
