/*
 * Copyright (c) 2022-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <juno_tbb_key.h>
#include <ethosn_cert.h>

static key_t juno_plat_tbb_keys[] = {
	ETHOSN_NPU_FW_CONTENT_CERT_KEY_DEF
};

PLAT_REGISTER_KEYS(juno_plat_tbb_keys);
