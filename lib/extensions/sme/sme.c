/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/extensions/sme.h>

#if CTX_INCLUDE_FPREGS
	#error CTX_INCLUDE_FPREGS is not supported on SME-enabled systems.
#endif

bool feat_sme_supported(void)
{
	uint64_t features;

	features = read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_SME_SHIFT;
	return (features & ID_AA64PFR1_EL1_SME_MASK) != 0U;
}

bool feat_sme_fa64_supported(void)
{
	uint64_t features;

	features = read_id_aa64smfr0_el1();
	return (features & ID_AA64SMFR0_EL1_FA64_BIT) != 0U;
}

void sme_enable_ns(cpu_context_t *context)
{
	u_register_t reg;
	u_register_t cptr_el3;
	el3_state_t *state;

	/* Make sure SME is implemented in hardware before continuing. */
	if (!feat_sme_supported()) {
		return;
	}

	state = get_el3state_ctx(context);

	/* Enable SME in NS CPTR_EL3. */
	reg = read_ctx_reg(state, CTX_CPTR_EL3);
	reg |= ESM_BIT;
	write_ctx_reg(state, CTX_CPTR_EL3, reg);

	/* Set the ENTP2 bit in NS SCR_EL3 to enable access to TPIDR2_EL0. */
	reg = read_ctx_reg(state, CTX_SCR_EL3);
	reg |= SCR_ENTP2_BIT;
	write_ctx_reg(state, CTX_SCR_EL3, reg);

	/* Set CPTR_EL3.ESM bit so we can write SMCR_EL3 without trapping. */
	cptr_el3 = read_cptr_el3();
	write_cptr_el3(cptr_el3 | ESM_BIT);

	/*
	 * Set the max LEN value and FA64 bit. Since we are only enabling SME for
	 * NS, this register does not need to be saved/restored on world changes
	 * so we set it globally. CPTR_EL3.ESM will ensure that unauthorized
	 * accesses are trapped.
	 */
	reg = SMCR_ELX_LEN_MASK;
	if (feat_sme_fa64_supported()) {
		VERBOSE("[SME] FA64 enabled\n");
		reg |= SMCR_ELX_FA64_BIT;
	}
	write_smcr_el3(reg);

	/* Reset CPTR_EL3 value. */
	write_cptr_el3(cptr_el3);
}
