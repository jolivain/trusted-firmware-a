/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/smccc.h>
#include <lib/debug_reg_smc.h>
#include <smccc_helpers.h>

enum debug_reg_id
{
	actlr_el1,
	actlr_el2,
	actlr_el3,
	scr_el3,
	hcr_el2,
	vbar_el1,
	vbar_el2,
	vbar_el3,
	sctlr_el1,
	sctlr_el2,
	sctlr_el3,
	mair_el1,
	mair_el2,
	mair_el3,
	amair_el1,
	amair_el2,
	amair_el3,
	rvbar_el1,
	rvbar_el2,
	rvbar_el3,
	midr_el1,
	mpidr_el1,
	id_aa64mmfr0_el1,
	id_aa64mmfr1_el1,
	id_aa64smfr0_el1,
	id_aa64mmfr2_el1,
	id_aa64mmfr3_el1,
	tcr_el1,
	tcr_el2,
	tcr_el3,
	ttbr0_el1,
	ttbr0_el2,
	ttbr0_el3,
	ttbr1_el1,
	ttbr1_el2,
	vttbr_el2,
	cptr_el2,
	cptr_el3,
	cpacr_el1,
	vtcr_el2,
	vpidr_el2,
	vmpidr_el2,
	hacr_el2,
	hstr_el2,
	mdscr_el1,
	mdcr_el2,
	mdcr_el3,
	pmcr_el0,
	csselr_el1,
	dacr32_el2,
	zcr_el3,
	zcr_el2,
	smcr_el3,
	mpamidr_el1,
	mpam3_el3,
	mpam2_el2,
	mpamhcr_el2,
	tfsre0_el1,
	tfsr_el1,
	rgsr_el1,
	gcr_el1,
	tfsr_el2,
	rndr,
	rndrrs,
	tcr2_el1,
	tcr2_el2,
	cpuppmcr_el3,
	cpumpmmcr_el3,
	gptbr_el3,
	gpccr_el3,
	DEBUG_REG_MAX
};

#define CASE_READ_REG(_reg, out) \
	case _reg:                   \
		*out = read_##_reg();    \
		break

static int32_t debug_reg_smc_op_info(u_register_t *info)
{
	union debug_reg_smc_info information = {
		.major_version = DEBUG_REG_SMC_MAJOR_VERSION,
		.minor_version = DEBUG_REG_SMC_MINOR_VERSION,
		.num_registers = DEBUG_REG_MAX
	};
	*info = (u_register_t) information.raw;

	return SMC_ARCH_CALL_SUCCESS;
}

static int32_t debug_reg_smc_op_read(u_register_t reg_id, u_register_t *value)
{
	/*
	   Cases where registers are not readable commented out but
	   kept in to keep track
	*/
	switch (reg_id)
	{
		CASE_READ_REG(actlr_el1, value);
		CASE_READ_REG(actlr_el2, value);
		CASE_READ_REG(actlr_el3, value);
		CASE_READ_REG(scr_el3, value);
		CASE_READ_REG(hcr_el2, value);
		CASE_READ_REG(vbar_el1, value);
		CASE_READ_REG(vbar_el2, value);
		CASE_READ_REG(vbar_el3, value);
		CASE_READ_REG(sctlr_el1, value);
		CASE_READ_REG(sctlr_el2, value);
		CASE_READ_REG(sctlr_el3, value);
		CASE_READ_REG(mair_el1, value);
		CASE_READ_REG(mair_el2, value);
		CASE_READ_REG(mair_el3, value);
		CASE_READ_REG(amair_el1, value);
		CASE_READ_REG(amair_el2, value);
		CASE_READ_REG(amair_el3, value);
		CASE_READ_REG(rvbar_el1, value);
		CASE_READ_REG(rvbar_el2, value);
		CASE_READ_REG(rvbar_el3, value);
		CASE_READ_REG(midr_el1, value);
		CASE_READ_REG(mpidr_el1, value);
		CASE_READ_REG(id_aa64mmfr0_el1, value);
		CASE_READ_REG(id_aa64mmfr1_el1, value);
		CASE_READ_REG(id_aa64smfr0_el1, value);
		CASE_READ_REG(id_aa64mmfr2_el1, value);
		CASE_READ_REG(id_aa64mmfr3_el1, value);
		CASE_READ_REG(tcr_el1, value);
		CASE_READ_REG(tcr_el2, value);
		CASE_READ_REG(tcr_el3, value);
		CASE_READ_REG(ttbr0_el1, value);
		CASE_READ_REG(ttbr0_el2, value);
		CASE_READ_REG(ttbr0_el3, value);
		CASE_READ_REG(ttbr1_el1, value);
		CASE_READ_REG(ttbr1_el2, value);
		CASE_READ_REG(vttbr_el2, value);
		CASE_READ_REG(cptr_el2, value);
		CASE_READ_REG(cptr_el3, value);
		CASE_READ_REG(cpacr_el1, value);
		CASE_READ_REG(vtcr_el2, value);
		CASE_READ_REG(vpidr_el2, value);
		CASE_READ_REG(vmpidr_el2, value);
		CASE_READ_REG(hacr_el2, value);
		CASE_READ_REG(hstr_el2, value);
		CASE_READ_REG(mdscr_el1, value);
		CASE_READ_REG(mdcr_el2, value);
		CASE_READ_REG(mdcr_el3, value);
		CASE_READ_REG(pmcr_el0, value);
		CASE_READ_REG(csselr_el1, value);
		CASE_READ_REG(dacr32_el2, value);
		// CASE_READ_REG(zcr_el3, value);
		// CASE_READ_REG(zcr_el2, value);
		CASE_READ_REG(smcr_el3, value);
		CASE_READ_REG(mpamidr_el1, value);
		CASE_READ_REG(mpam3_el3, value);
		CASE_READ_REG(mpam2_el2, value);
		CASE_READ_REG(mpamhcr_el2, value);
		CASE_READ_REG(tfsre0_el1, value);
		CASE_READ_REG(tfsr_el1, value);
		CASE_READ_REG(rgsr_el1, value);
		CASE_READ_REG(gcr_el1, value);
		CASE_READ_REG(tfsr_el2, value);
		CASE_READ_REG(rndr, value);
		CASE_READ_REG(rndrrs, value);
		CASE_READ_REG(tcr2_el1, value);
		CASE_READ_REG(tcr2_el2, value);
		CASE_READ_REG(cpuppmcr_el3, value);
		CASE_READ_REG(cpumpmmcr_el3, value);
		CASE_READ_REG(gptbr_el3, value);
		CASE_READ_REG(gpccr_el3, value);
	default:
		return SMC_ARCH_CALL_INVAL_PARAM;
	}

	NOTICE("Reg read %lu: 0x%lx\n", reg_id, *value);

	return SMC_ARCH_CALL_SUCCESS;
}
#define CASE_WRITE_REG(_reg, in) \
	case _reg:                   \
		write_##_reg(in);        \
		break

static int32_t debug_reg_smc_op_write(u_register_t reg_id, u_register_t value)
{

	/*
	   Cases where registers are not writeable commented out but
	   kept in to keep track
	*/
	switch (reg_id)
	{
		CASE_WRITE_REG(actlr_el1, value);
		CASE_WRITE_REG(actlr_el2, value);
		CASE_WRITE_REG(actlr_el3, value);
		CASE_WRITE_REG(scr_el3, value);
		CASE_WRITE_REG(hcr_el2, value);
		CASE_WRITE_REG(vbar_el1, value);
		CASE_WRITE_REG(vbar_el2, value);
		CASE_WRITE_REG(vbar_el3, value);
		CASE_WRITE_REG(sctlr_el1, value);
		CASE_WRITE_REG(sctlr_el2, value);
		CASE_WRITE_REG(sctlr_el3, value);
		CASE_WRITE_REG(mair_el1, value);
		CASE_WRITE_REG(mair_el2, value);
		CASE_WRITE_REG(mair_el3, value);
		CASE_WRITE_REG(amair_el1, value);
		CASE_WRITE_REG(amair_el2, value);
		CASE_WRITE_REG(amair_el3, value);
		// CASE_WRITE_REG(rvbar_el1, value);
		// CASE_WRITE_REG(rvbar_el2, value);
		// CASE_WRITE_REG(rvbar_el3, value);
		// CASE_WRITE_REG(midr_el1, value);
		// CASE_WRITE_REG(mpidr_el1, value);
		// CASE_WRITE_REG(id_aa64mmfr0_el1, value);
		// CASE_WRITE_REG(id_aa64mmfr1_el1, value);
		// CASE_WRITE_REG(id_aa64smfr0_el1, value);
		// CASE_WRITE_REG(id_aa64mmfr2_el1, value);
		// CASE_WRITE_REG(id_aa64mmfr3_el1, value);
		CASE_WRITE_REG(tcr_el1, value);
		CASE_WRITE_REG(tcr_el2, value);
		CASE_WRITE_REG(tcr_el3, value);
		CASE_WRITE_REG(ttbr0_el1, value);
		CASE_WRITE_REG(ttbr0_el2, value);
		CASE_WRITE_REG(ttbr0_el3, value);
		CASE_WRITE_REG(ttbr1_el1, value);
		CASE_WRITE_REG(ttbr1_el2, value);
		CASE_WRITE_REG(vttbr_el2, value);
		CASE_WRITE_REG(cptr_el2, value);
		CASE_WRITE_REG(cptr_el3, value);
		CASE_WRITE_REG(cpacr_el1, value);
		CASE_WRITE_REG(vtcr_el2, value);
		CASE_WRITE_REG(vpidr_el2, value);
		CASE_WRITE_REG(vmpidr_el2, value);
		CASE_WRITE_REG(hacr_el2, value);
		CASE_WRITE_REG(hstr_el2, value);
		CASE_WRITE_REG(mdscr_el1, value);
		CASE_WRITE_REG(mdcr_el2, value);
		CASE_WRITE_REG(mdcr_el3, value);
		CASE_WRITE_REG(pmcr_el0, value);
		CASE_WRITE_REG(csselr_el1, value);
		CASE_WRITE_REG(dacr32_el2, value);
		CASE_WRITE_REG(zcr_el3, value);
		CASE_WRITE_REG(zcr_el2, value);
		CASE_WRITE_REG(smcr_el3, value);
		// CASE_WRITE_REG(mpamidr_el1, value);
		CASE_WRITE_REG(mpam3_el3, value);
		CASE_WRITE_REG(mpam2_el2, value);
		CASE_WRITE_REG(mpamhcr_el2, value);
		CASE_WRITE_REG(tfsre0_el1, value);
		CASE_WRITE_REG(tfsr_el1, value);
		CASE_WRITE_REG(rgsr_el1, value);
		CASE_WRITE_REG(gcr_el1, value);
		CASE_WRITE_REG(tfsr_el2, value);
		// CASE_WRITE_REG(rndr, value);
		// CASE_WRITE_REG(rndrrs, value);
		CASE_WRITE_REG(tcr2_el1, value);
		CASE_WRITE_REG(tcr2_el2, value);
		CASE_WRITE_REG(cpuppmcr_el3, value);
		CASE_WRITE_REG(cpumpmmcr_el3, value);
		CASE_WRITE_REG(gptbr_el3, value);
		CASE_WRITE_REG(gpccr_el3, value);
	default:
		return SMC_ARCH_CALL_INVAL_PARAM;
	}

	NOTICE("Reg write %lu: 0x%lx\n", reg_id, value);

	return SMC_ARCH_CALL_SUCCESS;
}

uintptr_t debug_reg_smc_handler(unsigned int smc_fid,
								u_register_t x1,
								u_register_t x2,
								u_register_t x3,
								u_register_t x4,
								void *cookie,
								void *handle,
								u_register_t flags)
{
	switch (GET_SMC_NUM(smc_fid) & DEBUG_REG_SMC_OP_MASK)
	{
	case DEBUG_REG_SMC_OP_INFO:
		debug_reg_smc_op_info(&x1);
		SMC_RET1(handle, x1);
	case DEBUG_REG_SMC_OP_READ:
		if (SMC_ARCH_CALL_INVAL_PARAM == debug_reg_smc_op_read(x1, &x2))
		{
			SMC_RET1(handle, SMC_UNK);
		}
		SMC_RET2(handle, SMC_OK, x2);
	case DEBUG_REG_SMC_OP_WRITE:
		SMC_RET1(handle, debug_reg_smc_op_write(x1, x2));
	}

	SMC_RET1(handle, SMC_UNK);
}
