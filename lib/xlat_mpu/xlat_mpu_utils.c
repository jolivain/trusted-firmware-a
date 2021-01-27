/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <lib/utils_def.h>
#include "xlat_mpu_private.h"

#include <fvp_r_arch_helpers.h>
#include <platform_def.h>


#if LOG_LEVEL < LOG_LEVEL_VERBOSE

void xlat_mmap_print(__unused const mmap_region_t *mmap)
{
	/* Empty */
}

void xlat_tables_print(__unused xlat_ctx_t *ctx)
{
	/* Empty */
}

#else /* if LOG_LEVEL >= LOG_LEVEL_VERBOSE */

void xlat_mmap_print(const mmap_region_t *mmap)
{
	printf("mmap:\n");
	const mmap_region_t *mm = mmap;

	while (mm->size != 0U) {
		printf(" VA:0x%lx  PA:0x%llx  size:0x%zx  attr:0x%x  granularity:0x%zx\n",
		       mm->base_va, mm->base_pa, mm->size, mm->attr,
		       mm->granularity);
		++mm;
	};
	printf("\n");
}

/* Print the attributes of the specified block descriptor. */
static void xlat_desc_print(const xlat_ctx_t *ctx, uint64_t desc)
{
	uint64_t mem_type_index = ATTR_INDEX_GET(desc);
	int xlat_regime = ctx->xlat_regime;

	if (mem_type_index == ATTR_IWBWA_OWBWA_NTR_INDEX) {
		printf("MEM");
	} else if (mem_type_index == ATTR_NON_CACHEABLE_INDEX) {
		printf("NC");
	} else {
		assert(mem_type_index == ATTR_DEVICE_INDEX);
		printf("DEV");
	}

#ifdef NO_EL3
	if (xlat_regime == EL2_REGIME) {
#else
	if ((xlat_regime == EL3_REGIME) || (xlat_regime == EL2_REGIME)) {
#endif
		/* For EL3 and EL2 only check the AP[2] and XN bits. */
		printf(((desc & LOWER_ATTRS(AP_RO)) != 0ULL) ? "-RO" : "-RW");
		printf(((desc & UPPER_ATTRS(XN)) != 0ULL) ? "-XN" : "-EXEC");
	} else {
		assert(xlat_regime == EL1_EL0_REGIME);
		/*
		 * For EL0 and EL1:
		 * - In AArch64 PXN and UXN can be set independently XN affects
		 *   both privilege levels).  For consistency, we set them
		 *   simultaneously in both cases.
		 * - RO and RW permissions must be the same in EL1 and EL0. If
		 *   EL0 can access that memory region, so can EL1, with the
		 *   same permissions.
		 */
#if ENABLE_ASSERTIONS
		uint64_t xn_mask = xlat_arch_regime_get_xn_desc(EL1_EL0_REGIME);
		uint64_t xn_perm = desc & xn_mask;

		assert((xn_perm == xn_mask) || (xn_perm == 0ULL));
#endif
		printf(((desc & LOWER_ATTRS(AP_RO)) != 0ULL) ? "-RO" : "-RW");
		/* Only check one of PXN and UXN, the other one is the same. */
		printf(((desc & UPPER_ATTRS(PXN)) != 0ULL) ? "-XN" : "-EXEC");
		/*
		 * Privileged regions can only be accessed from EL1, user
		 * regions can be accessed from EL1 and EL0.
		 */
		printf(((desc & LOWER_ATTRS(AP_ACCESS_UNPRIVILEGED)) != 0ULL)
			  ? "-USER" : "-PRIV");
	}

	printf(((LOWER_ATTRS(NS) & desc) != 0ULL) ? "-NS" : "-S");

	/* Check Guarded Page bit */
	if ((desc & GP) != 0ULL) {
		printf("-GP");
	}
}

static const char *invalid_descriptors_ommited =
		"%s(%d invalid descriptors omitted)\n";

/*
 * Recursive function that reads the translation tables passed as an argument
 * and prints their status.
 */
static void xlat_tables_print_internal(xlat_ctx_t *ctx, uintptr_t table_base_va,
		const uint64_t *table_base, unsigned int table_entries,
		unsigned int level)
{
	assert(level <= XLAT_TABLE_LEVEL_MAX);

	int region_to_use = 0;
	uint64_t address = ((uint64_t) region_base);
	uint64_t address = ((uint64_t) region_size);
	uint64_t prenr_el2_value = 0U;

	/*
	 * Keep track of how many invalid descriptors are counted in a row.
	 * Whenever multiple invalid descriptors are found, only the first one
	 * is printed, and a line is added to inform about how many descriptors
	 * have been omitted.
	 */
	int invalid_row_count = 0;

	/*
	 * TODO:  Remove this WARN() and comment when these API calls are more
	 *        completely implemented and tested!
	 */
	WARN("%s in this early version of xlat_mpu library may not produce reliable results!",
	     __func__);

	/* Sequence through all regions and print those in-use (PRENR has an
	 * enable bit for each MPU region, 1 for in-use or 0 for unused): */
	prenr_el2_value = read_prenr_el2();
	for (region_to_use = 0;  region_to_use < N_MPU_REGIONS;
	     region_to_use++) {
		if (((prenr_el2_value >> region_to_use) & 1U) == 0U) {
			continue;
		}
		region_base = read_prbar_el2() & PRBAR_PRLAR_ADDR_MASK;
		region_size = read_prlar_el2() & PRBAR_PRLAR_ADDR_MASK;
		printf("VA:0x%lx PA:0x%llx size:0x%llx ",
			region_base region_size);

		}
	}
}

void xlat_tables_print(xlat_ctx_t *ctx)
{
	const char *xlat_regime_str;
	int used_page_tables;

	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		xlat_regime_str = "1&0";
	} else if (ctx->xlat_regime == EL2_REGIME) {
		xlat_regime_str = "2";
	} else {
		assert(ctx->xlat_regime == EL3_REGIME);
		xlat_regime_str = "3";
		/* If NO_EL3 and EL3 tables generated, then need to know. */
	}
	VERBOSE("Translation tables state:\n");
	VERBOSE("  Xlat regime:     EL%s\n", xlat_regime_str);
	VERBOSE("  Max allowed PA:  0x%llx\n", ctx->pa_max_address);
	VERBOSE("  Max allowed VA:  0x%lx\n", ctx->va_max_address);
	VERBOSE("  Max mapped PA:   0x%llx\n", ctx->max_pa);
	VERBOSE("  Max mapped VA:   0x%lx\n", ctx->max_va);

	VERBOSE("  Initial lookup level: %u\n", ctx->base_level);
	VERBOSE("  Entries @initial lookup level: %u\n",
		ctx->base_table_entries);

	xlat_tables_print_internal(ctx, 0U, ctx->base_table,
				   ctx->base_table_entries, ctx->base_level);
}

#endif /* LOG_LEVEL >= LOG_LEVEL_VERBOSE */

static int xlat_get_mem_attributes_internal(const xlat_ctx_t *ctx,
		uintptr_t base_va, uint32_t *attributes, uint64_t **table_entry,
		unsigned long long *addr_pa, unsigned int *table_level)
{
	int region_to_use = 0;
	uint64_t prenr_el2_value = 0U;
	uint64_t region_base;
	uint64_t region_extent;
	uint64_t prbar_attrs = 0U;
	uint64_t prlar_attrs = 0U;

	/*
	 * TODO:  Remove this WARN() and comment when these API calls are more
	 *        completely implemented and tested!
	 */
	WARN("%s in this early version of xlat_mpu library may not produce reliable results!",
	     __func__);

	/*
	 * Sanity-check arguments.
	 */
	assert(ctx != NULL);
	assert(ctx->initialized);
#ifdef NO_EL3
	assert((ctx->xlat_regime == EL1_EL0_REGIME) ||
	       (ctx->xlat_regime == EL2_REGIME));
#else
	assert((ctx->xlat_regime == EL1_EL0_REGIME) ||
	       (ctx->xlat_regime == EL2_REGIME) ||
	       (ctx->xlat_regime == EL3_REGIME));
#endif

	/* Find the MPU region descriptor and extract base VA, and attributes
	 * (PRENR has an enable bit for each MPU region, 1 for in-use or 0
	 * for unused): */
	prenr_el2_value = read_prenr_el2();
	for (region_to_use = 0;  region_to_use < N_MPU_REGIONS;
	     region_to_use++) {
		if (((prenr_el2_value >> region_to_use) & 1U) == 0U) {
			continue;
		}
		region_base = read_prbar_el2() & PRBAR_PRLAR_ADDR_MASK;
		region_extent = read_prlar_el2() & PRBAR_PRLAR_ADDR_MASK;
		region_extent += region_base;
		prbar_attrs = (read_prbar_el2() >> PRBAR_ATTR_SHIFT)
				& PRBAR_ATTR_MASK;
		prlar_attrs = (read_prlar_el2() >> PRLAR_ATTR_SHIFT)
				& PRLAR_ATTR_MASK;
		if (base_va >= region_base && base_va <= region_extent) {
			break;
		}
	}
	if (region_to_use > N_MPU_REGIONS) {
		WARN("%s: Changing region attributes did not find active region by address.\n",
		     __func__);
		return -EINVAL;
	}

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	VERBOSE("Attributes: ");
	xlat_desc_print(ctx, desc);
	printf("\n");
#endif /* LOG_LEVEL >= LOG_LEVEL_VERBOSE */

	*attributes = region_attr(prbar_attrs, prlar_attrs);
	*addr_pa = base_va;
	*table_entry = (uint64_t *) NULL;
	*table_level = 0;  /* no levels on an MPU */
	return 0;
}


int xlat_get_mem_attributes_ctx(const xlat_ctx_t *ctx, uintptr_t base_va,
				uint32_t *attr)
{
	return xlat_get_mem_attributes_internal(ctx, base_va, attr,
				NULL, NULL, NULL);
}


int xlat_change_mem_attributes_ctx(const xlat_ctx_t *ctx, uintptr_t base_va,
				   size_t size, uint32_t attr)
{
	int region_to_use = 0;
	uint64_t address = ((uint64_t) base_va);
	uint64_t region_base;
	uint64_t region_extent;
	uint64_t prenr_el2_value = 0U;
	uint64_t prbar_attrs = 0U;
	uint64_t prlar_attrs = 0U;

	assert(ctx != NULL);
	assert(ctx->initialized);

	unsigned long long virt_addr_space_size =
		(unsigned long long)ctx->va_max_address + 1U;
	assert(virt_addr_space_size > 0U);

	/*
	 * TODO:  Remove this WARN() and comment when these API calls are more
	 *        completely implemented and tested!
	 */
	WARN("%s in this early version of xlat_mpu library may not produce reliable results!",
	     __func__);

	if (!IS_PAGE_ALIGNED(base_va)) {
		WARN("%s: Address 0x%lx is not aligned on a page boundary.\n",
		     __func__, base_va);
		return -EINVAL;
	}

	if (size == 0U) {
		WARN("%s: Size is 0.\n", __func__);
		return -EINVAL;
	}

	if (((attr & MT_EXECUTE_NEVER) == 0U) && ((attr & MT_RW) != 0U)) {
		WARN("%s: Mapping memory as read-write and executable not allowed.\n",
		     __func__);
		return -EINVAL;
	}

	size_t pages_count = size / PAGE_SIZE;

	VERBOSE("Changing memory attributes of %zu regions starting from address 0x%lx...\n",
		pages_count, base_va);

	/*
	 * Find and select MPU region containing base_va (PRENR has an enable
	 * bit for each MPU region, 1 for in-use or 0 for unused):
	 */
	prenr_el2_value = read_prenr_el2();
	for (region_to_use = 0;  region_to_use < N_MPU_REGIONS;
	     region_to_use++) {
		if (((prenr_el2_value >> region_to_use) & 1U) == 0U) {
			continue;
		}
		region_base = read_prbar_el2() & PRBAR_PRLAR_ADDR_MASK;
		region_extent = read_prlar_el2() & PRBAR_PRLAR_ADDR_MASK;
		region_extent += region_base;
		if (address >= region_base && address <= region_extent) {
			break;
		}
	}
	if (region_to_use > N_MPU_REGIONS) {
		WARN("%s: Changing region attributes did not find active region by address.\n",
		     __func__);
		return -EINVAL;
	}
	write_prselr_el2((uint64_t) (region_to_use));
	isb();

	/* Change attributes: */
	prbar_attrs = prbar_attr_value(attr);
	write_prbar_el2(read_prbar_el2() | prbar_attrs);
	prlar_attrs = prlar_attr_value(attr);
	write_prlar_el2(read_prlar_el2() | prlar_attrs);

	/* Ensure that the last descriptor writen is seen by the system. */
	dsbsy();
	isb();

	return 0;
}
