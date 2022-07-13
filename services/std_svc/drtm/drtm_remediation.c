/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * DRTM support for DRTM error remediation.
 *
 */
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include "drtm_main.h"
#include <plat/common/platform.h>

static enum drtm_retc drtm_error_set(long long error_code)
{
	int ret = plat_set_drtm_error(error_code);

	if (ret != 0) {
		return INTERNAL_ERROR;
	}

	return SUCCESS;
}

static enum drtm_retc drtm_error_get(long long *error_code)
{
	int ret = plat_get_drtm_error(error_code);

	if (ret != 0) {
		return INTERNAL_ERROR;
	}

	return SUCCESS;
}

void drtm_enter_remediation(long long err_code, const char *err_str)
{
	enum drtm_retc rc = drtm_error_set(err_code);

	if (rc != SUCCESS) {
		ERROR("%s(): drtm_error_set() failed unexpectedly rc=%d\n",
		      __func__, rc);
		panic();
	}

	NOTICE("DRTM: entering remediation of error:\n%lld\t\'%s\'\n",
	       err_code, err_str);

	ERROR("%s(): system reset is not yet supported\n", __func__);
	plat_system_reset();
}

uintptr_t drtm_set_error(uint64_t x1, void *ctx)
{
	enum drtm_retc rc = drtm_error_set(x1);

	if (rc != SUCCESS) {
		SMC_RET1(ctx, rc);
	}

	SMC_RET1(ctx, SUCCESS);
}

uintptr_t drtm_get_error(void *ctx)
{
	long long error_code;
	enum drtm_retc rc = drtm_error_get(&error_code);

	if (rc != SUCCESS) {
		SMC_RET1(ctx, rc);
	}

	SMC_RET2(ctx, SUCCESS, error_code);
}
