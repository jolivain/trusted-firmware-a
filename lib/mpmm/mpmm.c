/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include "mpmm_private.h"

#include <plat/common/platform.h>

static uint64_t read_cpuppmcr_el3_mpmmpinctl(void)
{
	return (read_cpuppmcr_el3() >> CPUPPMCR_EL3_MPMMPINCTL_SHIFT) &
		CPUPPMCR_EL3_MPMMPINCTL_MASK;
}

static void write_cpumpmmcr_el3_mpmm_en(uint64_t mpmm_en)
{
	uint64_t value = read_cpumpmmcr_el3();

	value &= ~(CPUMPMMCR_EL3_MPMM_EN_MASK << CPUMPMMCR_EL3_MPMM_EN_SHIFT);
	value |= (mpmm_en & CPUMPMMCR_EL3_MPMM_EN_MASK) <<
		CPUMPMMCR_EL3_MPMM_EN_SHIFT;

	write_cpumpmmcr_el3(value);
}

static bool mpmm_supported(void)
{
	bool supported = false;
	const struct mpmm_fconf_topology *topology = mpmm_topology();

	/*
	 * For the current core firstly try to find out if the hardware
	 * configuration has claimed support for MPMM, then make sure that MPMM
	 * is controllable through the system registers.
	 */

	if (topology != NULL) {
		unsigned int core_pos = plat_my_core_pos();

		supported = topology->cores[core_pos].supported &&
			(read_cpuppmcr_el3_mpmmpinctl() == 0U);
	}

	return supported;
}

void mpmm_enable(void)
{
	bool supported = mpmm_supported();

	if (supported) {
		write_cpumpmmcr_el3_mpmm_en(1U);
	}
}
