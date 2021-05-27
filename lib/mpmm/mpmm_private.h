/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPMM_PRIVATE_H
#define MPMM_PRIVATE_H

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <platform_def.h>

/*
 * Note that the following system register may be alternatively named and
 * documented by individual core technical reference manuals. Cores implementing
 * MPMM will, however, share this implementation.
 */

/*
 * Global PPM Configuration Register definitions.
 */

#define CPUPPMCR_EL3	S3_6_C15_C2_0

#define CPUPPMCR_EL3_MPMMPINCTL_SHIFT	UINT64_C(0)
#define CPUPPMCR_EL3_MPMMPINCTL_MASK	(UINT64_C(0x1) << CPUPPMCR_EL3_MPMMPINCTL_SHIFT)

DEFINE_RENAME_SYSREG_RW_FUNCS(cpuppmcr_el3, CPUPPMCR_EL3)

/*
 * MPMM Control Register definitions.
 */

#define CPUMPMMCR_EL3	S3_1_C15_C2_1

#define CPUMPMMCR_EL3_MPMM_EN_SHIFT	UINT64_C(0)
#define CPUMPMMCR_EL3_MPMM_EN_MASK	(UINT64_C(0x1) << CPUMPMMCR_EL3_MPMM_EN_SHIFT)

DEFINE_RENAME_SYSREG_RW_FUNCS(cpumpmmcr_el3, CPUMPMMCR_EL3)

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
