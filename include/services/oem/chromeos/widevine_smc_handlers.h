/*
 * Copyright (c) 2023, The ChromiumOS Authors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CROS_WIDEVINE_SMC_HANDLERS_H
#define CROS_WIDEVINE_SMC_HANDLERS_H

#include <lib/smccc.h>

/*******************************************************************************
 * Defines for CrOS OEM Service queries
 ******************************************************************************/

/* 0xC300C050 - 0xC300C05F are CrOS OEM service calls */
#define CROS_OEM_SMC_ID 0xC050
#define CROS_OEM_SMC_CALL_ID(func_num)                                         \
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) |                                \
	 ((SMC_64) << FUNCID_CC_SHIFT) | (OEN_OEM_START << FUNCID_OEN_SHIFT) | \
	 (CROS_OEM_SMC_ID) | ((func_num)&FUNCID_NUM_MASK))

enum cros_drm_set {
	CROS_DRM_SET_TPM_AUTH_PUB = 0u,
	CROS_DRM_SET_DEVICE_ID = 1u,
	CROS_DRM_SET_WIDEVINE_DICE = 2u,
	CROS_DRM_SET_WIDEVINE_TA_KEY = 3u,
};

/*******************************************************************************
 * Defines for runtime services func ids
 ******************************************************************************/

/* Sets the TPM auth public key.
 * |x1| is the length of the data, |x2| is the physical address of the data.
 */
#define CROS_OEM_SMC_DRM_SET_TPM_AUTH_PUB_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_DRM_SET_TPM_AUTH_PUB)

/* Sets the device unique identifier.
 * |x1| is the length of the data, |x2| is the physical address of the data.
 */
#define CROS_OEM_SMC_DRM_SET_DEVICE_ID_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_DRM_SET_DEVICE_ID)

/* Sets the widevine root of trust certificate chain.
 * |x1| is the length of the data, |x2| is the physical address of the data.
 */
#define CROS_OEM_SMC_DRM_SET_WIDEVINE_DICE_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_DRM_SET_WIDEVINE_DICE)

/* Sets the widevine TA key.
 * |x1| is the length of the data, |x2| is the physical address of the data.
 */
#define CROS_OEM_SMC_DRM_SET_WIDEVINE_TA_KEY_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_DRM_SET_WIDEVINE_TA_KEY)

#define is_cros_oem_smc(_call_id) (((_call_id)&0xFFF0U) == CROS_OEM_SMC_ID)

typedef struct cros_oem_data {
	uint8_t *buffer;
	const uint32_t max_length;
	uint32_t length;
	uint8_t is_set;
} cros_oem_data;

extern cros_oem_data cros_oem_tpm_auth_pk;

extern cros_oem_data cros_oem_huk;

extern cros_oem_data cros_oem_widevine_dice;

extern cros_oem_data cros_oem_widevine_ta_key;

#endif /* CROS_WIDEVINE_SMC_HANDLERS_H */
