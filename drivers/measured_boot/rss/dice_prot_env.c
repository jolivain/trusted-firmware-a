/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/rss/dice_prot_env.h>
#include <lib/psa/dice_protection_environment.h>
#include <platform_def.h>
#include <psa/crypto_types.h>
#include <psa/crypto_values.h>


#define DPE_ALG_SHA512 0
#define DPE_ALG_SHA384 1
#define DPE_ALG_SHA256 2

#if DPE_ALG_ID == DPE_ALG_SHA512
#define	CRYPTO_MD_ID		CRYPTO_MD_SHA512
#define PSA_CRYPTO_MD_ID	PSA_ALG_SHA_512
#elif DPE_ALG_ID == DPE_ALG_SHA384
#define	CRYPTO_MD_ID		CRYPTO_MD_SHA384
#define PSA_CRYPTO_MD_ID	PSA_ALG_SHA_384
#elif DPE_ALG_ID == DPE_ALG_SHA256
#define	CRYPTO_MD_ID		CRYPTO_MD_SHA256
#define PSA_CRYPTO_MD_ID	PSA_ALG_SHA_256
#else
#  error Invalid DPE hash algorithm.
#endif /* DPE_ALG_ID */

/* Pointer to struct dpe_metadata */
static struct dpe_metadata *plat_metadata_ptr;

static int initial_context_handle;

void dpe_init(void)
{
	/* At this point it is expected that communication channel over MHU
	 * is already initialised by platform init.
	 */
	struct dpe_metadata *metadata_ptr;

	/* Get pointer to platform's dpe_metadata structure */
	plat_metadata_ptr = plat_dpe_get_metadata();
	assert(plat_metadata_ptr != NULL);

	/* Use a local variable to preserve the value of the global pointer */
	metadata_ptr = plat_metadata_ptr;

	/* Init the non-const members of the metadata structure */
	while (metadata_ptr->id != DPE_INVALID_ID) {
		metadata_ptr->sw_type_size =
			strlen((const char *)&metadata_ptr->sw_type) + 1;
		metadata_ptr++;
	}

	/* TODO: Obtain ctx handle and assign it to initial_context_handle */
}

int dpe_measure_and_record(uintptr_t data_base, uint32_t data_size,
			   uint32_t data_id)
{
	const struct dpe_metadata *metadata_ptr = plat_metadata_ptr;
	static int current_context_handle;
	DiceInputValues dice_inputs = { 0 };
	int parent_context_handle;
	int child_context_handle;
	dpe_error_t ret;
	int rc;

	/* Get the metadata associated with this image. */
	while ((metadata_ptr->id != DPE_INVALID_ID) &&
	       (metadata_ptr->id != data_id)) {
		metadata_ptr++;
	}

	/* If image is not present in metadata array then skip */
	if (metadata_ptr->id == DPE_INVALID_ID) {
		return 0;
	}

	/* Calculate hash */
	rc = crypto_mod_calc_hash(CRYPTO_MD_ID,
				  (void *)data_base, data_size,
				   dice_inputs.code_hash);
	if (rc != 0) {
		return rc;
	}

	/* TODO: Fill up the DICE inputs from the platform metadata array */

	/* Only at the first call */
	if (current_context_handle == 0) {
		current_context_handle = initial_context_handle;
	}

	VERBOSE("Calling dpe_derive_child, image_id: %d\n", metadata_ptr->id);
	ret =  dpe_derive_child(current_context_handle,
				metadata_ptr->retain_parent_context,
				metadata_ptr->allow_child_to_derive,
				metadata_ptr->create_certificate,
				&dice_inputs,
				&child_context_handle,
				&parent_context_handle);
	if (ret == DPE_NO_ERROR) {
		current_context_handle = parent_context_handle;
		if (metadata_ptr->allow_child_to_derive == true) {
			/* Add child_context_handle to dtb to handover to child
			 * component: e.g: BL2, BL33
			 */
			VERBOSE("Share child_context_handle over dtb: 0x%x\n",
				child_context_handle);
			plat_dpe_share_context_handle(&child_context_handle);
		}
	} else {
		ERROR("dpe_derive_child failed\n");
	}

	return ret;
}
