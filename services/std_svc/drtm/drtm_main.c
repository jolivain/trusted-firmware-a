/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * DRTM service
 *
 * Authors:
 *	Lucian Paul-Trifu <lucian.paul-trifu@arm.com>
 * 	Brian Nezvadovitz <brinez@microsoft.com> 2021-02-01
 */

#include <stdint.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/auth/crypto_mod.h>
#include <services/drtm_svc.h>

#include "drtm_main.h"

/* This value is used by the SMC to adevertise the boot PE */
static uint64_t boot_pe_aff_value;

int drtm_setup(void)
{
	int rc;

	INFO("DRTM service setup\n");

	boot_pe_aff_value = read_mpidr_el1() & MPIDR_AFFINITY_MASK;

	rc = drtm_dma_prot_init();
	if (rc != 0) {
		return rc;
	}

	/*
	 * initialise the platform supported crypto module that will
	 * be used by the DRTM-service to calculate hash of DRTM-
	 * implementation specific components
	 */
	crypto_mod_init();

	return 0;
}

uint64_t drtm_smc_handler(uint32_t smc_fid,
		uint64_t x1,
		uint64_t x2,
		uint64_t x3,
		uint64_t x4,
		void *cookie,
		void *handle,
		uint64_t flags)
{
	/* Check that the SMC call is from the Normal World. */
	if (!is_caller_non_secure(flags)) {
		SMC_RET1(handle, NOT_SUPPORTED);
	}

	switch (smc_fid) {
	case ARM_DRTM_SVC_VERSION:
		INFO("DRTM service handler: version\n");
		/* Return the version of current implementation */
		SMC_RET1(handle, ARM_DRTM_VERSION);

	case ARM_DRTM_SVC_FEATURES:
		if ((x1 >> 63 & 0x1U) == 0) {
			uint32_t func_id = x1;

			/* Dispatch function-based queries. */
			switch (func_id) {
			case ARM_DRTM_SVC_VERSION:
				SMC_RET1(handle, SUCCESS);

			case ARM_DRTM_SVC_FEATURES:
				SMC_RET1(handle, SUCCESS);

			case ARM_DRTM_SVC_UNPROTECT_MEM:
				SMC_RET1(handle, SUCCESS);

			case ARM_DRTM_SVC_DYNAMIC_LAUNCH:
				SMC_RET1(handle, SUCCESS);

			case ARM_DRTM_SVC_CLOSE_LOCALITY:
				WARN("ARM_DRTM_SVC_CLOSE_LOCALITY feature"
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);

			case ARM_DRTM_SVC_GET_ERROR:
				WARN("ARM_DRTM_SVC_GET_ERROR feature"
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);

			case ARM_DRTM_SVC_SET_ERROR:
				WARN("ARM_DRTM_SVC_SET_ERROR feature"
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);

			case ARM_DRTM_SVC_TCB_HASH:
				WARN("ARM_DRTM_SVC_TCB_HASH feature"
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);

			case ARM_DRTM_SVC_LOCK_TCB_HASH:
				WARN("ARM_DRTM_SVC_LOCK_TCB_HASH feature"
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);

			default:
				ERROR("Unknown ARM DRTM service function feature\n");
				SMC_RET1(handle, NOT_SUPPORTED);
			}
		}

	case ARM_DRTM_SVC_UNPROTECT_MEM:
		INFO("DRTM service handler: unprotect mem\n");
		SMC_RET1(handle, SMC_OK);

	case ARM_DRTM_SVC_DYNAMIC_LAUNCH:
		INFO("DRTM service handler: dynamic launch\n");
		SMC_RET1(handle, SMC_OK);

	case ARM_DRTM_SVC_CLOSE_LOCALITY:
		INFO("DRTM service handler: close locality\n");
		SMC_RET1(handle, SMC_OK);

	case ARM_DRTM_SVC_GET_ERROR:
		INFO("DRTM service handler: get error\n");
		/* dummy error code */
		uint64_t err_code = 0;
		SMC_RET2(handle, SMC_OK, err_code);

	case ARM_DRTM_SVC_SET_ERROR:
		INFO("DRTM service handler: set error\n");
		SMC_RET1(handle, SMC_OK);

	default:
		ERROR("Unknown ARM DRTM service call: 0x%x \n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}
