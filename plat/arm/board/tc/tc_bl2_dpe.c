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

/* The content and the values of this array depends on:
 * - build config: Which components are loaded: SPMD, TOS, SPx, etc ?
 * - boot order: the last element in a layer should be treated differently.
 */
struct dpe_metadata tc_dpe_metadata[] = {
	{
		.id = BL31_IMAGE_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = BL31_IMAGE_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = BL32_IMAGE_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = BL31_IMAGE_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
// TODO: Strings are too long for sw_type array, increase SW_TYPE_MAX_SIZE
//	{
//		.id = BL32_EXTRA1_IMAGE_ID,
//		.signer_id_size = SIGNER_ID_MIN_SIZE,
//		.sw_type = BL32_EXTRA1_IMAGE_STRING,
//		.allow_child_to_derive = false,
//		.retain_parent_context = true,
//		.create_certificate = false },
//	{
//		.id = BL32_EXTRA2_IMAGE_ID,
//		.signer_id_size = SIGNER_ID_MIN_SIZE,
//		.sw_type = BL32_EXTRA2_IMAGE_STRING,
//		.allow_child_to_derive = false,
//		.retain_parent_context = true,
//		.create_certificate = false },
	{
		.id = BL33_IMAGE_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = BL33_IMAGE_STRING,
		.allow_child_to_derive = true,
		.retain_parent_context = true,
		.create_certificate = false },

	/* TODO: This might rather belongs to the Hypervisor layer, decide later
	 * But for now, set the create_certificate to true to indicate the end
	 * of the platform layer.
	 */
	{
		.id = HW_CONFIG_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = HW_CONFIG_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = true },
	{
		.id = NT_FW_CONFIG_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = NT_FW_CONFIG_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = SCP_BL2_IMAGE_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = SCP_BL2_IMAGE_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = SOC_FW_CONFIG_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = SOC_FW_CONFIG_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = TOS_FW_CONFIG_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = TOS_FW_CONFIG_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
#if defined(SPD_spmd)
	{
		.id = SP_PKG1_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = SP1_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = SP_PKG2_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = SP2_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = SP_PKG3_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = SP3_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = SP_PKG4_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = SP4_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = SP_PKG5_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = SP5_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = SP_PKG6_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = SP6_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = SP_PKG7_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = SP7_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },
	{
		.id = SP_PKG8_ID,
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = SP8_STRING,
		.allow_child_to_derive = false,
		.retain_parent_context = true,
		.create_certificate = false },

#endif
	{
		.id = DPE_INVALID_ID }
};

/* Context handle is meant to be used by BL33. Sharing it via NT_FW_CONFIG */
static int child_ctx_handle;

void plat_dpe_share_context_handle(int *ctx_handle)
{
	child_ctx_handle = *ctx_handle;
}

void bl2_plat_mboot_init(void)
{
	/* Initialize the communication channel between AP and RSS */
	(void)rss_comms_init(PLAT_RSS_AP_SND_MHU_BASE,
			     PLAT_RSS_AP_RCV_MHU_BASE);

	dpe_init();
}

void bl2_plat_mboot_finish(void)
{
	int rc;

	VERBOSE("Share DPE context handle with BL33: 0x%x\n", child_ctx_handle);
	rc = arm_set_nt_fw_info(&child_ctx_handle);
	if (rc != 0) {
		ERROR("Unable to set DPE context handle in NT_FW_CONFIG\n");
		/*
		 * It is a fatal error because on TC platform, BL33 software
		 * assumes that a valid DPE context_handle is passed through
		 * the DTB object by BL2.
		 */
		plat_panic_handler();
	}
}
