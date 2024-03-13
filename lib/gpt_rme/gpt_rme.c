/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include "gpt_rme_private.h"
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#if !ENABLE_RME
#error "ENABLE_RME must be enabled to use the GPT library"
#endif

/*
 * Lookup T from PPS
 *
 *   PPS    Size    T
 *   0b000  4GB     32
 *   0b001  64GB    36
 *   0b010  1TB     40
 *   0b011  4TB     42
 *   0b100  16TB    44
 *   0b101  256TB   48
 *   0b110  4PB     52
 *
 * See section 15.1.27 of the RME specification.
 */
static const gpt_t_val_e gpt_t_lookup[] = {PPS_4GB_T, PPS_64GB_T,
					   PPS_1TB_T, PPS_4TB_T,
					   PPS_16TB_T, PPS_256TB_T,
					   PPS_4PB_T};

/*
 * Lookup P from PGS
 *
 *   PGS    Size    P
 *   0b00   4KB     12
 *   0b10   16KB    14
 *   0b01   64KB    16
 *
 * Note that pgs=0b10 is 16KB and pgs=0b01 is 64KB, this is not a typo.
 *
 * See section 15.1.27 of the RME specification.
 */
static const gpt_p_val_e gpt_p_lookup[] = {PGS_4KB_P, PGS_64KB_P, PGS_16KB_P};

typedef struct {
	size_t size;
	unsigned int contig;
} gpt_fill_lookup_t;

static const gpt_fill_lookup_t gpt_fill_lookup[] = {
#if (GPT_MAX_BLOCK == 512)
	{ SZ_512M, GPT_L1_CONT_DESC_512MB },
#endif
#if (GPT_MAX_BLOCK >= 32)
	{ SZ_32M, GPT_L1_CONT_DESC_32MB },
#endif
#if (GPT_MAX_BLOCK != 0)
	{ SZ_2M, GPT_L1_CONT_DESC_2MB }
#endif
};

typedef uint64_t *(*gpt_shatter_func)(uintptr_t base, gpi_info_t *gpi_info, uint64_t l1_desc);
typedef void (*gpt_tlbi_func)(uintptr_t base);

static uint64_t *shatter_2mb(uintptr_t base, gpi_info_t *gpi_info, uint64_t l1_desc);
static uint64_t *shatter_32mb(uintptr_t base, gpi_info_t *gpi_info, uint64_t l1_desc);
static uint64_t *shatter_512mb(uintptr_t base, gpi_info_t *gpi_info, uint64_t l1_desc);

static void tlbi_4kb(uintptr_t base);
static void tlbi_16kb(uintptr_t base);
static void tlbi_64kb(uintptr_t base);
static void tlbi_2mb(uintptr_t base);
static void tlbi_32mb(uintptr_t base);
static void tlbi_512mb(uintptr_t base);

static const gpt_shatter_func gpt_shatter_lookup[] = {
	shatter_2mb,
	shatter_32mb,
	shatter_512mb
};

typedef struct {
	gpt_tlbi_func function;
	size_t mask;
} gpt_tlbi_lookup_t;

static const gpt_tlbi_lookup_t gpt_tlbi_lookup[] = {
	{ tlbi_2mb, ~(SZ_2M - 1UL) },
	{ tlbi_32mb, ~(SZ_32M - 1UL) },
	{ tlbi_512mb, ~(SZ_512M - 1UL) }
};

static const gpt_tlbi_lookup_t gpt_tlbi_pgs_lookup[] = {
	{ tlbi_4kb, ~(SZ_4K - 1UL) },
	{ tlbi_16kb, ~(SZ_16K - 1UL) },
	{ tlbi_64kb, ~(SZ_64K - 1UL) }
};

/*
 * Lookup S from PGS
 *
 *   Num = number of L1 entries in 512MB:
 *   512MB / (16 * 4KB) = 8192
 *   512MB / (16 * 16KB) = 2048
 *   512MB / (16 * 64KB) = 512
 *
 *   S = log2(Num)
 *
 *   PGS    Size    Num     S
 *   0b00   4KB     8192    13
 *   0b10   16KB    2048    11
 *   0b01   64KB    512     9
 *
 */
static const unsigned int gpt_s_lookup[] = {13U, 9U, 11U};

/*
 * This structure contains GPT configuration data
 */
typedef struct {
	uintptr_t plat_gpt_l0_base;
	gpccr_pps_e pps;
	gpt_t_val_e t;
	gpccr_pgs_e pgs;
	gpt_p_val_e p;
} gpt_config_t;

static gpt_config_t gpt_config;

/*
 * Number of L1 entries in 2MB, depending on
 * GPCCR_EL3.L0GPTSZ and GPCCR_EL3.PGS:
 * +---------+---------------------+
 * |         |       PGS           |
 * +---------+-------+------+------+
 * | L0GPTSZ |  4KB  | 16KB | 64KB |
 * +---------+-------+------+------+
 * |  1GB    |  32   |  8   |  2   |
 * +---------+-------+------+------+
 * |  16GB   |  512  | 128  |  32  |
 * +---------+-------+------+------+
 * |  64GB   | 2048  | 512  | 128  |
 * +---------+-------+------+------+
 * |  512GB  | 16384 | 4096 | 1024 |
 * +---------+-------+------+------+
 */
static unsigned int gpt_l1_cnt_2mb;

/*
 * Mask for the L1 index field, depending on
 * GPCCR_EL3.L0GPTSZ and GPCCR_EL3.PGS:
 * +---------+-------------------------------+
 * |         |             PGS               |
 * +---------+----------+----------+---------+
 * | L0GPTSZ |   4KB    |   16KB   |   64KB  |
 * +---------+----------+----------+---------+
 * |  1GB    |  0x3FFF  |  0xFFF   |  0x3FF  |
 * +---------+----------+----------+---------+
 * |  16GB   | 0x3FFFF  |  0xFFFF  | 0x3FFF  |
 * +---------+----------+----------+---------+
 * |  64GB   | 0xFFFFF  | 0x3FFFF  | 0xFFFF  |
 * +---------+----------+----------+---------+
 * |  512GB  | 0x7FFFFF | 0x1FFFFF | 0x7FFFF |
 * +---------+----------+----------+---------+
 */
static uint64_t gpt_l1_index_mask;

/* Number of 128-bit L1 entries in 2MB, 32MB and 512MB */
#define QWORDS_2MB	(gpt_l1_cnt_2mb / 2U)
#define QWORDS_32MB	(QWORDS_2MB * 16U)
#define QWORDS_512MB	(QWORDS_32MB * 16U)

/* Get the index into the L1 table from a physical address */
#define GPT_L1_INDEX(_pa)		\
	(((_pa) >> GPT_L1_IDX_SHIFT(gpt_config.p)) & gpt_l1_index_mask)

/* These variables are used during initialization of the L1 tables */
static unsigned int gpt_next_l1_tbl_idx;
static uintptr_t gpt_l1_tbl;
static size_t gpt_l0_mem_size;

/* These variables are used during runtime */
static uint64_t *gpt_l1_first;		/* Address of the 1st L1 table */
static unsigned int gpt_l1_size_bit;	/* log2(GPT_L1_TABLE_SIZE) */
static unsigned int gpt_512mb_bit;	/* log2(Number of L1 entries in 512MB) */
static bitlock_t *gpt_locks;		/* L1 tables bit locks */

/*
 * With 1 bit per 512MB block:
 * GPCCR_EL3.L0GPTSZ = 0: 1GB: 2 bits: 1
 * GPCCR_EL3.L0GPTSZ = 4: 16GB: 32 bits: 5
 * GPCCR_EL3.L0GPTSZ = 6: 64GB: 128 bits: 7
 * GPCCR_EL3.L0GPTSZ = 9: 512GB: 1024 bits: 10
 */
static unsigned int gpt_lock_bits;	/* log2(Number of lock bits per L1 entry) */

/*
 * Invalidate TLBs of GPT entries by Physical address, last level.
 *
 * @base: the starting address for the range of invalidation.
 */
static void tlbi_4kb(uintptr_t base)
{
	tlbirpallos(TLBI_ARG(base, SZ_4K));
}

static void tlbi_16kb(uintptr_t base)
{
	tlbirpallos(TLBI_ARG(base, SZ_16K));
}

static void tlbi_64kb(uintptr_t base)
{
	tlbirpallos(TLBI_ARG(base, SZ_64K));
}

static void tlbi_2mb(uintptr_t base)
{
	tlbirpallos(TLBI_ARG(base, SZ_2M));
	dsbsy();
}

static void tlbi_32mb(uintptr_t base)
{
	tlbirpallos(TLBI_ARG(base, SZ_32M));
	dsbsy();
}

static void tlbi_512mb(uintptr_t base)
{
	tlbirpallos(TLBI_ARG(base, SZ_512M));
	dsbsy();
}

static void tlbi_page(uintptr_t base)
{
	gpt_tlbi_pgs_lookup[gpt_config.pgs].function(
			base & gpt_tlbi_pgs_lookup[gpt_config.pgs].mask);
	dsbsy();
}

/*
 * Helper function to fill out GPI entries in a single L1 table
 * with Granules or Contiguous descriptor.
 *
 * Parameters
 *   l1			Pointer to 2MB, 32MB or 512MB aligned L1 table entry to fill out
 *   l1_desc		GPT Granules or Contiguous descriptor set this range to
 *   cnt		Number of double 128-bit L1 entries to fill
 *
 * Return
 *   Address of the next L1 table entry.
 */
static uint64_t *fill_desc(uint64_t *l1, uint64_t l1_desc, unsigned int cnt)
{
	uint128_t *l2 = (uint128_t *)l1;

	VERBOSE("GPT: %s(%p 0x%"PRIx64" %u)\n", __func__, l1, l1_desc, cnt);

	for (unsigned int i = 0U; i < cnt; i++) {
		*l2++ = l1_desc | ((uint128_t)l1_desc << 64);
	}
	return (uint64_t *)l2;
}

static uint64_t *shatter_2mb(uintptr_t base, gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	unsigned int idx = GPT_L1_INDEX(ALIGN_2MB(base));

	VERBOSE("GPT: %s(0x%"PRIxPTR" 0x%"PRIx64")\n", __func__, base, l1_desc);

	/* Convert 2MB Contiguous block to Granules */
	return fill_desc(&gpi_info->gpt_l1_addr[idx], l1_desc, QWORDS_2MB);
}

static uint64_t *shatter_32mb(uintptr_t base, gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	unsigned int idx = GPT_L1_INDEX(ALIGN_2MB(base));
	uint64_t *l1_gran = &gpi_info->gpt_l1_addr[idx];
	uint64_t l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, GPT_L1_CONT_DESC_2MB);
	uint64_t *l1;

	VERBOSE("GPT: %s(0x%"PRIxPTR" 0x%"PRIx64")\n", __func__, base, l1_desc);

	/* Get index corresponding to 32MB aligned address */
	idx = GPT_L1_INDEX(ALIGN_32MB(base));
	l1 = &gpi_info->gpt_l1_addr[idx];

	/* 16 x 2MB blocks in 32MB */
	for (unsigned int i = 0U; i < 16U; i++) {
		/* Fill with Granules or Contiguous descriptors */
		l1 = fill_desc(l1, (l1 == l1_gran) ? l1_desc : l1_cont_desc,
								QWORDS_2MB);
	}
	return l1;
}

static uint64_t *shatter_512mb(uintptr_t base, gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	unsigned int idx = GPT_L1_INDEX(ALIGN_32MB(base));
	uint64_t *l1_32mb = &gpi_info->gpt_l1_addr[idx];
	uint64_t l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, GPT_L1_CONT_DESC_32MB);
	uint64_t *l1;

	VERBOSE("GPT: %s(0x%"PRIxPTR" 0x%"PRIx64")\n", __func__, base, l1_desc);

	/* Get index corresponding to 512MB aligned address */
	idx = GPT_L1_INDEX(ALIGN_512MB(base));
	l1 = &gpi_info->gpt_l1_addr[idx];

	/* 16 x 32MB blocks in 512MB */
	for (unsigned int i = 0U; i < 16U; i++) {
		if (l1 == l1_32mb) {
			/* Shatter this 32MB block */
			l1 = shatter_32mb(base, gpi_info, l1_desc);
		} else {
			/* Fill 32MB with Contiguous descriptors */
			l1 = fill_desc(l1, l1_cont_desc, QWORDS_32MB);
		}
	}
	return l1;
}

/*
 * This function checks to see if a GPI value is valid.
 *
 * These are valid GPI values.
 *   GPT_GPI_NO_ACCESS   U(0x0)
 *   GPT_GPI_SECURE      U(0x8)
 *   GPT_GPI_NS          U(0x9)
 *   GPT_GPI_ROOT        U(0xA)
 *   GPT_GPI_REALM       U(0xB)
 *   GPT_GPI_ANY         U(0xF)
 *
 * Parameters
 *   gpi		GPI to check for validity.
 *
 * Return
 *   true for a valid GPI, false for an invalid one.
 */
static bool is_gpi_valid(unsigned int gpi)
{
	if ((gpi == GPT_GPI_NO_ACCESS) || (gpi == GPT_GPI_ANY) ||
	    ((gpi >= GPT_GPI_SECURE) && (gpi <= GPT_GPI_REALM))) {
		return true;
	}
	return false;
}

/*
 * This function checks to see if two PAS regions overlap.
 *
 * Parameters
 *   base_1: base address of first PAS
 *   size_1: size of first PAS
 *   base_2: base address of second PAS
 *   size_2: size of second PAS
 *
 * Return
 *   True if PAS regions overlap, false if they do not.
 */
static bool check_pas_overlap(uintptr_t base_1, size_t size_1,
			      uintptr_t base_2, size_t size_2)
{
	if (((base_1 + size_1) > base_2) && ((base_2 + size_2) > base_1)) {
		return true;
	}
	return false;
}

/*
 * This helper function checks to see if a PAS region from index 0 to
 * (pas_idx - 1) occupies the L0 region at index l0_idx in the L0 table.
 *
 * Parameters
 *   l0_idx:      Index of the L0 entry to check
 *   pas_regions: PAS region array
 *   pas_idx:     Upper bound of the PAS array index.
 *
 * Return
 *   True if a PAS region occupies the L0 region in question, false if not.
 */
static bool does_previous_pas_exist_here(unsigned int l0_idx,
					 pas_region_t *pas_regions,
					 unsigned int pas_idx)
{
	/* Iterate over PAS regions up to pas_idx */
	for (unsigned int i = 0U; i < pas_idx; i++) {
		if (check_pas_overlap((GPT_L0GPTSZ_ACTUAL_SIZE * l0_idx),
		    GPT_L0GPTSZ_ACTUAL_SIZE,
		    pas_regions[i].base_pa, pas_regions[i].size)) {
			return true;
		}
	}
	return false;
}

/*
 * This function iterates over all of the PAS regions and checks them to ensure
 * proper alignment of base and size, that the GPI is valid, and that no regions
 * overlap. As a part of the overlap checks, this function checks existing L0
 * mappings against the new PAS regions in the event that gpt_init_pas_l1_tables
 * is called multiple times to place L1 tables in different areas of memory. It
 * also counts the number of L1 tables needed and returns it on success.
 *
 * Parameters
 *   *pas_regions	Pointer to array of PAS region structures.
 *   pas_region_cnt	Total number of PAS regions in the array.
 *
 * Return
 *   Negative Linux error code in the event of a failure, number of L1 regions
 *   required when successful.
 */
static int validate_pas_mappings(pas_region_t *pas_regions,
				 unsigned int pas_region_cnt)
{
	unsigned int idx;
	unsigned int l1_cnt = 0U;
	unsigned int pas_l1_cnt;
	uint64_t *l0_desc = (uint64_t *)gpt_config.plat_gpt_l0_base;

	assert(pas_regions != NULL);
	assert(pas_region_cnt != 0U);

	for (idx = 0U; idx < pas_region_cnt; idx++) {
		/* Check for arithmetic overflow in region */
		if ((ULONG_MAX - pas_regions[idx].base_pa) <
		    pas_regions[idx].size) {
			ERROR("GPT: Address overflow in PAS[%u]!\n", idx);
			return -EOVERFLOW;
		}

		/* Initial checks for PAS validity */
		if (((pas_regions[idx].base_pa + pas_regions[idx].size) >
		    GPT_PPS_ACTUAL_SIZE(gpt_config.t)) ||
		    !is_gpi_valid(GPT_PAS_ATTR_GPI(pas_regions[idx].attrs))) {
			ERROR("GPT: PAS[%u] is invalid!\n", idx);
			return -EFAULT;
		}

		/*
		 * Make sure this PAS does not overlap with another one. We
		 * start from idx + 1 instead of 0 since prior PAS mappings will
		 * have already checked themselves against this one.
		 */
		for (unsigned int i = idx + 1U; i < pas_region_cnt; i++) {
			if (check_pas_overlap(pas_regions[idx].base_pa,
			    pas_regions[idx].size,
			    pas_regions[i].base_pa,
			    pas_regions[i].size)) {
				ERROR("GPT: PAS[%u] overlaps with PAS[%u]\n",
					i, idx);
				return -EFAULT;
			}
		}

		/*
		 * Since this function can be called multiple times with
		 * separate L1 tables we need to check the existing L0 mapping
		 * to see if this PAS would fall into one that has already been
		 * initialized.
		 */
		for (unsigned int i = GPT_L0_IDX(pas_regions[idx].base_pa);
		     i <= GPT_L0_IDX(pas_regions[idx].base_pa +
						pas_regions[idx].size - 1UL);
		     i++) {
			if ((GPT_L0_TYPE(l0_desc[i]) == GPT_L0_TYPE_BLK_DESC) &&
			    (GPT_L0_BLKD_GPI(l0_desc[i]) == GPT_GPI_ANY)) {
				/* This descriptor is unused so continue */
				continue;
			}

			/*
			 * This descriptor has been initialized in a previous
			 * call to this function so cannot be initialized again.
			 */
			ERROR("GPT: PAS[%u] overlaps with previous L0[%d]!\n",
			      idx, i);
			return -EFAULT;
		}

		/* Check for block mapping (L0) type */
		if (GPT_PAS_ATTR_MAP_TYPE(pas_regions[idx].attrs) ==
		    GPT_PAS_ATTR_MAP_TYPE_BLOCK) {
			/* Make sure base and size are block-aligned */
			if (!GPT_IS_L0_ALIGNED(pas_regions[idx].base_pa) ||
			    !GPT_IS_L0_ALIGNED(pas_regions[idx].size)) {
				ERROR("GPT: PAS[%u] is not block-aligned!\n",
				      idx);
				return -EFAULT;
			}

			continue;
		}

		/* Check for granule mapping (L1) type */
		if (GPT_PAS_ATTR_MAP_TYPE(pas_regions[idx].attrs) ==
		    GPT_PAS_ATTR_MAP_TYPE_GRANULE) {
			/* Make sure base and size are granule-aligned */
			if (!GPT_IS_L1_ALIGNED(gpt_config.p, pas_regions[idx].base_pa) ||
			    !GPT_IS_L1_ALIGNED(gpt_config.p, pas_regions[idx].size)) {
				ERROR("GPT: PAS[%u] is not granule-aligned!\n",
				      idx);
				return -EFAULT;
			}

			/* Find how many L1 tables this PAS occupies */
			pas_l1_cnt = (GPT_L0_IDX(pas_regions[idx].base_pa +
				     pas_regions[idx].size - 1UL) -
				     GPT_L0_IDX(pas_regions[idx].base_pa) + 1U);

			/*
			 * This creates a situation where, if multiple PAS
			 * regions occupy the same table descriptor, we can get
			 * an artificially high total L1 table count. The way we
			 * handle this is by checking each PAS against those
			 * before it in the array, and if they both occupy the
			 * same PAS we subtract from pas_l1_cnt and only the
			 * first PAS in the array gets to count it.
			 */

			/*
			 * If L1 count is greater than 1 we know the start and
			 * end PAs are in different L0 regions so we must check
			 * both for overlap against other PAS.
			 */
			if (pas_l1_cnt > 1) {
				if (does_previous_pas_exist_here(
				    GPT_L0_IDX(pas_regions[idx].base_pa +
				    pas_regions[idx].size - 1UL),
				    pas_regions, idx)) {
					pas_l1_cnt--;
				}
			}

			if (does_previous_pas_exist_here(
			    GPT_L0_IDX(pas_regions[idx].base_pa),
			    pas_regions, idx)) {
				pas_l1_cnt--;
			}

			l1_cnt += pas_l1_cnt;
			continue;
		}

		/* If execution reaches this point, mapping type is invalid */
		ERROR("GPT: PAS[%u] has invalid mapping type 0x%x.\n", idx,
		      GPT_PAS_ATTR_MAP_TYPE(pas_regions[idx].attrs));
		return -EINVAL;
	}

	return l1_cnt;
}

/*
 * This function validates L0 initialization parameters.
 *
 * Parameters
 *   l0_mem_base	Base address of memory used for L0 tables.
 *   l1_mem_size	Size of memory available for L0 tables.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
static int validate_l0_params(gpccr_pps_e pps, uintptr_t l0_mem_base,
				size_t l0_mem_size)
{
	size_t l0_alignment;

	/*
	 * Make sure PPS is valid and then store it since macros need this value
	 * to work.
	 */
	if (pps > GPT_PPS_MAX) {
		ERROR("GPT: Invalid PPS: 0x%x\n", pps);
		return -EINVAL;
	}
	gpt_config.pps = pps;
	gpt_config.t = gpt_t_lookup[pps];

	/* Alignment must be the greater of 4KB or l0 table size */
	l0_alignment = PAGE_SIZE_4KB;
	if (l0_alignment < GPT_L0_TABLE_SIZE(gpt_config.t)) {
		l0_alignment = GPT_L0_TABLE_SIZE(gpt_config.t);
	}

	/* Check base address */
	if ((l0_mem_base == 0UL) ||
	   ((l0_mem_base & (l0_alignment - 1UL)) != 0UL)) {
		ERROR("GPT: Invalid L0 base address: 0x%lx\n", l0_mem_base);
		return -EFAULT;
	}

	/* Check size */
	if (l0_mem_size < GPT_L0_TABLE_SIZE(gpt_config.t)) {
		ERROR("%sL0%s\n", "GPT: Inadequate ", " memory\n");
		ERROR("      Expected 0x%lx bytes, got 0x%lx bytes\n",
		      GPT_L0_TABLE_SIZE(gpt_config.t),
		      l0_mem_size);
		return -ENOMEM;
	}

	return 0;
}

/*
 * In the event that L1 tables are needed, this function validates
 * the L1 table generation parameters.
 *
 * Parameters
 *   l1_mem_base	Base address of memory used for L1 table allocation.
 *   l1_mem_size	Total size of memory available for L1 tables.
 *   l1_gpt_cnt		Number of L1 tables needed.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
static int validate_l1_params(uintptr_t l1_mem_base, size_t l1_mem_size,
				unsigned int l1_gpt_cnt)
{
	size_t l1_gpt_mem_sz;

	/* Check if the granularity is supported */
	if (!xlat_arch_is_granule_size_supported(
	    GPT_PGS_ACTUAL_SIZE(gpt_config.p))) {
		return -EPERM;
	}

	/* Make sure L1 tables are aligned to their size */
	if ((l1_mem_base & (GPT_L1_TABLE_SIZE(gpt_config.p) - 1UL)) != 0UL) {
		ERROR("GPT: Unaligned L1 GPT base address: 0x%"PRIxPTR"\n",
		      l1_mem_base);
		return -EFAULT;
	}

	/* Get total memory needed for L1 tables */
	l1_gpt_mem_sz = l1_gpt_cnt * GPT_L1_TABLE_SIZE(gpt_config.p);

	/* Check for overflow */
	if ((l1_gpt_mem_sz / GPT_L1_TABLE_SIZE(gpt_config.p)) != l1_gpt_cnt) {
		ERROR("GPT: Overflow calculating L1 memory size\n");
		return -ENOMEM;
	}

	/* Make sure enough space was supplied */
	if (l1_mem_size < l1_gpt_mem_sz) {
		ERROR("%sL1 GPTs%s", "GPT: Inadequate ", " memory\n");
		ERROR("      Expected 0x%lx bytes, got 0x%lx bytes\n",
		      l1_gpt_mem_sz, l1_mem_size);
		return -ENOMEM;
	}

	VERBOSE("GPT: Requested 0x%lx bytes for L1 GPTs\n", l1_gpt_mem_sz);
	return 0;
}

/*
 * This function initializes L0 block descriptors (regions that cannot be
 * transitioned at the granule level) according to the provided PAS.
 *
 * Parameters
 *   *pas		Pointer to the structure defining the PAS region to
 *			initialize.
 */
static void generate_l0_blk_desc(pas_region_t *pas)
{
	uint64_t gpt_desc;
	unsigned int end_idx;
	unsigned int idx;
	uint64_t *l0_gpt_arr;

	assert(gpt_config.plat_gpt_l0_base != 0U);
	assert(pas != NULL);

	/*
	 * Checking of PAS parameters has already been done in
	 * validate_pas_mappings so no need to check the same things again.
	 */

	l0_gpt_arr = (uint64_t *)gpt_config.plat_gpt_l0_base;

	/* Create the GPT Block descriptor for this PAS region */
	gpt_desc = GPT_L0_BLK_DESC(GPT_PAS_ATTR_GPI(pas->attrs));

	/* Start index of this region in L0 GPTs */
	idx = GPT_L0_IDX(pas->base_pa);

	/*
	 * Determine number of L0 GPT descriptors covered by
	 * this PAS region and use the count to populate these
	 * descriptors.
	 */
	end_idx = GPT_L0_IDX(pas->base_pa + pas->size);

	/* Generate the needed block descriptors */
	for (; idx < end_idx; idx++) {
		l0_gpt_arr[idx] = gpt_desc;
		VERBOSE("GPT: L0 entry (BLOCK) index %u [%p]: GPI = 0x%"PRIx64" (0x%"PRIx64")\n",
			idx, &l0_gpt_arr[idx],
			(gpt_desc >> GPT_L0_BLK_DESC_GPI_SHIFT) &
			GPT_L0_BLK_DESC_GPI_MASK, l0_gpt_arr[idx]);
	}
}

/*
 * Helper function to determine if the end physical address lies in the same L0
 * region as the current physical address. If true, the end physical address is
 * returned else, the start address of the next region is returned.
 *
 * Parameters
 *   cur_pa		Physical address of the current PA in the loop through
 *			the range.
 *   end_pa		Physical address of the end PA in a PAS range.
 *
 * Return
 *   The PA of the end of the current range.
 */
static uintptr_t get_l1_end_pa(uintptr_t cur_pa, uintptr_t end_pa)
{
	uintptr_t cur_idx;
	uintptr_t end_idx;

	cur_idx = GPT_L0_IDX(cur_pa);
	end_idx = GPT_L0_IDX(end_pa);

	assert(cur_idx <= end_idx);

	if (cur_idx == end_idx) {
		return end_pa;
	}

	return (cur_idx + 1U) << GPT_L0_IDX_SHIFT;
}

/*
 * Helper function to fill out GPI entries in a single L1 table with
 * Contiguous descriptors.
 *
 * Parameters
 *   l1			Pointer to L1 table to fill out
 *   first		Address of first granule in range
 *   length		Length of the range in bytes
 *   l1_desc		GPT Granules descriptor set this range to
 *
 * Return
 *   Address of next granule in range.
 */
static uintptr_t fill_l1_cont_desc(uint64_t *l1, uintptr_t first,
				   size_t length, uint64_t l1_desc)
{
	/*
	 * Iterate through all block sizes (512MB, 32MB and 2MB)
	 * starting with maximum supported.
	 */
	for (unsigned long i = 0UL; i < ARRAY_SIZE(gpt_fill_lookup); ) {
		/* Calculate index */
		unsigned int idx = GPT_L1_INDEX(first);

		/* Contiguous block size */
		size_t cont_size = gpt_fill_lookup[i].size;

		if (GPT_REGION_IS_CONT(length, first, cont_size)) {
			/* Generate Contiguous descriptor */
			uint64_t l1_cont_desc =	GPT_L1_CONT_DESC(l1_desc,
						gpt_fill_lookup[i].contig);

			/* Number pf 128-bit L1 entries in block */
			unsigned int cnt;

			switch (cont_size) {
			case SZ_512M:
				cnt = QWORDS_512MB;
				break;
			case SZ_32M:
				cnt = QWORDS_32MB;
				break;
			default:			/* SZ_2MB */
				cnt = QWORDS_2MB;
			}

			VERBOSE("GPT: Contiguous descriptor 0x%"PRIxPTR" %luMB\n",
				first, cont_size / SZ_1M);

			/* Fill Contiguous descriptors */
			(void)fill_desc(&l1[idx], l1_cont_desc, cnt);
			first += cont_size;
			length -= cont_size;

			if (length == 0UL) {
				break;
			}
		} else {
			++i;	/* Next contiguous block */
		}
	}

	return first;
}

/*
 * Helper function to fill out GPI entries in a single L1 table with
 * Granules descriptors.
 * This function is called when the address of the first granule is not
 * aligned to at least 2MB or the length of the range is less than 2MB.
 * If the size of the range is greater than 2MB, function checks the length
 * of the remaining range for 2MB aligned address and if it is less than 2MB,
 * Granules descriptors are written for the full range. Otherwise Granules
 * descriptors are written for the range up to the next 2MB aligned address,
 * and function returns, so the remaining range can be filled with Contiguous
 * descriptors.
 *
 * Parameters
 *   l1			Pointer to L1 table to fill out
 *   first		Address of first granule in range
 *   last		Address of last granule in range (inclusive)
 *   length		Length of the range in bytes
 *   l1_desc		GPT Granules descriptor set this range to
 *
 * Return
 *   Address of next granule in range.
 */
static uintptr_t fill_l1_gran_desc(uint64_t *l1, uintptr_t first,
				   uintptr_t last, size_t length,
				   uint64_t l1_desc)
{
	uint64_t gpi_mask;
	uintptr_t new_last;

	/*
	 * If the region length is >= 2MB, we can try to
	 * write data up to the next 2MB aligned address.
	 */
	if (length >= SZ_2M) {
		new_last = ALIGN_2MB(first + SZ_2M) -
				GPT_PGS_ACTUAL_SIZE(gpt_config.p);
		/*
		 * If the remaining length is < 2MB, proceed all data,
		 * otherwise the remaining data will be written as
		 * Contiguous descriptor in the next iteration.
		 */
		if ((last - new_last) < SZ_2M) {
			new_last = last;
		}
	} else {
		new_last = last;
	}

	/* Shift the mask if we're starting in the middle of an L1 entry */
	gpi_mask = ULONG_MAX << (GPT_L1_GPI_IDX(gpt_config.p, first) << 2);

	/* Fill out each L1 entry for this region */
	for (unsigned int i = GPT_L1_INDEX(first);
	     i <= GPT_L1_INDEX(new_last); i++) {
		/* Account for stopping in the middle of an L1 entry */
		if (i == GPT_L1_INDEX(new_last)) {
			gpi_mask &= (gpi_mask >> ((15U -
				    GPT_L1_GPI_IDX(gpt_config.p, new_last)) << 2));
		}

		/* Write GPI values */
		assert((l1[i] & gpi_mask) == (GPT_L1_ANY_DESC & gpi_mask));

		l1[i] = (l1[i] & ~gpi_mask) | (l1_desc & gpi_mask);

		/* Reset mask */
		gpi_mask = ULONG_MAX;
	}
	return new_last + GPT_PGS_ACTUAL_SIZE(gpt_config.p);
}

/*
 * Helper function to fill out GPI entries in a single L1 table.
 * This function fills out entire L1 table with either Contiguous
 * or Granules descriptors depending on region length and alignment.
 *
 * Parameters
 *   l1			Pointer to L1 table to fill out
 *   first		Address of first granule in range
 *   last		Address of last granule in range (inclusive)
 *   l1_desc		GPT Granules descriptor set this range to
 */
static void fill_l1_tbl(uint64_t *l1, uintptr_t first, uintptr_t last,
			uint64_t l1_desc)
{
	assert(l1 != NULL);
	assert(first <= last);
	assert((first & (GPT_PGS_ACTUAL_SIZE(gpt_config.p) - 1UL)) == 0UL);
	assert((last & (GPT_PGS_ACTUAL_SIZE(gpt_config.p) - 1UL)) == 0UL);
	assert(GPT_L0_IDX(first) == GPT_L0_IDX(last));

	while (first < last) {
		/* Region length */
		size_t length = last - first + GPT_PGS_ACTUAL_SIZE(gpt_config.p);

		/*
		 * Start with check for 2MB aligned contiguous region.
		 * If the condition is met, 'fill_l1_cont_desc()' will
		 * iterate through all block sizes (512MB, 32MB and 2MB)
		 * supported and fill corresponding Contiguous descriptors.
		 */
		if (GPT_REGION_IS_CONT(length, first, SZ_2M)) {
			first = fill_l1_cont_desc(l1, first, length, l1_desc);
			if (first > last)  {
				break;
			}
			length = last - first + GPT_PGS_ACTUAL_SIZE(gpt_config.p);
		}
		/* Fill with Granules descriptors */
		first = fill_l1_gran_desc(l1, first, last, length, l1_desc);
	}

	assert(first == (last + GPT_PGS_ACTUAL_SIZE(gpt_config.p)));
}

/*
 * This function finds the next available unused L1 table and initializes all
 * granules descriptor entries to GPI_ANY. This ensures that there are no chunks
 * of GPI_NO_ACCESS (0b0000) memory floating around in the system in the
 * event that a PAS region stops midway through an L1 table, thus guaranteeing
 * that all memory not explicitly assigned is GPI_ANY. This function does not
 * check for overflow conditions, that should be done by the caller.
 *
 * Return
 *   Pointer to the next available L1 table.
 */
static uint64_t *get_new_l1_tbl(void)
{
	/* Retrieve the next L1 table */
	uint64_t *l1 = (uint64_t *)((uint64_t)(gpt_l1_tbl) +
		       (GPT_L1_TABLE_SIZE(gpt_config.p) *
		       gpt_next_l1_tbl_idx));

	/* Increment L1 counter */
	gpt_next_l1_tbl_idx++;

	/* Initialize all GPIs to GPT_GPI_ANY */
	for (unsigned int i = 0U; i < GPT_L1_ENTRY_COUNT(gpt_config.p); i++) {
		l1[i] = GPT_L1_ANY_DESC;
	}
	return l1;
}

/*
 * When L1 tables are needed, this function creates the necessary L0 table
 * descriptors and fills out the L1 table entries according to the supplied
 * PAS range.
 *
 * Parameters
 *   *pas		Pointer to the structure defining the PAS region.
 */
static void generate_l0_tbl_desc(pas_region_t *pas)
{
	uintptr_t end_pa;
	uintptr_t cur_pa;
	uintptr_t last_gran_pa;
	uint64_t *l0_gpt_base;
	uint64_t *l1_gpt_arr;
	unsigned int l0_idx;

	assert(gpt_config.plat_gpt_l0_base != 0U);
	assert(pas != NULL);

	/*
	 * Checking of PAS parameters has already been done in
	 * validate_pas_mappings so no need to check the same things again.
	 */
	end_pa = pas->base_pa + pas->size;
	l0_gpt_base = (uint64_t *)gpt_config.plat_gpt_l0_base;

	/* We start working from the granule at base PA */
	cur_pa = pas->base_pa;

	/* Iterate over each L0 region in this memory range */
	for (l0_idx = GPT_L0_IDX(pas->base_pa);
	     l0_idx <= GPT_L0_IDX(end_pa - 1U);
	     l0_idx++) {
		/*
		 * See if the L0 entry is already a table descriptor or if we
		 * need to create one.
		 */
		if (GPT_L0_TYPE(l0_gpt_base[l0_idx]) == GPT_L0_TYPE_TBL_DESC) {
			/* Get the L1 array from the L0 entry */
			l1_gpt_arr = GPT_L0_TBLD_ADDR(l0_gpt_base[l0_idx]);
		} else {
			/* Get a new L1 table from the L1 memory space */
			l1_gpt_arr = get_new_l1_tbl();

			/* Fill out the L0 descriptor and flush it */
			l0_gpt_base[l0_idx] = GPT_L0_TBL_DESC(l1_gpt_arr);
		}

		VERBOSE("GPT: L0 entry (TABLE) index %u [%p] ==> L1 Addr %p (0x%"PRIx64")\n",
			l0_idx, &l0_gpt_base[l0_idx], l1_gpt_arr, l0_gpt_base[l0_idx]);

		/*
		 * Determine the PA of the last granule in this L0 descriptor.
		 */
		last_gran_pa = get_l1_end_pa(cur_pa, end_pa) -
			       GPT_PGS_ACTUAL_SIZE(gpt_config.p);

		/*
		 * Fill up L1 GPT entries between these two addresses. This
		 * function needs the addresses of the first granule and last
		 * granule in the range.
		 */
		fill_l1_tbl(l1_gpt_arr, cur_pa, last_gran_pa,
			    GPT_BUILD_L1_DESC(GPT_PAS_ATTR_GPI(pas->attrs)));

		/* Advance cur_pa to first granule in next L0 region */
		cur_pa = get_l1_end_pa(cur_pa, end_pa);
	}
}

/*
 * This function flushes a range of L0 descriptors used by a given PAS region
 * array. There is a chance that some unmodified L0 descriptors would be flushed
 * in the case that there are "holes" in an array of PAS regions but overall
 * this should be faster than individually flushing each modified L0 descriptor
 * as they are created.
 *
 * Parameters
 *   *pas		Pointer to an array of PAS regions.
 *   pas_count		Number of entries in the PAS array.
 */
static void flush_l0_for_pas_array(pas_region_t *pas, unsigned int pas_count)
{
	unsigned int idx;
	unsigned int start_idx;
	unsigned int end_idx;
	uint64_t *l0 = (uint64_t *)gpt_config.plat_gpt_l0_base;

	assert(pas != NULL);
	assert(pas_count != 0U);

	/* Initial start and end values */
	start_idx = GPT_L0_IDX(pas[0].base_pa);
	end_idx = GPT_L0_IDX(pas[0].base_pa + pas[0].size - 1UL);

	/* Find lowest and highest L0 indices used in this PAS array */
	for (idx = 1U; idx < pas_count; idx++) {
		if (GPT_L0_IDX(pas[idx].base_pa) < start_idx) {
			start_idx = GPT_L0_IDX(pas[idx].base_pa);
		}
		if (GPT_L0_IDX(pas[idx].base_pa + pas[idx].size - 1UL) > end_idx) {
			end_idx = GPT_L0_IDX(pas[idx].base_pa + pas[idx].size - 1UL);
		}
	}

	/*
	 * Flush all covered L0 descriptors, add 1 because we need to include
	 * the end index value.
	 */
	flush_dcache_range((uintptr_t)&l0[start_idx],
			   ((end_idx + 1U) - start_idx) * sizeof(uint64_t));
}

/*
 * Public API to enable granule protection checks once the tables have all been
 * initialized. This function is called at first initialization and then again
 * later during warm boots of CPU cores.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_enable(void)
{
	u_register_t gpccr_el3;

	/*
	 * Granule tables must be initialised before enabling
	 * granule protection.
	 */
	if (gpt_config.plat_gpt_l0_base == 0UL) {
		ERROR("GPT: Tables have not been initialized!\n");
		return -EPERM;
	}

	/* Write the base address of the L0 tables into GPTBR */
	write_gptbr_el3(((gpt_config.plat_gpt_l0_base >> GPTBR_BADDR_VAL_SHIFT)
			>> GPTBR_BADDR_SHIFT) & GPTBR_BADDR_MASK);

	/* GPCCR_EL3.PPS */
	gpccr_el3 = SET_GPCCR_PPS(gpt_config.pps);

	/* GPCCR_EL3.PGS */
	gpccr_el3 |= SET_GPCCR_PGS(gpt_config.pgs);

	/*
	 * Since EL3 maps the L1 region as Inner shareable, use the same
	 * shareability attribute for GPC as well so that
	 * GPC fetches are visible to PEs
	 */
	gpccr_el3 |= SET_GPCCR_SH(GPCCR_SH_IS);

	/* Outer and Inner cacheability set to Normal memory, WB, RA, WA */
	gpccr_el3 |= SET_GPCCR_ORGN(GPCCR_ORGN_WB_RA_WA);
	gpccr_el3 |= SET_GPCCR_IRGN(GPCCR_IRGN_WB_RA_WA);

	/* Prepopulate GPCCR_EL3 but don't enable GPC yet */
	write_gpccr_el3(gpccr_el3);
	isb();

	/* Invalidate any stale TLB entries and any cached register fields */
	tlbipaallos();
	dsb();
	isb();

	/* Enable GPT */
	gpccr_el3 |= GPCCR_GPC_BIT;

	/* TODO: Configure GPCCR_EL3_GPCP for Fault control */
	write_gpccr_el3(gpccr_el3);
	isb();
	tlbipaallos();
	dsb();
	isb();

	return 0;
}

/*
 * Public API to disable granule protection checks.
 */
void gpt_disable(void)
{
	u_register_t gpccr_el3 = read_gpccr_el3();

	write_gpccr_el3(gpccr_el3 & ~GPCCR_GPC_BIT);
	dsbsy();
	isb();
}

/*
 * Public API that initializes the entire protected space to GPT_GPI_ANY using
 * the L0 tables (block descriptors). Ideally, this function is invoked prior
 * to DDR discovery and initialization. The MMU must be initialized before
 * calling this function.
 *
 * Parameters
 *   pps		PPS value to use for table generation
 *   l0_mem_base	Base address of L0 tables in memory.
 *   l0_mem_size	Total size of memory available for L0 tables.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_init_l0_tables(gpccr_pps_e pps, uintptr_t l0_mem_base,
		       size_t l0_mem_size)
{
	int ret;
	uint64_t gpt_desc;

	/* Ensure that MMU and Data caches are enabled */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0U);

	/* Validate other parameters */
	ret = validate_l0_params(pps, l0_mem_base, l0_mem_size);
	if (ret != 0) {
		return ret;
	}

	/* Create the descriptor to initialize L0 entries with */
	gpt_desc = GPT_L0_BLK_DESC(GPT_GPI_ANY);

	/* Iterate through all L0 entries */
	for (unsigned int i = 0U; i < GPT_L0_REGION_COUNT(gpt_config.t); i++) {
		((uint64_t *)l0_mem_base)[i] = gpt_desc;
	}

	/* Flush updated L0 tables to memory */
	flush_dcache_range((uintptr_t)l0_mem_base,
			   (size_t)GPT_L0_TABLE_SIZE(gpt_config.t));

	/* Stash the L0 base address once initial setup is complete */
	gpt_config.plat_gpt_l0_base = l0_mem_base;

	/* Save total size of memory available for L0 tables */
	gpt_l0_mem_size = l0_mem_size;

	return 0;
}

/*
 * Public API that carves out PAS regions from the L0 tables and builds any L1
 * tables that are needed. This function ideally is run after DDR discovery and
 * initialization. The L0 tables must have already been initialized to GPI_ANY
 * when this function is called.
 *
 * This function can be called multiple times with different L1 memory ranges
 * and PAS regions if it is desirable to place L1 tables in different locations
 * in memory. (ex: you have multiple DDR banks and want to place the L1 tables
 * in the DDR bank that they control)
 *
 * Parameters
 *   pgs		PGS value to use for table generation.
 *   l1_mem_base	Base address of memory used for L1 tables.
 *   l1_mem_size	Total size of memory available for L1 tables.
 *   *pas_regions	Pointer to PAS regions structure array.
 *   pas_count		Total number of PAS regions.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_init_pas_l1_tables(gpccr_pgs_e pgs, uintptr_t l1_mem_base,
			   size_t l1_mem_size, pas_region_t *pas_regions,
			   unsigned int pas_count)
{
	int l1_gpt_cnt, ret;
	size_t locks_size;
	bitlock_t *bit_locks;

	/* Ensure that MMU and Data caches are enabled */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0U);

	/* PGS is needed for validate_pas_mappings so check it now */
	if (pgs > GPT_PGS_MAX) {
		ERROR("GPT: Invalid PGS: 0x%x\n", pgs);
		return -EINVAL;
	}
	gpt_config.pgs = pgs;
	gpt_config.p = gpt_p_lookup[pgs];

	/* Make sure L0 tables have been initialized */
	if (gpt_config.plat_gpt_l0_base == 0U) {
		ERROR("GPT: L0 tables must be initialized first!\n");
		return -EPERM;
	}

	/* Check if L1 GPTs are required and how many */
	l1_gpt_cnt = validate_pas_mappings(pas_regions, pas_count);
	if (l1_gpt_cnt < 0) {
		return l1_gpt_cnt;
	}

	VERBOSE("GPT: %i L1 GPTs requested\n", l1_gpt_cnt);

	/* If L1 tables are needed then validate the L1 parameters */
	if (l1_gpt_cnt > 0) {
		ret = validate_l1_params(l1_mem_base, l1_mem_size,
					(unsigned int)l1_gpt_cnt);
		if (ret != 0) {
			return ret;
		}

		/* Set up parameters for L1 table generation */
		gpt_l1_tbl = l1_mem_base;
		gpt_next_l1_tbl_idx = 0U;
	}

	/* Number of L1 entries in 2MB depends on GPCCR_EL3.PGS value */
	gpt_l1_cnt_2mb = GPT_L1_ENTRY_COUNT_2MB(gpt_config.p);

	/* Mask for the L1 index field */
	gpt_l1_index_mask = GPT_L1_IDX_MASK(gpt_config.p);

	/* Size of bit locks in bytes */
	locks_size = (l1_gpt_cnt << (GPT_L0GPTSZ + 1U)) / 8U;

	/* Check space for bit locks */
	if (locks_size > (gpt_l0_mem_size - GPT_L0_TABLE_SIZE(gpt_config.t))) {
		ERROR("%sbitlock%s", "GPT: Inadequate ", " memory\n");
		ERROR("      Expected 0x%lx bytes, got 0x%lx bytes\n",
			locks_size,
			gpt_l0_mem_size - GPT_L0_TABLE_SIZE(gpt_config.t));
		return -ENOMEM;
	}

	/* Initialise bitlocks at the end of L0 table */
	bit_locks = (bitlock_t *)(gpt_config.plat_gpt_l0_base +
					GPT_L0_TABLE_SIZE(gpt_config.t));

	for (size_t i = 0UL; i < (locks_size / LOCK_SIZE); i++) {
		bit_locks[i].lock = 0U;
	}

	INFO("GPT: Boot Configuration\n");
	INFO("  PPS/T:     0x%x/%u\n", gpt_config.pps, gpt_config.t);
	INFO("  PGS/P:     0x%x/%u\n", gpt_config.pgs, gpt_config.p);
	INFO("  L0GPTSZ/S: 0x%x/%u\n", GPT_L0GPTSZ, GPT_S_VAL);
	INFO("  PAS count: %u\n", pas_count);
	INFO("  L0 base:   0x%"PRIxPTR"\n", gpt_config.plat_gpt_l0_base);
	INFO("  Bitlocks:  0x%"PRIxPTR" %lu\n", (uintptr_t)bit_locks, locks_size);

	/* Generate the tables in memory */
	for (unsigned int idx = 0U; idx < pas_count; idx++) {
		VERBOSE("GPT: PAS[%u]: base 0x%"PRIxPTR"\tsize 0x%lx\tGPI 0x%x\ttype 0x%x\n",
			idx, pas_regions[idx].base_pa, pas_regions[idx].size,
			GPT_PAS_ATTR_GPI(pas_regions[idx].attrs),
			GPT_PAS_ATTR_MAP_TYPE(pas_regions[idx].attrs));

		/* Check if a block or table descriptor is required */
		if (GPT_PAS_ATTR_MAP_TYPE(pas_regions[idx].attrs) ==
		    GPT_PAS_ATTR_MAP_TYPE_BLOCK) {
			generate_l0_blk_desc(&pas_regions[idx]);

		} else {
			generate_l0_tbl_desc(&pas_regions[idx]);
		}
	}

	/* Flush modified L0 tables */
	flush_l0_for_pas_array(pas_regions, pas_count);

	/* Flush L1 tables and bitlocks if needed */
	if (l1_gpt_cnt > 0) {
		flush_dcache_range(l1_mem_base,
				   GPT_L1_TABLE_SIZE(gpt_config.p) *
				   l1_gpt_cnt);

		flush_dcache_range((uintptr_t)bit_locks, locks_size);
	}

	/* Make sure that all the entries are written to the memory */
	dsbishst();
	tlbipaallos();
	dsb();
	isb();

	return 0;
}

/*
 * Public API to initialize the runtime gpt_config structure based on the values
 * present in the GPTBR_EL3 and GPCCR_EL3 registers. GPT initialization
 * typically happens in a bootloader stage prior to setting up the EL3 runtime
 * environment for the granule transition service so this function detects the
 * initialization from a previous stage. Granule protection checks must be
 * enabled already or this function will return an error.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_runtime_init(void)
{
	u_register_t reg;
	uint64_t *l0_desc;

	/* Ensure that MMU and Data caches are enabled */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0U);

	/* Ensure GPC are already enabled */
	if ((read_gpccr_el3() & GPCCR_GPC_BIT) == 0U) {
		ERROR("GPT: Granule protection checks are not enabled!\n");
		return -EPERM;
	}

	/*
	 * Read the L0 table address from GPTBR, we don't need the L1 base
	 * address since those are included in the L0 tables as needed.
	 */
	reg = read_gptbr_el3();
	gpt_config.plat_gpt_l0_base = ((reg >> GPTBR_BADDR_SHIFT) &
				      GPTBR_BADDR_MASK) <<
				      GPTBR_BADDR_VAL_SHIFT;

	/* Read GPCCR to get PGS and PPS values */
	reg = read_gpccr_el3();
	gpt_config.pps = (reg >> GPCCR_PPS_SHIFT) & GPCCR_PPS_MASK;
	gpt_config.t = gpt_t_lookup[gpt_config.pps];
	gpt_config.pgs = (reg >> GPCCR_PGS_SHIFT) & GPCCR_PGS_MASK;
	gpt_config.p = gpt_p_lookup[gpt_config.pgs];

	l0_desc = (uint64_t *)gpt_config.plat_gpt_l0_base;

	/* Find the first L1 entry */
	for (unsigned int l0_idx = 0U;
	     l0_idx < GPT_L0_TABLE_SIZE(gpt_config.t) / sizeof(uint64_t);
	     l0_idx++) {
		if (GPT_L0_TYPE(l0_desc[l0_idx]) == GPT_L0_TYPE_TBL_DESC) {
			/*
			 * Set address of the 1st L1 table used for
			 * calculation of L1 table index.
			 */
			gpt_l1_first = GPT_L0_TBLD_ADDR(l0_desc[l0_idx]);
			break;
		}
	}

	/* Number of L1 entries in 2MB depends on GPCCR_EL3.PGS value */
	gpt_l1_cnt_2mb = GPT_L1_ENTRY_COUNT_2MB(gpt_config.p);

	/* Mask for the L1 index field */
	gpt_l1_index_mask = GPT_L1_IDX_MASK(gpt_config.p);

	/* log2(GPT_L1_TABLE_SIZE) */
	gpt_l1_size_bit = __builtin_ctzl(GPT_L1_TABLE_SIZE(gpt_config.p));

	/* log2(Number of L1 entries in 512MB) */
	gpt_512mb_bit = gpt_s_lookup[gpt_config.pgs];

	/* Bit locks at the end of L0 table */
	gpt_locks = (bitlock_t *)(gpt_config.plat_gpt_l0_base +
					GPT_L0_TABLE_SIZE(gpt_config.t));

	/* log2(Number of lock bits per L1 entry) */
	gpt_lock_bits = GPT_L0GPTSZ + 1U;

	VERBOSE("GPT: Runtime Configuration\n");
	VERBOSE("  PPS/T:     0x%x/%u\n", gpt_config.pps, gpt_config.t);
	VERBOSE("  PGS/P:     0x%x/%u\n", gpt_config.pgs, gpt_config.p);
	VERBOSE("  L0GPTSZ/S: 0x%x/%u\n", GPT_L0GPTSZ, GPT_S_VAL);
	VERBOSE("  L0 base:   0x%"PRIxPTR"\n", gpt_config.plat_gpt_l0_base);
	VERBOSE("  Bitlocks:  0x%"PRIxPTR"\n", (uintptr_t)gpt_locks);

	return 0;
}

/*
 * A helper to write the value (target_pas << gpi_shift) to the index of
 * the gpt_l1_addr.
 */
static inline void write_gpt(uint64_t *gpt_l1_desc, uint64_t *gpt_l1_addr,
			     unsigned int gpi_shift, unsigned int idx,
			     unsigned int target_pas)
{
	*gpt_l1_desc &= ~(GPT_L1_GRAN_DESC_GPI_MASK << gpi_shift);
	*gpt_l1_desc |= ((uint64_t)target_pas << gpi_shift);
	gpt_l1_addr[idx] = *gpt_l1_desc;
}

/*
 * Helper to retrieve the gpt_l1_* information from the base address
 * returned in gpi_info.
 */
static int get_gpi_params(uint64_t base, gpi_info_t *gpi_info)
{
	uint64_t gpt_l0_desc, *gpt_l0_base;
	unsigned int l1_idx, n_512, offset;

	gpt_l0_base = (uint64_t *)gpt_config.plat_gpt_l0_base;
	gpt_l0_desc = gpt_l0_base[GPT_L0_IDX(base)];
	if (GPT_L0_TYPE(gpt_l0_desc) != GPT_L0_TYPE_TBL_DESC) {
		VERBOSE("GPT: Granule is not covered by a table descriptor!\n");
		VERBOSE("      Base=0x%"PRIx64"\n", base);
		return -EINVAL;
	}

	/* Get the table index and GPI shift from PA */
	gpi_info->gpt_l1_addr = GPT_L0_TBLD_ADDR(gpt_l0_desc);
	gpi_info->idx = GPT_L1_INDEX(base);
	gpi_info->gpi_shift = GPT_L1_GPI_IDX(gpt_config.p, base) << 2;

	/* L1 table index */
	l1_idx = (unsigned int)(((uintptr_t)gpi_info->gpt_l1_addr -
				 (uintptr_t)gpt_l1_first) >> gpt_l1_size_bit);

	/* 512MB block index */
	n_512 = gpi_info->idx >> gpt_512mb_bit;
	offset = (l1_idx << gpt_lock_bits) + n_512;

	gpi_info->bitlock = &gpt_locks[offset / LOCK_BITS];
	gpi_info->mask = 1U << (offset & (LOCK_BITS - 1U));

	return 0;
}

/*
 * Helper to retrieve the gpt_l1_desc and GPI information from gpi_info.
 * This function is called with bitlock acquired.
 */
static void read_gpi(gpi_info_t *gpi_info)
{
	gpi_info->gpt_l1_desc = (gpi_info->gpt_l1_addr)[gpi_info->idx];

	if ((gpi_info->gpt_l1_desc & GPT_L1_TYPE_CONT_DESC_MASK) ==
					GPT_L1_TYPE_CONT_DESC) {
		/* Read GPI from Coniguous descriptor */
		gpi_info->gpi = GPT_L1_CONT_GPI(gpi_info->gpt_l1_desc);
	} else {
		/* Read GPI from Granules descriptor */
		gpi_info->gpi = (gpi_info->gpt_l1_desc >> gpi_info->gpi_shift) &
				GPT_L1_GRAN_DESC_GPI_MASK;
	}
}

static void flush_page_to_popa(uintptr_t addr)
{
	if (is_feat_mte2_supported()) {
		flush_dcache_to_popa_range_mte2(addr,
					   GPT_PGS_ACTUAL_SIZE(gpt_config.p));
	} else {
		flush_dcache_to_popa_range(addr,
					   GPT_PGS_ACTUAL_SIZE(gpt_config.p));
	}
}

static void remove_access(uint64_t base, gpi_info_t *gpi_info, uint64_t nse)
{
	/* In order to maintain mutual distrust between Realm and Secure
	 * states, remove access now, in order to guarantee that writes
	 * to the currently-accessible physical address space will not
	 * later become observable.
	 */
	write_gpt(&gpi_info->gpt_l1_desc, gpi_info->gpt_l1_addr,
		  gpi_info->gpi_shift, gpi_info->idx, GPT_GPI_NO_ACCESS);
	dsboshst();

	/* Ensure that all agents observe the new NS configuration */
	tlbi_page(base);
	dsbosh();

	/* Ensure that the scrubbed data have made it past the PoPA */
	flush_page_to_popa(base | nse);

	/*
	 * Remove any data loaded speculatively in NS space from
	 * before the scrubbing.
	 */
	nse = (uint64_t)GPT_NSE_NS << GPT_NSE_SHIFT;

	flush_page_to_popa(base | nse);
}

__unused static void delegate_shatter(uint64_t base, gpi_info_t *gpi_info)
{
	unsigned int level = GPT_L1_CONT_CONTIG(gpi_info->gpt_l1_desc) - 1U;

	/* Shatter contiguous block */
	(void)gpt_shatter_lookup[level](base, gpi_info, GPT_L1_NS_DESC);
	dsboshst();

	gpt_tlbi_lookup[level].function(base & gpt_tlbi_lookup[level].mask);
	dsboshst();

	/* Set Non-secure Granules descriptor */
	gpi_info->gpt_l1_desc = GPT_L1_NS_DESC;
}

/*
 * Helper function to convert GPI entries in a single L1 table
 * from Granules to Contiguous descriptor.
 *
 * Parameters
 *   base		2MB aligned base address of the region to be written
 *   gpi_info		Pointer to 'gpt_config_t' structure
 *   l1_desc		GPT Contiguous descriptor set this range to
 */
__unused static void fuse_block(uint64_t base, gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	uint64_t l1_cont_desc;
	unsigned int idx_2 = GPT_L1_INDEX(base);
	unsigned int idx_off = gpi_info->idx - idx_2;
	unsigned int cnt = gpt_l1_cnt_2mb - 1U;
	__unused unsigned int idx_32, idx_512, j;

	/* Skip current L1 entry and check all others in 2MB */
	for (unsigned int i = 1U; i <= cnt; i++) {
		if (gpi_info->gpt_l1_addr[idx_2 + ((idx_off + i) & cnt)] !=
								l1_desc) {
			return;
		}
	}

	/* 2MB Contiguous descriptor */
	l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, GPT_L1_CONT_DESC_2MB);

#if (GPT_MAX_BLOCK == 2)
	(void)fill_desc(&gpi_info->gpt_l1_addr[idx_2], l1_cont_desc, QWORDS_2MB);
	tlbi_2mb(base);
#else
	/* Set 1st 2MB Contiguous descriptor */
	gpi_info->gpt_l1_addr[idx_2] = l1_cont_desc;

	idx_32 = GPT_L1_INDEX(ALIGN_32MB(base));
	j = idx_32;

	/* Check each 1st 2MB L1 entry in 32MB */
	for (unsigned int i = 0U; i < 16U; i++, j += gpt_l1_cnt_2mb) {
		if (gpi_info->gpt_l1_addr[j] != l1_cont_desc) {
			(void)fill_desc(&gpi_info->gpt_l1_addr[idx_2],
					l1_cont_desc, QWORDS_2MB);
			tlbi_2mb(base);
			return;
		}
	}

	/* 32MB Contiguous descriptor */
	l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, GPT_L1_CONT_DESC_32MB);

#if (GPT_MAX_BLOCK == 32)
	(void)fill_desc(&gpi_info->gpt_l1_addr[idx_32], l1_cont_desc, QWORDS_32MB);
	tlbi_32mb(ALIGN_32MB(base));
#else
	/* Set 1st 32MB Contiguous descriptor */
	gpi_info->gpt_l1_addr[idx_32] = l1_cont_desc;

	idx_512 = GPT_L1_INDEX(ALIGN_512MB(base));
	j = idx_512;

	/* Check each 1st 32MB L1 entry in 512MB */
	for (unsigned int i = 0U; i < 16U; i++, j += gpt_l1_cnt_2mb * 16U) {
		if (gpi_info->gpt_l1_addr[j] != l1_cont_desc) {
			(void)fill_desc(&gpi_info->gpt_l1_addr[idx_32],
					l1_cont_desc, QWORDS_32MB);
			tlbi_32mb(ALIGN_32MB(base));
			return;
		}
	}

	/* 512MB Contiguous descriptor */
	l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, GPT_L1_CONT_DESC_512MB);

	(void)fill_desc(&gpi_info->gpt_l1_addr[idx_512], l1_cont_desc, QWORDS_512MB);
	tlbi_512mb(ALIGN_512MB(base));

#endif	/* GPT_MAX_BLOCK == 32 */
#endif	/* GPT_MAX_BLOCK == 2 */
}

__unused static void undelegate_shatter(uint64_t base, gpi_info_t *gpi_info,
					uint64_t l1_desc, uint64_t nse)
{
	unsigned int level = GPT_L1_CONT_CONTIG(gpi_info->gpt_l1_desc) - 1U;

	/* Shatter contiguous block */
	(void)gpt_shatter_lookup[level](base, gpi_info, l1_desc);

	/* Set Granules descriptor */
	gpi_info->gpt_l1_desc = l1_desc;

	/*
	 * Remove access in order to maintain mutual distrust between
	 * Realm and Secure states.
	 */
	remove_access(base, gpi_info, nse);

	/* Clear existing GPI encoding and transition granule */
	write_gpt(&gpi_info->gpt_l1_desc, gpi_info->gpt_l1_addr,
			  gpi_info->gpi_shift, gpi_info->idx, GPT_GPI_NS);
	dsboshst();

	/* Ensure that all agents observe the new NS configuration */
	tlbi_page(base);
	dsbosh();

	gpt_tlbi_lookup[level].function(base & gpt_tlbi_lookup[level].mask);
	dsboshst();
}

/*
 * This function is the granule transition delegate service. When a granule
 * transition request occurs it is routed to this function to have the request,
 * if valid, fulfilled following A1.1.1 Delegate of RME supplement
 *
 * TODO: implement support for transitioning multiple granules at once.
 *
 * Parameters
 *   base		Base address of the region to transition, must be
 *			aligned to granule size.
 *   size		Size of region to transition, must be aligned to granule
 *			size.
 *   src_sec_state	Security state of the caller.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_delegate_pas(uint64_t base, size_t size, unsigned int src_sec_state)
{
	gpi_info_t gpi_info;
	uint64_t nse, __unused l1_desc;
	unsigned int target_pas;
	int res;

	/* Ensure that the tables have been set up before taking requests */
	assert(gpt_config.plat_gpt_l0_base != 0UL);

	/* Ensure that caches are enabled */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0UL);

	/* See if this is a single or a range of granule transition */
	if (size != GPT_PGS_ACTUAL_SIZE(gpt_config.p)) {
		return -EINVAL;
	}

	/* Check that base and size are valid */
	if ((ULONG_MAX - base) < size) {
		VERBOSE("GPT: Transition request address overflow!\n");
		VERBOSE("      Base=0x%"PRIx64"\n", base);
		VERBOSE("      Size=0x%lx\n", size);
		return -EINVAL;
	}

	/* Make sure base and size are valid */
	if (((base & (GPT_PGS_ACTUAL_SIZE(gpt_config.p) - 1UL)) != 0UL) ||
	    ((size & (GPT_PGS_ACTUAL_SIZE(gpt_config.p) - 1UL)) != 0UL) ||
	    (size == 0UL) ||
	    ((base + size) >= GPT_PPS_ACTUAL_SIZE(gpt_config.t))) {
		VERBOSE("GPT: Invalid granule transition address range!\n");
		VERBOSE("      Base=0x%"PRIx64"\n", base);
		VERBOSE("      Size=0x%lx\n", size);
		return -EINVAL;
	}

	/* Delegate request can only come from REALM or SECURE */
	if (src_sec_state == SMC_FROM_REALM) {
		target_pas = GPT_GPI_REALM;
		nse = (uint64_t)GPT_NSE_REALM << GPT_NSE_SHIFT;
		l1_desc = GPT_L1_REALM_DESC;
	} else {
		if (src_sec_state == SMC_FROM_SECURE) {
			target_pas = GPT_GPI_SECURE;
			nse = (uint64_t)GPT_NSE_SECURE << GPT_NSE_SHIFT;
			l1_desc = GPT_L1_SECURE_DESC;
		} else {
			VERBOSE("GPT: Invalid caller security state 0x%x\n",
							src_sec_state);
			return -EINVAL;
		}
	}

	res = get_gpi_params(base, &gpi_info);
	if (res != 0) {
		return res;
	}

	/*
	 * Access to each 512MB block in L1 tables is controlled by a lock to
	 * ensure that no more than one CPU is allowed to make changes at any
	 * given time.
	 */
	bit_lock(gpi_info.bitlock, gpi_info.mask);

	read_gpi(&gpi_info);

	/* Check that the current address is in NS state */
	if (gpi_info.gpi != GPT_GPI_NS) {
		VERBOSE("GPT: Only Granule in NS state can be delegated.\n");
		VERBOSE("      Caller: %u, Current GPI: %u\n", src_sec_state,
			gpi_info.gpi);
		bit_unlock(gpi_info.bitlock, gpi_info.mask);
		return -EPERM;
	}

	/*
	 * In order to maintain mutual distrust between Realm and Secure
	 * states, remove any data speculatively fetched into the target
	 * physical address space. Issue DC CIPAPA over address range.
	 */
	flush_page_to_popa(base | nse);

#if (GPT_MAX_BLOCK != 0)
	/* Check for Contiguous descriptor */
	if ((gpi_info.gpt_l1_desc & GPT_L1_TYPE_CONT_DESC_MASK) ==
					GPT_L1_TYPE_CONT_DESC) {
		delegate_shatter(base, &gpi_info);
	}
#endif
	write_gpt(&gpi_info.gpt_l1_desc, gpi_info.gpt_l1_addr,
		  gpi_info.gpi_shift, gpi_info.idx, target_pas);
	dsboshst();

	tlbi_page(base);
	dsbosh();

#if (GPT_MAX_BLOCK != 0)
	if (gpi_info.gpt_l1_desc == l1_desc) {
		/* Try to fuse 2MB block */
		fuse_block(ALIGN_2MB(base), &gpi_info, l1_desc);
	}
#endif
	nse = (uint64_t)GPT_NSE_NS << GPT_NSE_SHIFT;

	/* Ensure that the scrubbed data have made it past the PoPA */
	flush_page_to_popa(base | nse);

	/* Unlock access to 512MB block */
	bit_unlock(gpi_info.bitlock, gpi_info.mask);

	/*
	 * The isb() will be done as part of context
	 * synchronization when returning to lower EL.
	 */
	VERBOSE("GPT: Granule 0x%"PRIx64" GPI 0x%x->0x%x\n",
		base, gpi_info.gpi, target_pas);

	return 0;
}

/*
 * This function is the granule transition undelegate service. When a granule
 * transition request occurs it is routed to this function where the request is
 * validated then fulfilled if possible.
 *
 * TODO: implement support for transitioning multiple granules at once.
 *
 * Parameters
 *   base		Base address of the region to transition, must be
 *			aligned to granule size.
 *   size		Size of region to transition, must be aligned to granule
 *			size.
 *   src_sec_state	Security state of the caller.
 *
 * Return
 *    Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_undelegate_pas(uint64_t base, size_t size, unsigned int src_sec_state)
{
	gpi_info_t gpi_info;
	uint64_t nse, __unused l1_desc;
	int res;

	/* Ensure that the tables have been set up before taking requests */
	assert(gpt_config.plat_gpt_l0_base != 0UL);

	/* Ensure that MMU and caches are enabled */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0UL);

	/* See if this is a single or a range of granule transition */
	if (size != GPT_PGS_ACTUAL_SIZE(gpt_config.p)) {
		return -EINVAL;
	}

	/* Check that base and size are valid */
	if ((ULONG_MAX - base) < size) {
		VERBOSE("GPT: Transition request address overflow!\n");
		VERBOSE("      Base=0x%"PRIx64"\n", base);
		VERBOSE("      Size=0x%lx\n", size);
		return -EINVAL;
	}

	/* Make sure base and size are valid */
	if (((base & (GPT_PGS_ACTUAL_SIZE(gpt_config.p) - 1UL)) != 0UL) ||
	    ((size & (GPT_PGS_ACTUAL_SIZE(gpt_config.p) - 1UL)) != 0UL) ||
	    (size == 0UL) ||
	    ((base + size) >= GPT_PPS_ACTUAL_SIZE(gpt_config.t))) {
		VERBOSE("GPT: Invalid granule transition address range!\n");
		VERBOSE("      Base=0x%"PRIx64"\n", base);
		VERBOSE("      Size=0x%lx\n", size);
		return -EINVAL;
	}

	res = get_gpi_params(base, &gpi_info);
	if (res != 0) {
		bit_unlock(gpi_info.bitlock, gpi_info.mask);
		return res;
	}

	/*
	 * Access to each 512MB block in L1 tables is controlled by a lock to
	 * ensure that no more than one CPU is allowed to make changes at any
	 * given time.
	 */
	bit_lock(gpi_info.bitlock, gpi_info.mask);

	read_gpi(&gpi_info);

	/* Check that the current address is in the delegated state */
	if ((src_sec_state == SMC_FROM_REALM) &&
		(gpi_info.gpi == GPT_GPI_REALM)) {
		l1_desc = GPT_L1_REALM_DESC;
		nse = (uint64_t)GPT_NSE_REALM << GPT_NSE_SHIFT;
	} else if ((src_sec_state == SMC_FROM_SECURE) &&
		(gpi_info.gpi == GPT_GPI_SECURE)) {
		l1_desc = GPT_L1_SECURE_DESC;
		nse = (uint64_t)GPT_NSE_SECURE << GPT_NSE_SHIFT;
	} else {
		VERBOSE("GPT: Only Granule in REALM or SECURE state can be undelegated\n");
		VERBOSE("      Caller: %u Current GPI: %u\n", src_sec_state,
			gpi_info.gpi);
		bit_unlock(gpi_info.bitlock, gpi_info.mask);
		return -EPERM;
	}

#if (GPT_MAX_BLOCK != 0)
	/* Check for Contiguous descriptor */
	if ((gpi_info.gpt_l1_desc & GPT_L1_TYPE_CONT_DESC_MASK) ==
					GPT_L1_TYPE_CONT_DESC) {
		undelegate_shatter(base, &gpi_info, l1_desc, nse);
	} else {
#endif
		remove_access(base, &gpi_info, nse);

		/* Clear existing GPI encoding and transition granule */
		write_gpt(&gpi_info.gpt_l1_desc, gpi_info.gpt_l1_addr,
			  gpi_info.gpi_shift, gpi_info.idx, GPT_GPI_NS);
		dsboshst();

		/* Ensure that all agents observe the new NS configuration */
		tlbi_page(base);
		dsbosh();

#if (GPT_MAX_BLOCK != 0)
		if (gpi_info.gpt_l1_desc == GPT_L1_NS_DESC) {
			/* Try to fuse 2MB block */
			fuse_block(ALIGN_2MB(base), &gpi_info, GPT_L1_NS_DESC);
		}
	}
#endif
	/* Unlock access to 512MB block */
	bit_unlock(gpi_info.bitlock, gpi_info.mask);

	/*
	 * The isb() will be done as part of context
	 * synchronization when returning to lower EL.
	 */
	VERBOSE("GPT: Granule 0x%"PRIx64" GPI 0x%x->0x%x\n",
		base, gpi_info.gpi, GPT_GPI_NS);

	return 0;
}
