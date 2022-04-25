/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMM_CORE_MANIFEST_H
#define RMM_CORE_MANIFEST_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/cassert.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

#define RMMD_MANIFEST_VERSION_MAJOR		U(0)
#define RMMD_MANIFEST_VERSION_MINOR		U(1)

/*
 * Manifest version encoding:
 *	- Bit[31] RES0
 *	- Bits [30:16] Major version
 *	- Bits [15:0] Minor version
 */
#define _RMMD_MANIFEST_VERSION(_major, _minor)				\
	((((_major) & 0x7FFF) << 16) | ((_minor) & 0xFFFF))

#define RMMD_MANIFEST_VERSION _RMMD_MANIFEST_VERSION(			\
				RMMD_MANIFEST_VERSION_MAJOR,		\
				RMMD_MANIFEST_VERSION_MINOR)

#define RMMD_GET_MANIFEST_VERSION_MAJOR(_version)			\
	((_version >> 16) & 0x7FFF)

#define RMMD_GET_MANIFEST_VERSION_MINOR(_version)			\
	(_version & 0xFFFF)

/* Boot manifest core structure as per v0.1 */
typedef struct rmm_manifest {
	uint32_t version;	/* Manifest version */
	uintptr_t plat_data;	/* Manifest platform data */
} rmm_manifest_t;

CASSERT(offsetof(rmm_manifest_t, version) == 0,
				rmm_manifest_t_version_unaligned);
CASSERT(offsetof(rmm_manifest_t, plat_data) == 8,
				rmm_manifest_t_plat_data_unaligned);

/* Accessors for the manifest platform data */
static inline int rmmd_set_platform_manifest_data(uintptr_t core_manifest,
						  uintptr_t platform_manifest)
{
	rmm_manifest_t *manifest;

	/* core_manifest and platform manifest must be in the same page */
	assert((core_manifest & ~(PAGE_SIZE_MASK)) ==
	       (platform_manifest & ~(PAGE_SIZE_MASK)));

	/* core_manifest and platform manifest cannot overlap */
	assert(platform_manifest + sizeof(rmm_manifest_t) <=
	       core_manifest);

	manifest = (rmm_manifest_t *)core_manifest;
	manifest->plat_data = platform_manifest;

	return 0;
}

static inline uintptr_t rmmd_get_platform_manifest_data(
						uintptr_t core_manifest)
{
	rmm_manifest_t *manifest = (rmm_manifest_t *)core_manifest;

	assert(manifest != NULL);

	return manifest->plat_data;
}

/*
 * This function searches for the next 8-byte aligned address after
 * the core manifest that can be used to store the platform manifest
 * data. It assings that address to the plat_data field of the
 * core manifest passed on the argument.
 *
 * Return: The address used for the platform manifest.
 */
static inline uintptr_t rmmd_allocate_platform_manifest(
						uintptr_t core_manifest)
{
	rmm_manifest_t *manifest;
	uintptr_t retval;

	manifest = (rmm_manifest_t *)core_manifest;
	assert(manifest != NULL);

	retval = core_manifest + sizeof(rmm_manifest_t);

	/* Align the area for the platform manifest to 8 bytes */
	retval &= ~((unsigned long)
				(1UL << (sizeof(unsigned long) - 1UL)));
	retval += sizeof(unsigned long);

	manifest->plat_data = retval;

	return retval;
}

#endif /* RMM_CORE_MANIFEST_H */
