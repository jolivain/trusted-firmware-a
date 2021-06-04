/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <plat/common/platform.h>

#include "mpmm.h"

/*
 * Per-core context information.
 */
struct mpmm_core_ctx {
	/*
	 * Whether MPMM is supported on this core.
	 */
	bool supported;
};

/*
 * System context information.
 */
struct mpmm_ctx {
	/*
	 * Per-core contexts.
	 */
	struct mpmm_core_ctx cores[PLATFORM_CORE_COUNT];
};

/*
 * System context instance.
 */
struct mpmm_ctx mpmm_ctx_;

static uint64_t read_cpuppmcr_el3_mpmmpinctl(void)
{
	return (read_cpuppmcr_el3() & CPUPPMCR_EL3_MPMMPINCTL_MASK) >>
		CPUPPMCR_EL3_MPMMPINCTL_SHIFT;
}

static void write_cpumpmmcr_el3_mpmm_en(uint64_t mpmm_en)
{
	uint64_t value = read_cpumpmmcr_el3();

	value &= ~CPUMPMMCR_EL3_MPMM_EN_MASK;
	value |= (mpmm_en << CPUMPMMCR_EL3_MPMM_EN_SHIFT) &
		CPUMPMMCR_EL3_MPMM_EN_MASK;

	write_cpumpmmcr_el3(value);
}

void mpmm_enable(void)
{
	const struct mpmm_fconf_topology *topology = NULL;
	unsigned int core_pos = plat_my_core_pos();

	struct mpmm_core_ctx *ctx = &mpmm_ctx_.cores[core_pos];

	/*
	 * For the current core firstly try to find out if the hardware
	 * configuration has claimed support for MPMM, then make sure that MPMM
	 * is controllable through the system registers.
	 */

	topology = mpmm_topology();
	if (topology != NULL) {
		ctx->supported = topology->cores[core_pos].supported &&
			!read_cpuppmcr_el3_mpmmpinctl();
	}

	/*
	 * Enable MPMM.
	 */

	if (ctx->supported) {
		write_cpumpmmcr_el3_mpmm_en(1);
	}
}

void mpmm_context_restore(void)
{
	unsigned int core_pos = plat_my_core_pos();
	const struct mpmm_core_ctx *ctx = &mpmm_ctx_.cores[core_pos];

	if (ctx->supported) {
		write_cpumpmmcr_el3_mpmm_en(1);
	}
}
