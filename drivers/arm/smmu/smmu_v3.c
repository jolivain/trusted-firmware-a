/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cdefs.h>
#include <stdbool.h>
#include <drivers/arm/smmu_v3.h>
#include <lib/mmio.h>

/* SMMU poll number of retries */
#define SMMU_POLL_RETRY		1000000

static int __init smmuv3_poll(uintptr_t smmu_reg, uint32_t mask,
				uint32_t value)
{
	uint32_t retries = SMMU_POLL_RETRY;

	while (((mmio_read_32(smmu_reg) & mask) != value) && (--retries != 0))
		;

	if (retries != 0)
		return 0;
		
	ERROR("Failed to poll SMMUv3 register @%p\n", (void *)smmu_reg);
	ERROR("Read value 0x%x, expected 0x%x\n", mmio_read_32(smmu_reg),
		value == 0 ? mmio_read_32(smmu_reg) & ~mask :
		mmio_read_32(smmu_reg) | mask);
	return -1;
}

/*
 * Initialize the SMMU by invalidating all secure caches and TLBs.
 * Abort all incoming transactions in order to implement a default
 * deny policy on reset
 */
int __init smmuv3_init(uintptr_t smmu_base)
{
	/*
	 * Invalidation of secure caches and TLBs is required only if the SMMU
	 * supports secure state. If not, it's implementation defined as to how
	 * SMMU_S_INIT register is accessed.
	 */
	if ((mmio_read_32(smmu_base + SMMU_S_IDR1) &
			SMMU_S_IDR1_SECURE_IMPL) != 0) {

		/* Initiate invalidation */
		mmio_write_32(smmu_base + SMMU_S_INIT, SMMU_S_INIT_INV_ALL);

		/* Wait for global invalidation operation to finish */
		return smmuv3_poll(smmu_base + SMMU_S_INIT, SMMU_S_INIT_INV_ALL, 0);
	}
	return 0;
}
