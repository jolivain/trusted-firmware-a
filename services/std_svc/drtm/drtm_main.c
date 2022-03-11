/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * DRTM service
 *
 * Authors:
 *	Lucian Paul-Trifu <lucian.paultrifu@gmail.com>
 *	Brian Nezvadovitz <brinez@microsoft.com> 2021-02-01
 */

#include <stdint.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/auth/crypto_mod.h>
#include "drtm_main.h"
#include <plat/common/platform.h>
#include <services/drtm_svc.h>
#include <platform_def.h>

/* Structure to store DRTM features specific to the platform. */
static drtm_features_t plat_drtm_features;

/* DRTM-formatted memory map. */
static drtm_memory_region_descriptor_table_t *plat_drtm_mem_map;

int drtm_setup(void)
{
	int rc;
	plat_drtm_tpm_features_t *plat_tpm_feat;
	plat_drtm_dma_prot_features_t *plat_dma_prot_feat;
	uint64_t dlme_data_min_size;

	INFO("DRTM service setup\n");

	/* Read boot PE ID from MPIDR */
	plat_drtm_features.boot_pe_id = read_mpidr_el1() & MPIDR_AFFINITY_MASK;

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

	/* Build DRTM-compatible address map. */
	plat_drtm_mem_map = drtm_build_address_map();
	if (plat_drtm_mem_map == NULL) {
		return INTERNAL_ERROR;
	}

	/* Get DRTM features from platform hooks. */
	plat_tpm_feat = plat_drtm_get_tpm_features();
	if (plat_tpm_feat == NULL) {
		return INTERNAL_ERROR;
	}
	plat_dma_prot_feat = plat_drtm_get_dma_prot_features();
	if (plat_dma_prot_feat == NULL) {
		return INTERNAL_ERROR;
	}

	/*
	 * Add up minimum DLME data memory.
	 *
	 * For systems with complete DMA protection there is only one entry in
	 * the protected regions table.
	 * calculate.
	 */
	if (plat_dma_prot_feat->dma_protection_support ==
			ARM_DRTM_DMA_PROT_FEATURES_DMA_SUPPORT_COMPLETE) {
		dlme_data_min_size =
			sizeof(drtm_memory_region_descriptor_table_t) +
			sizeof(drtm_mem_region_t);
	} else {
		/*
		 * TODO set protected regions table size based on platform DMA
		 * protection configuration
		 */
		panic();
	}

	dlme_data_min_size += (drtm_get_address_map_size() +
			       PLAT_DRTM_EVENT_LOG_MAX_SIZE +
			       plat_drtm_get_tcb_hash_table_size() +
			       plat_drtm_get_imp_def_dlme_region_size());

	/*
	 * The value dlme_data_min_size is in number of 4k pages so we need to
	 * convert it from just bytes.
	 */
	if ((dlme_data_min_size % 4096) == 0) {
		dlme_data_min_size = dlme_data_min_size / 4096;
	} else {
		dlme_data_min_size = (dlme_data_min_size / 4096) + 1;
	}

	/* Fill out platform DRTM features structure */
	/* Only support default PCR schema (0x1) in this implementation. */
	ARM_DRTM_TPM_FEATURES_SET_PCR_SCHEMA(plat_drtm_features.tpm_features,
		ARM_DRTM_TPM_FEATURES_PCR_SCHEMA_DEFAULT);
	ARM_DRTM_TPM_FEATURES_SET_TPM_HASH(plat_drtm_features.tpm_features,
		plat_tpm_feat->tpm_based_hash_support);
	ARM_DRTM_TPM_FEATURES_SET_FW_HASH(plat_drtm_features.tpm_features,
		plat_tpm_feat->firmware_hash_algorithm);
	ARM_DRTM_MIN_MEM_REQ_SET_MIN_DLME_DATA_SIZE(plat_drtm_features.minimum_memory_requirement,
		dlme_data_min_size);
	ARM_DRTM_MIN_MEM_REQ_SET_DCE_SIZE(plat_drtm_features.minimum_memory_requirement,
		plat_drtm_get_min_size_normal_world_dce());
	ARM_DRTM_DMA_PROT_FEATURES_SET_MAX_REGIONS(plat_drtm_features.dma_prot_features,
		plat_dma_prot_feat->max_num_mem_prot_regions);
	ARM_DRTM_DMA_PROT_FEATURES_SET_DMA_SUPPORT(plat_drtm_features.dma_prot_features,
		plat_dma_prot_feat->dma_protection_support);
	ARM_DRTM_TCB_HASH_FEATURES_SET_MAX_NUM_HASHES(plat_drtm_features.tcb_hash_features,
		plat_drtm_get_tcb_hash_features());

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
		ERROR("Unknown ARM DRTM service call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}
