/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>

#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>

#include <plat/common/platform.h>

#include "../amu_private.h"

struct amu_ctx {
	uint64_t group0_enable;
	uint64_t group1_enable;

	uint64_t group0_cnts[AMU_GROUP0_MAX_COUNTERS];
	uint64_t group1_cnts[AMU_GROUP1_MAX_COUNTERS];

	/* Architected event counter 1 does not have an offset register */
	uint64_t group0_voffsets[AMU_GROUP0_MAX_COUNTERS - 1U];
	uint64_t group1_voffsets[AMU_GROUP1_MAX_COUNTERS];
};

static struct amu_ctx amu_ctxs[PLATFORM_CORE_COUNT];

static inline uint64_t read_id_aa64pfr0_el1_amu(void)
{
	return (read_id_aa64pfr0_el1() >> ID_AA64PFR0_AMU_SHIFT) &
		ID_AA64PFR0_AMU_MASK;
}

static inline uint64_t read_hcr_el2_amvoffen(void)
{
	return (read_hcr_el2() & HCR_AMVOFFEN_BIT) >>
		HCR_AMVOFFEN_SHIFT;
}

static inline void write_cptr_el2_tam(uint64_t value)
{
	write_cptr_el2((read_cptr_el2() & ~CPTR_EL2_TAM_BIT) |
		((value << CPTR_EL2_TAM_SHIFT) & CPTR_EL2_TAM_BIT));
}

static inline void write_cptr_el3_tam(cpu_context_t *ctx, uint64_t tam)
{
	uint64_t value = read_ctx_reg(get_el3state_ctx(ctx), CTX_CPTR_EL3);

	value &= ~TAM_BIT;
	value |= (tam << TAM_SHIFT) & TAM_BIT;

	write_ctx_reg(get_el3state_ctx(ctx), CTX_CPTR_EL3, value);
}

static inline void write_hcr_el2_amvoffen(uint64_t value)
{
	write_hcr_el2((read_hcr_el2() & ~HCR_AMVOFFEN_BIT) |
		((value << HCR_AMVOFFEN_SHIFT) & HCR_AMVOFFEN_BIT));
}

static inline void write_amcr_el0_cg1rz(uint64_t value)
{
	write_amcr_el0((read_amcr_el0() & ~AMCR_CG1RZ_BIT) |
		((value << AMCR_CG1RZ_SHIFT) & AMCR_CG1RZ_BIT));
}

static inline uint64_t read_amcfgr_el0_ncg(void)
{
	return (read_amcfgr_el0() >> AMCFGR_EL0_NCG_SHIFT) &
		AMCFGR_EL0_NCG_MASK;
}

static inline uint64_t read_amcgcr_el0_cg0nc(void)
{
	return (read_amcgcr_el0() >> AMCGCR_EL0_CG0NC_SHIFT) &
		AMCGCR_EL0_CG0NC_MASK;
}

static inline uint64_t read_amcg1idr_el0_voff(void)
{
	return (read_amcg1idr_el0() >> AMCG1IDR_VOFF_SHIFT) &
		AMCG1IDR_VOFF_MASK;
}

static inline uint64_t read_amcgcr_el0_cg1nc(void)
{
	return (read_amcgcr_el0() >> AMCGCR_EL0_CG1NC_SHIFT) &
		AMCGCR_EL0_CG1NC_MASK;
}

static inline uint64_t read_amcntenset0_el0_px(void)
{
	return (read_amcntenset0_el0() >> AMCNTENSET0_EL0_Pn_SHIFT) &
		AMCNTENSET0_EL0_Pn_MASK;
}

static inline uint64_t read_amcntenset1_el0_px(void)
{
	return (read_amcntenset1_el0() >> AMCNTENSET1_EL0_Pn_SHIFT) &
		AMCNTENSET1_EL0_Pn_MASK;
}

static inline void write_amcntenset0_el0_px(uint64_t px)
{
	uint64_t value = read_amcntenset0_el0();

	value &= ~AMCNTENSET0_EL0_Pn_MASK;
	value |= (px << AMCNTENSET0_EL0_Pn_SHIFT) & AMCNTENSET0_EL0_Pn_MASK;

	write_amcntenset0_el0(value);
}

static inline void write_amcntenset1_el0_px(uint64_t px)
{
	uint64_t value = read_amcntenset1_el0();

	value &= ~AMCNTENSET1_EL0_Pn_MASK;
	value |= (px << AMCNTENSET1_EL0_Pn_SHIFT) & AMCNTENSET1_EL0_Pn_MASK;

	write_amcntenset1_el0(value);
}

static inline void write_amcntenclr0_el0_px(uint64_t px)
{
	uint64_t value = read_amcntenclr0_el0();

	value &= ~AMCNTENCLR0_EL0_Pn_MASK;
	value |= (px << AMCNTENCLR0_EL0_Pn_SHIFT) & AMCNTENCLR0_EL0_Pn_MASK;

	write_amcntenclr0_el0(value);
}

static inline void write_amcntenclr1_el0_px(uint64_t px)
{
	uint64_t value = read_amcntenclr1_el0();

	value &= ~AMCNTENCLR1_EL0_Pn_MASK;
	value |= (px << AMCNTENCLR1_EL0_Pn_SHIFT) & AMCNTENCLR1_EL0_Pn_MASK;

	write_amcntenclr1_el0(value);
}

/*
 * Enable counters. This function is meant to be invoked by the context
 * management library before exiting from EL3.
 */
void amu_enable(bool el2_unused, cpu_context_t *ctx)
{
	const struct amu_fconf_topology *topology = NULL;
	unsigned int core_pos = plat_my_core_pos();

	uint64_t id_aa64pfr0_el1_amu; /* AMU version */

	uint64_t amcfgr_el0_ncg; /* Number of counter groups */
	uint64_t amcgcr_el0_cg0nc; /* Number of group 0 counters */

	uint64_t amcntenset0_el0_px = 0x0; /* Group 0 enable mask */
	uint64_t amcntenset1_el0_px = 0x0; /* Group 1 enable mask */

	id_aa64pfr0_el1_amu = read_id_aa64pfr0_el1_amu();
	if (id_aa64pfr0_el1_amu == ID_AA64PFR0_AMU_NOT_SUPPORTED) {
		/*
		 * If the AMU is unsupported, nothing needs to be done.
		 */

		return;
	}

	if (el2_unused) {
		/*
		 * CPTR_EL2.TAM: Set to zero so any accesses to the Activity
		 * Monitor registers do not trap to EL2.
		 */
		write_cptr_el2_tam(0U);
	}

	/*
	 * Retrieve and update the CPTR_EL3 value from the context mentioned
	 * in 'ctx'. Set CPTR_EL3.TAM to zero so that any accesses to
	 * the Activity Monitor registers do not trap to EL3.
	 */
	write_cptr_el3_tam(ctx, 0U);

	/*
	 * Retrieve the number of architected counters. All of these counters
	 * are enabled by default.
	 */

	amcgcr_el0_cg0nc = read_amcgcr_el0_cg0nc();
	amcntenset0_el0_px = (UINT64_C(1) << (amcgcr_el0_cg0nc)) - 1U;

	assert(amcgcr_el0_cg0nc <= AMU_AMCGCR_CG0NC_MAX);

	/*
	 * The platform may opt to enable specific auxiliary counters at boot
	 * through the hardware configuration device tree. These are enabled
	 * here.
	 */

	topology = amu_topology();
	if (topology != NULL) {
		amcntenset1_el0_px = topology->cores[core_pos].enable;
	}

	/*
	 * Enable the requested counters.
	 */

	write_amcntenset0_el0_px(amcntenset0_el0_px);

	amcfgr_el0_ncg = read_amcfgr_el0_ncg();
	if (amcfgr_el0_ncg > 0U) {
		write_amcntenset1_el0_px(amcntenset1_el0_px);
	}

	/* Initialize FEAT_AMUv1p1 features if present. */
	if (id_aa64pfr0_el1_amu < ID_AA64PFR0_AMU_V1P1) {
		return;
	}

	if (el2_unused) {
		/* Make sure virtual offsets are disabled if EL2 not used. */
		write_hcr_el2_amvoffen(0U);
	}

#if AMU_RESTRICT_COUNTERS
	/*
	 * FEAT_AMUv1p1 adds a register field to restrict access to group 1
	 * counters at all but the highest implemented EL.  This is controlled
	 * with the AMU_RESTRICT_COUNTERS compile time flag, when set, system
	 * register reads at lower ELs return zero.  Reads from the memory
	 * mapped view are unaffected.
	 */
	VERBOSE("AMU group 1 counter access restricted.\n");
	write_amcr_el0_cg1rz(1U);
#else
	write_amcr_el0_cg1rz(0U);
#endif
}

/* Read the group 0 counter identified by the given `idx`. */
static uint64_t amu_group0_cnt_read(unsigned int idx)
{
	assert(read_id_aa64pfr0_el1_amu() != ID_AA64PFR0_AMU_NOT_SUPPORTED);
	assert(idx < read_amcgcr_el0_cg0nc());

	return amu_group0_cnt_read_internal(idx);
}

/* Write the group 0 counter identified by the given `idx` with `val` */
static void amu_group0_cnt_write(unsigned  int idx, uint64_t val)
{
	assert(read_id_aa64pfr0_el1_amu() != ID_AA64PFR0_AMU_NOT_SUPPORTED);
	assert(idx < read_amcgcr_el0_cg0nc());

	amu_group0_cnt_write_internal(idx, val);
	isb();
}

/*
 * Read the group 0 offset register for a given index. Index must be 0, 2,
 * or 3, the register for 1 does not exist.
 *
 * Using this function requires FEAT_AMUv1p1 support.
 */
static uint64_t amu_group0_voffset_read(unsigned int idx)
{
	assert(read_id_aa64pfr0_el1_amu() >= ID_AA64PFR0_AMU_V1P1);
	assert(idx < read_amcgcr_el0_cg0nc());
	assert(idx != 1U);

	return amu_group0_voffset_read_internal(idx);
}

/*
 * Write the group 0 offset register for a given index. Index must be 0, 2, or
 * 3, the register for 1 does not exist.
 *
 * Using this function requires FEAT_AMUv1p1 support.
 */
static void amu_group0_voffset_write(unsigned int idx, uint64_t val)
{
	assert(read_id_aa64pfr0_el1_amu() >= ID_AA64PFR0_AMU_V1P1);
	assert(idx < read_amcgcr_el0_cg0nc());
	assert(idx != 1U);

	amu_group0_voffset_write_internal(idx, val);
	isb();
}

/* Read the group 1 counter identified by the given `idx` */
static uint64_t amu_group1_cnt_read(unsigned int idx)
{
	assert(read_id_aa64pfr0_el1_amu() != ID_AA64PFR0_AMU_NOT_SUPPORTED);
	assert(read_amcfgr_el0_ncg() > 0U);
	assert(idx < read_amcgcr_el0_cg1nc());

	return amu_group1_cnt_read_internal(idx);
}

/* Write the group 1 counter identified by the given `idx` with `val` */
static void amu_group1_cnt_write(unsigned int idx, uint64_t val)
{
	assert(read_id_aa64pfr0_el1_amu() != ID_AA64PFR0_AMU_NOT_SUPPORTED);
	assert(read_amcfgr_el0_ncg() > 0U);
	assert(idx < read_amcgcr_el0_cg1nc());

	amu_group1_cnt_write_internal(idx, val);
	isb();
}

/*
 * Read the group 1 offset register for a given index.
 *
 * Using this function requires FEAT_AMUv1p1 support.
 */
static uint64_t amu_group1_voffset_read(unsigned int idx)
{
	assert(read_id_aa64pfr0_el1_amu() >= ID_AA64PFR0_AMU_V1P1);
	assert(read_amcfgr_el0_ncg() > 0U);
	assert(idx < read_amcgcr_el0_cg1nc());
	assert((read_amcg1idr_el0_voff() & (UINT64_C(1) << idx)) != 0U);

	return amu_group1_voffset_read_internal(idx);
}

/*
 * Write the group 1 offset register for a given index.
 *
 * Using this function requires FEAT_AMUv1p1 support.
 */
static void amu_group1_voffset_write(unsigned int idx, uint64_t val)
{
	assert(read_id_aa64pfr0_el1_amu() >= ID_AA64PFR0_AMU_V1P1);
	assert(read_amcfgr_el0_ncg() > 0U);
	assert(idx < read_amcgcr_el0_cg1nc());
	assert((read_amcg1idr_el0_voff() & (UINT64_C(1) << idx)) != 0U);

	amu_group1_voffset_write_internal(idx, val);
	isb();
}

static void *amu_context_save(const void *arg)
{
	uint64_t i, j;

	unsigned int core_pos = plat_my_core_pos();
	struct amu_ctx *ctx = &amu_ctxs[core_pos];

	uint64_t id_aa64pfr0_el1_amu; /* AMU version */

	uint64_t hcr_el2_amvoffen; /* AMU virtual offsets enabled */
	uint64_t amcg1idr_el0_voff; /* Auxiliary counters with virtual offsets */

	uint64_t amcfgr_el0_ncg; /* Number of counter groups */
	uint64_t amcgcr_el0_cg0nc; /* Number of group 0 counters */
	uint64_t amcgcr_el0_cg1nc; /* Number of group 1 counters */

	id_aa64pfr0_el1_amu = read_id_aa64pfr0_el1_amu();
	if (id_aa64pfr0_el1_amu == ID_AA64PFR0_AMU_NOT_SUPPORTED) {
		return (void *)0U;
	}

	amcfgr_el0_ncg = read_amcfgr_el0_ncg();
	amcgcr_el0_cg0nc = read_amcgcr_el0_cg0nc();
	amcgcr_el0_cg1nc = (amcfgr_el0_ncg > 0U) ? read_amcgcr_el0_cg1nc() : 0U;

	hcr_el2_amvoffen = (id_aa64pfr0_el1_amu >= ID_AA64PFR0_AMU_V1P1) ?
		read_hcr_el2_amvoffen() : 0U;
	amcg1idr_el0_voff = (hcr_el2_amvoffen != 0U) ?
		read_amcg1idr_el0_voff() : 0U;

	/*
	 * Disable all AMU counters.
	 */

	ctx->group0_enable = read_amcntenset0_el0_px();
	write_amcntenclr0_el0_px(ctx->group0_enable);

	if (amcfgr_el0_ncg > 0U) {
		ctx->group1_enable = read_amcntenset1_el0_px();
		write_amcntenclr1_el0_px(ctx->group1_enable);
	}

	/*
	 * Save the counters to the local context.
	 */

	isb(); /* Ensure counters have been disabled */

	for (i = 0U; i < amcgcr_el0_cg0nc; i++) {
		ctx->group0_cnts[i] = amu_group0_cnt_read(i);
	}

	for (i = 0U; i < amcgcr_el0_cg1nc; i++) {
		ctx->group1_cnts[i] = amu_group1_cnt_read(i);
	}

	/*
	 * Save virtual offsets for counters that offer them.
	 */

	if (hcr_el2_amvoffen) {
		assert(amcgcr_el0_cg0nc < 4U); /* Will need explicit support */

		for (i = 0U, j = 0U; i < amcgcr_el0_cg0nc; i++) {
			if (i == 1U) { /* Not runtime-detectable */
				continue; /* No virtual offset */
			}

			ctx->group0_voffsets[j++] = amu_group0_voffset_read(i);
		}

		for (i = 0U, j = 0U; i < amcgcr_el0_cg1nc; i++) {
			if ((amcg1idr_el0_voff >> i) & 1U) {
				continue; /* No virtual offset */
			}

			ctx->group1_voffsets[j++] = amu_group1_voffset_read(i);
		}
	}

	return (void *)0;
}

static void *amu_context_restore(const void *arg)
{
	uint64_t i, j;

	unsigned int core_pos = plat_my_core_pos();
	struct amu_ctx *ctx = &amu_ctxs[core_pos];

	uint64_t id_aa64pfr0_el1_amu; /* AMU version */

	uint64_t hcr_el2_amvoffen; /* AMU virtual offsets enabled */
	uint64_t amcg1idr_el0_voff; /* Auxiliary counters with virtual offsets */

	uint64_t amcfgr_el0_ncg; /* Number of counter groups */
	uint64_t amcgcr_el0_cg0nc; /* Number of group 0 counters */
	uint64_t amcgcr_el0_cg1nc; /* Number of group 1 counters */

	id_aa64pfr0_el1_amu = read_id_aa64pfr0_el1_amu();
	if (id_aa64pfr0_el1_amu == ID_AA64PFR0_AMU_NOT_SUPPORTED) {
		return (void *)0U;
	}

	amcfgr_el0_ncg = read_amcfgr_el0_ncg();
	amcgcr_el0_cg0nc = read_amcgcr_el0_cg0nc();
	amcgcr_el0_cg1nc = (amcfgr_el0_ncg > 0U) ? read_amcgcr_el0_cg1nc() : 0U;

	hcr_el2_amvoffen = (id_aa64pfr0_el1_amu >= ID_AA64PFR0_AMU_V1P1) ?
		read_hcr_el2_amvoffen() : 0U;
	amcg1idr_el0_voff = (hcr_el2_amvoffen != 0U) ?
		read_amcg1idr_el0_voff() : 0U;

	/*
	 * Sanity check that all counters were disabled when the context was
	 * previously saved.
	 */

	assert(read_amcntenset0_el0_px() == 0U);

	if (amcfgr_el0_ncg > 0U) {
		assert(read_amcntenset1_el0_px() == 0U);
	}

	/*
	 * Restore the counter values from the local context.
	 */

	for (i = 0U; i < amcgcr_el0_cg0nc; i++) {
		amu_group0_cnt_write(i, ctx->group0_cnts[i]);
	}

	for (i = 0U; i < amcgcr_el0_cg1nc; i++) {
		amu_group1_cnt_write(i, ctx->group1_cnts[i]);
	}

	/*
	 * Restore virtual offsets for counters that offer them.
	 */

	if (hcr_el2_amvoffen != 0U) {
		assert(amcgcr_el0_cg0nc < 4U); /* Will need explicit support */

		for (i = 0U, j = 0U; i < amcgcr_el0_cg0nc; i++) {
			if (i == 1U) { /* Not runtime-detectable */
				continue; /* No virtual offset */
			}

			amu_group0_voffset_write(i, ctx->group0_voffsets[j++]);
		}

		for (i = 0U, j = 0U; i < amcgcr_el0_cg1nc; i++) {
			if ((amcg1idr_el0_voff >> i) & 1U) {
				continue; /* No virtual offset */
			}

			amu_group1_voffset_write(i, ctx->group1_voffsets[j++]);
		}
	}

	/*
	 * Re-enable counters that were disabled during context save.
	 */

	write_amcntenset0_el0_px(ctx->group0_enable);

	if (amcfgr_el0_ncg > 0) {
		write_amcntenset1_el0_px(ctx->group1_enable);
	}

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
