/*
 * Copyright (c) 2023, The ChromiumOS Authors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/psci/psci.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/oem/chromeos/widevine_smc_handlers.h>
#include <tools_share/uuid.h>

#define CROS_OEM_TPM_AUTH_PK_MAX_LEN 128
#define CROS_OEM_HUK_LEN 32
#define CROS_OEM_WIDEVINE_DICE_LEN 1024
#define CROS_OEM_WIDEVINE_TA_KEY_LEN 32

static uint8_t cros_oem_tpm_auth_pk_buffer[CROS_OEM_TPM_AUTH_PK_MAX_LEN];
static uint8_t cros_oem_huk_buffer[CROS_OEM_HUK_LEN];
static uint8_t cros_oem_widevine_dice_buffer[CROS_OEM_WIDEVINE_DICE_LEN];
static uint8_t cros_oem_widevine_ta_key_len_buffer[CROS_OEM_WIDEVINE_TA_KEY_LEN];

cros_oem_data cros_oem_tpm_auth_pk = {
	.buffer = cros_oem_tpm_auth_pk_buffer,
	.max_length = sizeof(cros_oem_tpm_auth_pk_buffer),
};

cros_oem_data cros_oem_huk = {
	.buffer = cros_oem_huk_buffer,
	.max_length = sizeof(cros_oem_huk_buffer),
};

cros_oem_data cros_oem_widevine_dice = {
	.buffer = cros_oem_widevine_dice_buffer,
	.max_length = sizeof(cros_oem_widevine_dice_buffer),
};

cros_oem_data cros_oem_widevine_ta_key = {
	.buffer = cros_oem_widevine_ta_key_len_buffer,
	.max_length = sizeof(cros_oem_widevine_ta_key_len_buffer),
};

static uintptr_t cros_write_data(cros_oem_data *data, u_register_t length,
				 u_register_t data_pa, void *handle)
{
	uintptr_t aligned_pa;
	uintptr_t aligned_va = 0;
	uintptr_t aligned_size;
	uintptr_t data_va;
	int32_t rc;

	if (data->is_set) {
		SMC_RET1(handle, PSCI_E_ALREADY_ON);
	}

	if ((data->length + length) > data->max_length) {
		SMC_RET1(handle, PSCI_E_INVALID_PARAMS);
	}

	aligned_pa = page_align(data_pa, DOWN);
	aligned_va = aligned_pa;
	aligned_size = page_align(length + (data_pa - aligned_pa), UP);

	/*
	 * We do not validate the passed in address because we are trusting the
	 * non-secure world at this point still.
	 */
	rc = mmap_add_dynamic_region(aligned_pa, aligned_va, aligned_size,
				     MT_MEMORY | MT_RO | MT_NS);
	if (rc != 0) {
		SMC_RET1(handle, SMC_UNK);
	}

	data_va = data_pa - aligned_pa + aligned_va;

	memcpy(&data->buffer[data->length], (void *)data_va, length);
	data->length = length;
	data->is_set = true;

	mmap_remove_dynamic_region(aligned_va, aligned_size);
	SMC_RET1(handle, SMC_OK);
}

/* Handler for servicing specific SMC calls. */
uintptr_t cros_oem_svc_smc_handler(uint32_t smc_fid, u_register_t x1,
				   u_register_t x2, u_register_t x3,
				   u_register_t x4, void *cookie, void *handle,
				   u_register_t flags)
{
	switch (smc_fid) {
	case CROS_OEM_SMC_DRM_SET_TPM_AUTH_PUB_FUNC_ID:
		return cros_write_data(&cros_oem_tpm_auth_pk, x1, x2, handle);
	case CROS_OEM_SMC_DRM_SET_DEVICE_ID_FUNC_ID:
		return cros_write_data(&cros_oem_huk, x1, x2, handle);
	case CROS_OEM_SMC_DRM_SET_WIDEVINE_DICE_FUNC_ID:
		return cros_write_data(&cros_oem_widevine_dice, x1, x2, handle);
	case CROS_OEM_SMC_DRM_SET_WIDEVINE_TA_KEY_FUNC_ID:
		return cros_write_data(&cros_oem_widevine_ta_key, x1, x2,
				       handle);
	default:
		WARN("Unimplemented OEM Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/*
 * Top level handler for SMC calls, dispatched if they are OEM specific.
 */
uintptr_t oem_svc_smc_handler(uint32_t smc_fid, u_register_t x1,
			      u_register_t x2, u_register_t x3, u_register_t x4,
			      void *cookie, void *handle, u_register_t flags)
{
	/*
	 * Dispatch OEM calls to OEM Common handler and return its return value
	 */
	if (is_cros_oem_smc(smc_fid)) {
		return cros_oem_svc_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
						handle, flags);
	}

	WARN("Unimplemented OEM Service Call: 0x%x\n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

/* Register OEM Service Calls as runtime service */
DECLARE_RT_SVC(cros_oem_smc_drm_svc, OEN_OEM_START, OEN_OEM_END, SMC_TYPE_FAST,
	       NULL, oem_svc_smc_handler);
