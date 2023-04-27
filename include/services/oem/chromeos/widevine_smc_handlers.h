/*
 * Copyright (c) 2023, The ChromiumOS Authors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CROS_WIDEVINE_SMC_HANDLERS_H
#define CROS_WIDEVINE_SMC_HANDLERS_H

#include <lib/smccc.h>

/*******************************************************************************
 * Defines for OEM Service queries
 ******************************************************************************/

/* 0x83000000 - 0x8300FEFF is OEM service calls */
#define CROS_OEM_SVC_CALL_COUNT U(0X8300FF00)
#define CROS_OEM_SVC_UID U(0X8300FF01)
/* 0x8300ff02 is reserved */
#define CROS_OEM_SVC_VERSION U(0X8300FF03)
/* 0x8300ff04 - 0x8300FFFF is reserved for future expansion */

/* CrOS service calls version */
#define CROS_OEM_VERSION_MAJOR U(0x0)
#define CROS_OEM_VERSION_MINOR U(0x1)

#define CROS_OEM_SMC_CALL_ID(func_num)                                         \
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) |                                \
	 ((SMC_32) << FUNCID_CC_SHIFT) | (OEN_OEM_START << FUNCID_OEN_SHIFT) | \
	 ((func_num) & FUNCID_NUM_MASK))

/*******************************************************************************
 * Defines for runtime services func ids
 ******************************************************************************/

/* Gets the TPM auth public key. */
#define CROS_OEM_SMC_GET_TPM_AUTH_PUB U(0)
#define CROS_OEM_SMC_GET_TPM_AUTH_PUB_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_OEM_SMC_GET_TPM_AUTH_PUB)

/* Sets the TPM auth public key. */
#define CROS_OEM_SMC_SET_TPM_AUTH_PUB U(1)
#define CROS_OEM_SMC_SET_TPM_AUTH_PUB_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_OEM_SMC_SET_TPM_AUTH_PUB)

/* Gets the device unique identifier. */
#define CROS_OEM_SMC_GET_DEVICE_ID U(2)
#define CROS_OEM_SMC_GET_DEVICE_ID_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_OEM_SMC_GET_DEVICE_ID)

/* Sets the device unique identifier. */
#define CROS_OEM_SMC_SET_DEVICE_ID U(3)
#define CROS_OEM_SMC_SET_DEVICE_ID_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_OEM_SMC_SET_DEVICE_ID)

/* Gets the widevine root of trust. */
#define CROS_OEM_SMC_GET_WIDEVINE_ROT U(4)
#define CROS_OEM_SMC_GET_WIDEVINE_ROT_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_OEM_SMC_GET_WIDEVINE_ROT)

/* Sets the widevine root of trust. */
#define CROS_OEM_SMC_SET_WIDEVINE_ROT U(5)
#define CROS_OEM_SMC_SET_WIDEVINE_ROT_FUNC_ID \
	CROS_OEM_SMC_CALL_ID(CROS_OEM_SMC_SET_WIDEVINE_ROT)

/*
 * Number of OEM calls (above) implemented.
 */
#define CROS_OEM_SVC_NUM_CALLS U(6)

/* The macros below are used to identify OEM calls from the SMC function ID */
/* SMC32 ID range from 0x83000000 to 0x83000FFF */
/* SMC64 ID range from 0xC3000000 to 0xC3000FFF */
#define OEM_FID_MASK U(0XF000)
#define OEM_FID_VALUE U(0)
#define is_oem_fid(_fid) (((_fid)&OEM_FID_MASK) == OEM_FID_VALUE)

#endif /* CROS_WIDEVINE_SMC_HANDLERS_H */
