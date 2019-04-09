/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>
#include <stdbool.h>

#include <drivers/arm/smmu_v3.h>
#include <lib/mmio.h>

static inline uint32_t __init smmuv3_read_s_idr1(uintptr_t base)
{
	return mmio_read_32(base + SMMU_S_IDR1);
}

static inline uint32_t __init smmuv3_read_s_init(uintptr_t base)
{
	return mmio_read_32(base + SMMU_S_INIT);
}

static inline void __init smmuv3_write_s_init(uintptr_t base, uint32_t value)
{
	mmio_write_32(base + SMMU_S_INIT, value);
}

/* Test for pending invalidate */
static inline bool smmuv3_inval_pending(uintptr_t base)
{
	return (smmuv3_read_s_init(base) & SMMU_S_INIT_INV_ALL_MASK) != 0U;
}

/*
 * Initialize the SMMU by invalidating all secure caches and TLBs.
 *
 * Returns 0 on success, and -1 on failure.
 */
int __init smmuv3_init(uintptr_t smmu_base)
{
	uint32_t idr1_reg;

	/*
	 * SMMU_CR0 resets to zero with all non-secure streams bypassing
	 * the SMMU, so just abort all incoming transactions in order
	 * to implement a default deny policy on reset.
	 */
	mmio_setbits_32(smmu_base + SMMU_GBPA, 1U << SMMU_GBPA_ABORT);

	/*
	 * Invalidation of secure caches and TLBs is required only if the SMMU
	 * supports secure state. If not, it's implementation defined as to how
	 * SMMU_S_INIT register is accessed.
	 */
	idr1_reg = smmuv3_read_s_idr1(smmu_base);
	if (((idr1_reg >> SMMU_S_IDR1_SECURE_IMPL_SHIFT) &
			SMMU_S_IDR1_SECURE_IMPL_MASK) == 0U) {
		return -1;
	}

	/* Initiate invalidation, and wait for it to finish */
	smmuv3_write_s_init(smmu_base, SMMU_S_INIT_INV_ALL_MASK);
	while (smmuv3_inval_pending(smmu_base))
		;

	/*
	 * SMMU_S_CR0 resets to zero with all secure streams bypassing
	 * the SMMU, so just abort all incoming transactions in order
	 * to implement a default deny policy on reset.
	 */
	mmio_setbits_32(smmu_base + SMMU_S_GBPA, 1U << SMMU_S_GBPA_ABORT);

	return 0;
}
