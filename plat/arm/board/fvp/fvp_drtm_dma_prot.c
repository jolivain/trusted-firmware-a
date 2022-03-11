/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>

#include <drivers/arm/smmu_v3.h>
#include <plat/common/platform.h>

#include <plat/arm/common/arm_config.h>
#include <platform_def.h>

/**
 * array mentioning number of SMMUs supported by FVP
 */
static const uintptr_t fvp_smmus[] = {
	PLAT_FVP_SMMUV3_BASE,
};

bool plat_has_non_host_platforms(void)
{
	/* Note: FVP base platforms typically have GPU, as per --list-instances. */
	return true;
}

bool plat_has_unmanaged_dma_peripherals(void)
{
	/*
	 * RevC's --list-instances does not show devices that are described as
	 * DMA-capable but not managed by an SMMU in the FVP documentation.
	 * However, the SMMU seems to have only been introduced in the RevC
	 * revision.
	 */
	return !(arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3);
}

unsigned int plat_get_total_smmus(void)
{
	if ((arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3) != 0U) {
		return 1;
	} else {
		return 0;
	}
}

void plat_enumerate_smmus(const uintptr_t **smmus_out,
			  size_t *smmu_count_out)
{
	if ((arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3)) {
		*smmus_out = fvp_smmus;
		*smmu_count_out = sizeof(fvp_smmus) / sizeof(uintptr_t);
	} else {
		*smmus_out = NULL;
		*smmu_count_out = 0;
	}
}

/* DRTM DMA Protection Features */
static plat_drtm_dma_prot_features_t dma_prot_features;

plat_drtm_dma_prot_features_t *plat_drtm_get_dma_prot_features(void)
{
	/* Unused */
	dma_prot_features.max_num_mem_prot_regions = 0;

	/* Complete DMA protection only */
	dma_prot_features.dma_protection_support = 0x1;

	return &dma_prot_features;
}

uint64_t plat_drtm_get_dma_protected_region_size(void)
{
	return 0U;
}
