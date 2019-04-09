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

void __init smmuv3_poll(uintptr_t smmu_reg, uint32_t mask, uint32_t value)
{
	uint32_t retries = SMMU_POLL_RETRY;

	while (((mmio_read_32(smmu_reg) & mask) != value) && (--retries != 0))
		;

	assert(retries != 0);
}

/*
 * Initialize the SMMU by invalidating all secure caches and TLBs.
 * Abort all incoming transactions in order to implement a default
 * deny policy on reset
 */
void __init smmuv3_init(uintptr_t smmu_base)
{
	/* Attribute update has completed when SMMU_(S)_GBPA.Update bit is 0 */
	smmuv3_poll(smmu_base + SMMU_GBPA, SMMU_GBPA_UPDATE, 0);

	/*
	 * SMMU_(S)_CR0 resets to zero with all streams bypassing the SMMU,
	 * so just abort all incoming transactions.
	 */
	mmio_setbits_32(smmu_base + SMMU_GBPA,
			SMMU_GBPA_UPDATE | SMMU_GBPA_ABORT);

	smmuv3_poll(smmu_base + SMMU_GBPA, SMMU_GBPA_UPDATE, 0);

	/*
	 * Invalidation of secure caches and TLBs is required only if the SMMU
	 * supports secure state. If not, it's implementation defined as to how
	 * SMMU_S_INIT register is accessed.
	 */
	if ((mmio_read_32(smmu_base + SMMU_S_IDR1) &
			SMMU_S_IDR1_SECURE_IMPL) == 0) {
		return;
	}

	/* Initiate invalidation */
	mmio_write_32(smmu_base + SMMU_S_INIT, SMMU_S_INIT_INV_ALL);

	/* Wait for global invalidation operation to finish */
	smmuv3_poll(smmu_base + SMMU_S_INIT, SMMU_S_INIT_INV_ALL, 0);

	/* Abort all incoming secure transactions */
	smmuv3_poll(smmu_base + SMMU_S_GBPA, SMMU_S_GBPA_UPDATE, 0);

	mmio_setbits_32(smmu_base + SMMU_S_GBPA,
			SMMU_S_GBPA_UPDATE | SMMU_S_GBPA_ABORT);

	smmuv3_poll(smmu_base + SMMU_S_GBPA, SMMU_S_GBPA_UPDATE, 0);
}
