/*
 * Copyright (c) 2023, The ChromiumOS Authors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <plat/common/platform.h>
#include <tools_share/uuid.h>

#include <cros_widevine_smc_handlers.h>

/* Cros OEM Service UUID */
DEFINE_SVC_UUID2(cros_oem_smc_svc_uid, 0x8f68bba4, 0xb39c, 0x4dbc, 0x80, 0x3e,
                 0xf6, 0x09, 0xb5, 0x7e, 0x5e, 0x94);

#define CROS_OEM_TPM_AUTH_PK_MAX_LEN 128
#define CROS_OEM_HUK_LEN 32
#define CROS_OEM_WIDEVINE_ROT_LEN 1024
#define CROS_OEM_END_OF_DATA (1 << 30)

typedef struct cros_oem_data {
  uint8_t *buffer;
  uint32_t max_length;
  uint32_t length;
  uint8_t is_set;
} cros_oem_data;

static uint8_t cros_oem_tpm_auth_pk_buffer[CROS_OEM_TPM_AUTH_PK_MAX_LEN];
static uint8_t cros_oem_huk_buffer[CROS_OEM_HUK_LEN];
static uint8_t cros_oem_widevine_rot_buffer[CROS_OEM_WIDEVINE_ROT_LEN];

static cros_oem_data cros_oem_tpm_auth_pk = {
    .buffer = cros_oem_tpm_auth_pk_buffer,
    .max_length = CROS_OEM_TPM_AUTH_PK_MAX_LEN,
    .length = 0,
    .is_set = 0,
};

static cros_oem_data cros_oem_huk = {
    .buffer = cros_oem_huk_buffer,
    .max_length = CROS_OEM_HUK_LEN,
    .length = 0,
    .is_set = 0,
};

static cros_oem_data cros_oem_widevine_rot = {
    .buffer = cros_oem_widevine_rot_buffer,
    .max_length = CROS_OEM_WIDEVINE_ROT_LEN,
    .length = 0,
    .is_set = 0,
};

/* Setup SMC Service */
static int32_t cros_oem_smc_setup(void) {
  /* Nothing to do for now. */
  return 0;
}

static uintptr_t cros_read_data(cros_oem_data *data, u_register_t offset,
                                void *handle) {
  uint32_t length;
  u_register_t ret = SMC_UNK;
  u_register_t results[7] = {};

  if (!data->is_set) {
    SMC_RET1(handle, SMC_UNK);
  }

  if (offset < 0 || offset > data->length) {
    SMC_RET1(handle, SMC_UNK);
  }

  length = data->length - offset;
  if (length > 7 * 8) {
    length = 7 * 8;
    ret = length;
  } else {
    ret = length | CROS_OEM_END_OF_DATA;
  }

  memcpy(results, &data->buffer[offset], length);

  SMC_RET8(handle, ret, results[0], results[1], results[2], results[3],
           results[4], results[5], results[6]);
}

static uintptr_t cros_write_data(cros_oem_data *data, u_register_t length,
                                 u_register_t x2, u_register_t x3,
                                 u_register_t x4, u_register_t x5,
                                 u_register_t x6, u_register_t x7,
                                 void *handle) {
  u_register_t value[6] = {x2, x3, x4, x5, x6, x7};

  if (data->is_set) {
    SMC_RET1(handle, SMC_UNK);
  }

  if (length & CROS_OEM_END_OF_DATA) {
    data->is_set = 1;
  }

  length &= ~CROS_OEM_END_OF_DATA;

  if (length < 0 || length > 6 * 8 ||
      data->length + length > data->max_length) {
    SMC_RET1(handle, SMC_UNK);
  }

  memcpy(&data->buffer[data->length], value, length);

  data->length += length;

  SMC_RET1(handle, SMC_OK);
}

/* Handler for servicing specific SMC calls. */
uintptr_t cros_oem_smc_handler(uint32_t smc_fid, u_register_t x1,
                               u_register_t x2, u_register_t x3,
                               u_register_t x4, void *cookie, void *handle,
                               u_register_t flags) {
  u_register_t x5, x6, x7;

  x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
  x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
  x7 = SMC_GET_GP(handle, CTX_GPREG_X7);

  switch (smc_fid) {
  case CROS_OEM_SMC_GET_TPM_AUTH_PUB_FUNC_ID:
    return cros_read_data(&cros_oem_tpm_auth_pk, x1, handle);
  case CROS_OEM_SMC_SET_TPM_AUTH_PUB_FUNC_ID:
    return cros_write_data(&cros_oem_tpm_auth_pk, x1, x2, x3, x4, x5, x6, x7,
                           handle);
  case CROS_OEM_SMC_GET_DEVICE_ID_FUNC_ID:
    return cros_read_data(&cros_oem_huk, x1, handle);
  case CROS_OEM_SMC_SET_DEVICE_ID_FUNC_ID:
    return cros_write_data(&cros_oem_huk, x1, x2, x3, x4, x5, x6, x7, handle);
  case CROS_OEM_SMC_GET_WIDEVINE_ROT_FUNC_ID:
    return cros_read_data(&cros_oem_widevine_rot, x1, handle);
  case CROS_OEM_SMC_SET_WIDEVINE_ROT_FUNC_ID:
    return cros_write_data(&cros_oem_widevine_rot, x1, x2, x3, x4, x5, x6, x7,
                           handle);
  default:
    WARN("Unimplemented OEM Call: 0x%x\n", smc_fid);
    SMC_RET1(handle, SMC_UNK);
  }
}

/*
 * Top level handler for SMC calls, dispatched if they are OEM specific.
 */
uintptr_t cros_oem_svc_smc_handler(uint32_t smc_fid, u_register_t x1,
                                   u_register_t x2, u_register_t x3,
                                   u_register_t x4, void *cookie, void *handle,
                                   u_register_t flags) {
  /* We only accept calls from the secure world. */
  if (is_caller_non_secure(flags)) {
    WARN("CrOS SMC call invoked from non-secure world");
    SMC_RET1(handle, SMC_UNK);
  }
  /*
   * Dispatch OEM calls to OEM Common handler and return its return value
   */
  if (is_oem_fid(smc_fid)) {
    return cros_oem_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
  }

  switch (smc_fid) {
  case CROS_OEM_SVC_CALL_COUNT:
    /*
     * Return the number of OEM Service Calls.
     */
    SMC_RET1(handle, CROS_OEM_SVC_NUM_CALLS);

  case CROS_OEM_SVC_UID:
    /* Return UID to the caller */
    SMC_UUID_RET(handle, cros_oem_smc_svc_uid);

  case CROS_OEM_SVC_VERSION:
    /* Return the version of current implementation */
    SMC_RET2(handle, CROS_OEM_VERSION_MAJOR, CROS_OEM_VERSION_MINOR);

  default:
    WARN("Unimplemented OEM Service Call: 0x%x\n", smc_fid);
    SMC_RET1(handle, SMC_UNK);
  }
}

/* Register OEM Service Calls as runtime service */
DECLARE_RT_SVC(cros_oem_smc_svc, OEN_OEM_START, OEN_OEM_END, SMC_TYPE_FAST,
               cros_oem_smc_setup, cros_oem_smc_handler);
