/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPMM_PRIVATE_H
#define MPMM_PRIVATE_H

#include <assert.h>
#include <stdint.h>

#include <platform_def.h>

/*
 * MPMM FCONF topology.
 *
 * This topology structure describes the system-wide representation of the
 * information retrieved from the hardware configuration device tree.
 */
struct mpmm_fconf_topology {
	/*
	 * Bit-mask of cores supporting MPMM.
	 *
	 * The support for core N is given by the following formula:
	 *
	 *     (supported[N / 8] >> (N % 8)) & 0b1
	 */
	uint8_t supported[((PLATFORM_CORE_COUNT - 1U) / 8U) + 1U];
};

/*
 * Retrieve the MPMM topology of the system.
 *
 * Returns `NULL` if the topology was not populated, otherwise a pointer with
 * static lifetime to the results of population.
 */
const struct mpmm_fconf_topology *mpmm_topology(void);

/*
 * Identify whether MPMM is supported for a given core position.
 */
bool mpmm_fconf_topology_supported(
	const struct mpmm_fconf_topology *topology, unsigned int core_pos);

/*
 * Set the MPMM support status for a given core position.
 */
void mpmm_fconf_topology_set_supported(
	struct mpmm_fconf_topology *topology, unsigned int core_pos,
	bool supported);

#endif /* MPMM_PRIVATE_H */
