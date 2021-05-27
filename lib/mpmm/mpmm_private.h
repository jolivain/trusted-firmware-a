/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPMM_PRIVATE_H
#define MPMM_PRIVATE_H

#include <stdbool.h>
#include <platform_def.h>

/*
 * MPMM FCONF core data.
 *
 * This structure represents per-core data retrieved from the hardware
 * configuration device tree.
 */
struct mpmm_fconf_core {
	/*
	 * Whether MPMM is supported.
	 *
	 * Cores with support for MPMM offer one or more auxiliary AMU counters
	 * representing MPMM gears.
	 */
	bool supported;
};

/*
 * MPMM FCONF topology.
 *
 * This topology structure describes the system-wide representation of the
 * information retrieved from the hardware configuration device tree.
 */
struct mpmm_fconf_topology {
	/*
	 * Per-core data.
	 */
	struct mpmm_fconf_core cores[PLATFORM_CORE_COUNT];
};

/*
 * Retrieve the MPMM topology of the system.
 *
 * Returns `NULL` if the topology was not populated, otherwise a pointer with
 * static lifetime to the results of population.
 */
const struct mpmm_fconf_topology *mpmm_topology(void);

#endif /* MPMM_PRIVATE_H */
