/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_features.h>
#include <arch_helpers.h>
#include <plat/common/platform.h>
#include <services/trng_svc.h>

bool plat_get_entropy(uint64_t *out)
{
	*out = read_rndr();
	return true;
}

DEFINE_SVC_UUID2(qemu_trng_uuid,
		 0xaa767875, 0xe700, 0x4d66, 0xbf, 0xb0,
		 0xdb, 0x35, 0xb4, 0xdf, 0x73, 0x25
);
uuid_t plat_trng_uuid;

void plat_entropy_setup(void)
{
	/* Use the RNDR instruction if the CPU supports it */
	if (is_feat_rng_supported()) {
		plat_trng_uuid = qemu_trng_uuid;
	}
}
