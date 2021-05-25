/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>

#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>
#include <lib/extensions/amu_private.h>

#include <plat/common/platform.h>

static struct amu_ctx amu_ctxs[PLATFORM_CORE_COUNT];

static inline uint32_t read_id_pfr0_amu(void)
{
	return (read_id_pfr0() >> ID_PFR0_AMU_SHIFT) &
		ID_PFR0_AMU_MASK;
}

static inline void write_hcptr_tam(uint32_t value)
{
	write_hcptr((read_hcptr() & ~TAM_BIT) |
		((value << TAM_SHIFT) & TAM_BIT));
}

static inline void write_amcr_cg1rz(uint32_t value)
{
	write_amcr((read_amcr() & ~AMCR_CG1RZ_BIT) |
		((value << AMCR_CG1RZ_SHIFT) & AMCR_CG1RZ_BIT));
}

static inline uint32_t read_amcfgr_ncg(void)
{
	return (read_amcfgr() >> AMCFGR_NCG_SHIFT) &
		AMCFGR_NCG_MASK;
}

static inline uint32_t read_amcgcr_cg1nc(void)
{
	return (read_amcgcr() >> AMCGCR_CG1NC_SHIFT) &
		AMCGCR_CG1NC_MASK;
}

#if ENABLE_ASSERTIONS
static inline uint32_t read_amcntenset0_px(void)
{
	return (read_amcntenset0() >> AMCNTENSET0_Pn_SHIFT) &
		AMCNTENSET0_Pn_MASK;
}

static inline uint32_t read_amcntenset1_px(void)
{
	return (read_amcntenset1() >> AMCNTENSET1_Pn_SHIFT) &
		AMCNTENSET1_Pn_MASK;
}
#endif

static inline void write_amcntenset0_px(uint32_t px)
{
	uint32_t value = read_amcntenset0();

	value &= ~AMCNTENSET0_Pn_MASK;
	value |= (px << AMCNTENSET0_Pn_SHIFT) &
		AMCNTENSET0_Pn_MASK;

	write_amcntenset0(value);
}

static inline void write_amcntenset1_px(uint32_t px)
{
	uint32_t value = read_amcntenset1();

	value &= ~AMCNTENSET1_Pn_MASK;
	value |= (px << AMCNTENSET1_Pn_SHIFT) &
		AMCNTENSET1_Pn_MASK;

	write_amcntenset1(value);
}

static inline void write_amcntenclr0_px(uint32_t px)
{
	uint32_t value = read_amcntenclr0();

	value &= ~AMCNTENCLR0_Pn_MASK;
	value |= (px << AMCNTENCLR0_Pn_SHIFT) & AMCNTENCLR0_Pn_MASK;

	write_amcntenclr0(value);
}

static inline void write_amcntenclr1_px(uint32_t px)
{
	uint32_t value = read_amcntenclr1();

	value &= ~AMCNTENCLR1_Pn_MASK;
	value |= (px << AMCNTENCLR1_Pn_SHIFT) & AMCNTENCLR1_Pn_MASK;

	write_amcntenclr1(value);
}

/*
 * Enable counters. This function is meant to be invoked
 * by the context management library before exiting from EL3.
 */
void amu_enable(bool el2_unused)
{
	if (read_id_pfr0_amu() == ID_PFR0_AMU_NOT_SUPPORTED) {
		return;
	}

	if (AMU_GROUP1_NR_COUNTERS > 0U) {
		/* Check and set presence of group 1 counters */
		if (read_amcfgr_ncg() < 1) {
			ERROR("AMU Counter Group 1 is not implemented\n");
			panic();
		}

		/* Check number of group 1 counters */
		uint32_t cnt_num = read_amcgcr_cg1nc();

		VERBOSE("%s%u. %s%u\n",
			"Number of AMU Group 1 Counters ", cnt_num,
			"Requested number ", AMU_GROUP1_NR_COUNTERS);

		if (cnt_num < AMU_GROUP1_NR_COUNTERS) {
			ERROR("%s%u is less than %s%u\n",
			"Number of AMU Group 1 Counters ", cnt_num,
			"Requested number ", AMU_GROUP1_NR_COUNTERS);
			panic();
		}
	}

	if (el2_unused) {
		/*
		 * Non-secure access from EL0 or EL1 to the Activity Monitor
		 * registers do not trap to EL2.
		 */
		write_hcptr_tam(0);
	}

	/* Enable group 0 counters */
	write_amcntenset0_px(AMU_GROUP0_COUNTERS_MASK);

	if (AMU_GROUP1_NR_COUNTERS > 0U) {
		/* Enable group 1 counters */
		write_amcntenset1_px(AMU_GROUP1_COUNTERS_MASK);
	}

	/* Initialize FEAT_AMUv1p1 features if present. */
	if (read_id_pfr0_amu() < ID_PFR0_AMU_V1P1) {
		return;
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
	write_amcr_cg1rz(1);
#else
	write_amcr_cg1rz(0);
#endif
}

/* Read the group 0 counter identified by the given `idx`. */
uint64_t amu_group0_cnt_read(unsigned int idx)
{
	assert(read_id_pfr0_amu() != ID_PFR0_AMU_NOT_SUPPORTED);
	assert(idx < AMU_GROUP0_NR_COUNTERS);

	return amu_group0_cnt_read_internal(idx);
}

/* Write the group 0 counter identified by the given `idx` with `val` */
void amu_group0_cnt_write(unsigned  int idx, uint64_t val)
{
	assert(read_id_pfr0_amu() != ID_PFR0_AMU_NOT_SUPPORTED);
	assert(idx < AMU_GROUP0_NR_COUNTERS);

	amu_group0_cnt_write_internal(idx, val);
	isb();
}

/* Read the group 1 counter identified by the given `idx` */
uint64_t amu_group1_cnt_read(unsigned  int idx)
{
	assert(read_id_pfr0_amu() != ID_PFR0_AMU_NOT_SUPPORTED);
	assert(read_amcfgr_ncg() > 0);
	assert(idx < AMU_GROUP1_NR_COUNTERS);

	return amu_group1_cnt_read_internal(idx);
}

/* Write the group 1 counter identified by the given `idx` with `val` */
void amu_group1_cnt_write(unsigned  int idx, uint64_t val)
{
	assert(read_id_pfr0_amu() != ID_PFR0_AMU_NOT_SUPPORTED);
	assert(read_amcfgr_ncg() > 0);
	assert(idx < AMU_GROUP1_NR_COUNTERS);

	amu_group1_cnt_write_internal(idx, val);
	isb();
}

/*
 * Program the event type register for the given `idx` with
 * the event number `val`
 */
void amu_group1_set_evtype(unsigned int idx, unsigned int val)
{
	assert(read_id_pfr0_amu() != ID_PFR0_AMU_NOT_SUPPORTED);
	assert(read_amcfgr_ncg() > 0);
	assert(idx < AMU_GROUP1_NR_COUNTERS);

	amu_group1_set_evtype_internal(idx, val);
	isb();
}

static void *amu_context_save(const void *arg)
{
	struct amu_ctx *ctx = &amu_ctxs[plat_my_core_pos()];
	unsigned int i;

	if (read_id_pfr0_amu() == ID_PFR0_AMU_NOT_SUPPORTED) {
		return (void *)-1;
	}

	if (AMU_GROUP1_NR_COUNTERS > 0U) {
		if (read_amcfgr_ncg() < 1) {
			return (void *)-1;
		}
	}
	/* Assert that group 0/1 counter configuration is what we expect */
	assert(read_amcntenset0_px() == AMU_GROUP0_COUNTERS_MASK);

	if (AMU_GROUP1_NR_COUNTERS > 0U) {
		assert(read_amcntenset1_px() == AMU_GROUP1_COUNTERS_MASK);
	}
	/*
	 * Disable group 0/1 counters to avoid other observers like SCP sampling
	 * counter values from the future via the memory mapped view.
	 */
	write_amcntenclr0_px(AMU_GROUP0_COUNTERS_MASK);

	if (AMU_GROUP1_NR_COUNTERS > 0U) {
		write_amcntenclr1_px(AMU_GROUP1_COUNTERS_MASK);
	}
	isb();

	/* Save all group 0 counters */
	for (i = 0U; i < AMU_GROUP0_NR_COUNTERS; i++) {
		ctx->group0_cnts[i] = amu_group0_cnt_read(i);
	}

	if (AMU_GROUP1_NR_COUNTERS > 0U) {
		/* Save group 1 counters */
		for (i = 0U; i < AMU_GROUP1_NR_COUNTERS; i++) {
			if ((AMU_GROUP1_COUNTERS_MASK & (1U << i)) != 0U) {
				ctx->group1_cnts[i] = amu_group1_cnt_read(i);
			}
		}
	}
	return (void *)0;
}

static void *amu_context_restore(const void *arg)
{
	struct amu_ctx *ctx = &amu_ctxs[plat_my_core_pos()];
	unsigned int i;

	if (read_id_pfr0_amu() == ID_PFR0_AMU_NOT_SUPPORTED) {
		return (void *)-1;
	}

	if (AMU_GROUP1_NR_COUNTERS > 0U) {
		if (read_amcfgr_ncg() < 1) {
			return (void *)-1;
		}
	}
	/* Counters were disabled in `amu_context_save()` */
	assert(read_amcntenset0_px() == 0U);

	if (AMU_GROUP1_NR_COUNTERS > 0U) {
		assert(read_amcntenset1_px() == 0U);
	}

	/* Restore all group 0 counters */
	for (i = 0U; i < AMU_GROUP0_NR_COUNTERS; i++) {
		amu_group0_cnt_write(i, ctx->group0_cnts[i]);
	}

	/* Restore group 0 counter configuration */
	write_amcntenset0_px(AMU_GROUP0_COUNTERS_MASK);

	if (AMU_GROUP1_NR_COUNTERS > 0U) {
		/* Restore group 1 counters */
		for (i = 0U; i < AMU_GROUP1_NR_COUNTERS; i++) {
			if ((AMU_GROUP1_COUNTERS_MASK & (1U << i)) != 0U) {
				amu_group1_cnt_write(i, ctx->group1_cnts[i]);
			}
		}

		/* Restore group 1 counter configuration */
		write_amcntenset1_px(AMU_GROUP1_COUNTERS_MASK);
	}

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
