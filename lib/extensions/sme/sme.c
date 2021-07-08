/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/extensions/sme.h>

#if CTX_INCLUDE_FPREGS
	#error CTX_INCLUDE_FPREGS is not supported on SME-enabled systems.
#endif

bool feat_sme_supported(void)
{
	uint64_t features;

	features = read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_SME_SHIFT;
	return (features & ID_AA64PFR1_EL1_SME_MASK) == SME_IMPLEMENTED;
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
	unsigned int current_vector_len;
	unsigned int requested_vector_len;
	unsigned int i;

	/* Make sure SME is implemented in hardware before continuing. */
	if (!feat_sme_supported()) {
		return;
	}

	/* Enable SME in NS context in CPTR_EL3. */
	reg = read_ctx_reg(get_el3state_ctx(context), CTX_CPTR_EL3);
	reg |= ESM_BIT;
	write_ctx_reg(get_el3state_ctx(context), CTX_CPTR_EL3, reg);

	/* Set the ENTP2 bit in SCR_EL3 for NS context. */
	reg = read_ctx_reg(get_el3state_ctx(context), CTX_SCR_EL3);
	reg |= SCR_ENTP2_BIT;
	write_ctx_reg(get_el3state_ctx(context), CTX_SCR_EL3, reg);

	/*
	 * Enable SME for the current context by writing directly to CPTR_EL3
	 * and SCR_EL3.
	 */
	write_cptr_el3(read_cptr_el3() | ESM_BIT);
	write_scr_el3(read_scr_el3() | SCR_ENTP2_BIT);

	/*
	 * Search for the largest supported LEN, we can't do this until ESM
	 * bit is set in CPTR_EL3. Start by setting SM bit to enter streaming
	 * mode. (changes behavior of SVE RDVL to return SME vector length)
	 */
	write_svcr(read_svcr() | SVCR_SM_BIT);

	/* Find out width of SMCR_ELx.LEN */
	write_smcr_el3(SME_SMCR_LEN_MAX);
	i = read_smcr_el3();

	/* Loop until we find the max supported vector length. */
	while(true) {
		/* Load new value into SMCR_EL3.LEN */
		write_smcr_el3(i);

		/* Compute current and requested vector lengths in bits. */
		current_vector_len = (sme_rdvl_1() * 8U);
		requested_vector_len = (i + 1U) * 128U;

		VERBOSE("[SME] Trying SMCR_EL3.LEN=%u...\n", i);

		/*
		 * We count down from the maximum LEN value, so if the values
		 * match, we've found the largest supported value for LEN.
		 */
		if (current_vector_len == requested_vector_len) {
			VERBOSE("[SME] Max vector size %u bits\n",
				current_vector_len);
			break;
		}

		if (i == 0U) {
			/* Execution should never reach this point. */
			panic();
		}

		i--;
	}

	/* Clear SM to exit streaming mode. */
	write_svcr(read_svcr() & ~SVCR_SM_BIT);

	/*
	 * Disable SME for this context by writing directly to CPTR_EL3 and
	 * SCR_EL3.
	 */
	write_cptr_el3(read_cptr_el3() & ~ESM_BIT);
	write_scr_el3(read_scr_el3() & ~SCR_ENTP2_BIT);

	/* Set the SMCR_EL3 LEN value and FA64 bit for NS context. */
	reg = i & SMCR_ELX_LEN_MASK;
	if (feat_sme_fa64_supported()) {
		VERBOSE("[SME] FA64 enabled\n");
		reg |= SMCR_ELX_FA64_BIT;
	}
	write_ctx_reg(get_el3state_ctx(context), CTX_SMCR_EL3, reg);
}
