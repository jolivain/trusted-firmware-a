/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <platform_def.h>

#include <lib/smccc.h>
#include <services/trng_svc.h>
#include <smccc_helpers.h>

#include <plat/common/platform.h>

#define NSAMPLE_CLOCKS	1U /* min 1 cycle, max 231 cycles */
#define NRETRIES	5U

static bool juno_trng_initialized = false;

static bool output_valid(void)
{
	unsigned int i;

	for (i = 0U; i < NRETRIES; i++) {
		uint32_t val;

		val = mmio_read_32(TRNG_BASE + TRNG_STATUS);
		if ((val & 1U) != 0U) {
			return true;
		}
	}
	return false; /* No output data available. */
}

DEFINE_SVC_UUID2(_plat_trng_uuid,
	0x23523c58, 0x7448, 0x4083, 0x9d, 0x16,
	0xe3, 0xfa, 0xb9, 0xf1, 0x73, 0xbc
);
uuid_t plat_trng_uuid = _plat_trng_uuid;

bool plat_get_entropy(uint64_t *out)
{
	uint64_t ret;

	assert(out);
	assert(!check_uptr_overflow((uintptr_t)out, sizeof(*out)));

	if (!juno_trng_initialized) {
		/* Disable interrupt mode. */
		mmio_write_32(TRNG_BASE + TRNG_INTMASK, 0U);
		/* Program TRNG to sample for `NSAMPLE_CLOCKS`. */
		mmio_write_32(TRNG_BASE + TRNG_CONFIG, NSAMPLE_CLOCKS);
		/* Abort any potentially pending sampling. */
		mmio_write_32(TRNG_BASE + TRNG_CONTROL, 2U);
		/* Reset TRNG outputs. */
		mmio_write_32(TRNG_BASE + TRNG_STATUS, 1U);

		juno_trng_initialized = true;
	}

	if (!output_valid()) {
		/* Start TRNG. */
		mmio_write_32(TRNG_BASE + TRNG_CONTROL, 1U);

		if (!output_valid()) {
			return false;
		}
	}

	/* XOR each two 32-bit registers together, combine the pairs */
	ret = mmio_read_32(TRNG_BASE + 0);
	ret ^= mmio_read_32(TRNG_BASE + 4);
	ret <<= 32;

	ret |= mmio_read_32(TRNG_BASE + 8);
	ret ^= mmio_read_32(TRNG_BASE + 12);
	*out = ret;

	/* Acknowledge current cycle, clear output registers. */
	mmio_write_32(TRNG_BASE + TRNG_STATUS, 1U);
	/* Trigger next TRNG cycle. */
	mmio_write_32(TRNG_BASE + TRNG_CONTROL, 1U);

	return true;
}

void plat_entropy_setup(void)
{
	uint64_t dummy;

	/* Initialise the entropy source and trigger RNG generation */
	plat_get_entropy(&dummy);
}
