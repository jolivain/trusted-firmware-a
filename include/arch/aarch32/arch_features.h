/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_FEATURES_H
#define ARCH_FEATURES_H

#include <stdbool.h>

#include <arch_helpers.h>

static inline bool is_armv7_gentimer_present(void)
{
	return ((read_id_pfr1() >> ID_PFR1_GENTIMER_SHIFT) &
		ID_PFR1_GENTIMER_MASK) != 0U;
}

static inline bool is_armv8_2_ttcnp_present(void)
{
	return ((read_id_mmfr4() >> ID_MMFR4_CNP_SHIFT) &
		ID_MMFR4_CNP_MASK) != 0U;
}

static inline unsigned int read_feat_coptrc_id_field(void)
{
	return (read_id_dfr0() >> ID_DFR0_COPTRC_SHIFT) & ID_DFR0_COPTRC_MASK;
}

static inline bool is_feat_sys_reg_trace_supported(void)
{
	if (ENABLE_SYS_REG_TRACE_FOR_NS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_SYS_REG_TRACE_FOR_NS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_coptrc_id_field() != 0U;
}

#endif /* ARCH_FEATURES_H */
