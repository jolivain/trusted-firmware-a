/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* C lib */
#include <assert.h>

/* TF-A header */
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/ep_info.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/smccc.h>

/* MTK header */
#include <plat/common/platform.h>

/* GZ header */
#include "gz_smc_to_el2.h"

#define EL2_ENTRY_VA 0x38003000ULL
#define EL2_SMC_DBG_ENABLE (0)
#define EL2_CONTEXT_DEBUG_EN (0)
#if EL2_SMC_DBG_ENABLE
#define EL2_SMC_DBG(fmt, ...) NOTICE(fmt, ##__VA_ARGS__)
#else
#define EL2_SMC_DBG(fmt, ...)                                                  \
	do {                                                                   \
	} while (0)
#endif
struct plat_cpu_context {
	struct cpu_context _cpu_context;
	uint64_t c_rt_ctx;
	uint64_t saved_sp_el2;
	uint64_t el2_ret_x0;
	uint64_t el2_ret_x1;
	uint64_t el2_ret_x2;
	uint64_t el2_ret_x3;
	uint64_t el2_ret_x4;
};
static struct plat_cpu_context
	__aligned(CACHE_WRITEBACK_GRANULE) plat_context[PLATFORM_CORE_COUNT];
static uint64_t el2_enter_sp(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			     u_register_t x3, u_register_t x4,
			     uintptr_t el2_entry)
{
	uint64_t rc;
	uint32_t cpu_id = plat_core_pos_by_mpidr(read_mpidr());
	cpu_context_t *ns_context = (cpu_context_t *)cm_get_context(NON_SECURE);
	struct plat_cpu_context *backup_context = &plat_context[cpu_id];
	gp_regs_t *ns_gpregs = get_gpregs_ctx(ns_context);
	el3_state_t *state = get_el3state_ctx(ns_context);

	assert(backup_context->c_rt_ctx == 0);

	/* Backup original REE context */
	memcpy(&backup_context->_cpu_context, ns_context,
	       sizeof(cpu_context_t));
	backup_context->saved_sp_el2 = read_sp_el2();

	/* mask IRQ, FIQ*/
	uint64_t ns_spsr = SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	uint64_t scr_el3 = read_ctx_reg(state, CTX_SCR_EL3);

	write_ctx_reg(state, CTX_SPSR_EL3, ns_spsr);
	write_ctx_reg(state, CTX_SCR_EL3, scr_el3 & (~SCR_FIQ_BIT));

	/* Update parameters passed from S-EL1 for EL2 */
	write_ctx_reg(ns_gpregs, CTX_GPREG_X0, smc_fid);
	write_ctx_reg(ns_gpregs, CTX_GPREG_X1, x1);
	write_ctx_reg(ns_gpregs, CTX_GPREG_X2, x2);
	write_ctx_reg(ns_gpregs, CTX_GPREG_X3, x3);
	write_ctx_reg(ns_gpregs, CTX_GPREG_X4, x4);
	/* Update el2 entry point */
	write_ctx_reg(state, CTX_ELR_EL3, el2_entry);

	/* Restore non-secure context for entering EL2 */
	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);

	/* Switch C runtime context */
	rc = tlkd_enter_sp(&backup_context->c_rt_ctx);
	/* Reset C runtime after exit */
	backup_context->c_rt_ctx = 0x0;
	return rc;
}

#if EL2_CONTEXT_DEBUG_EN
#define REG_CHECK(group, reg_name)                                              \
	do {                                                                    \
		if (read_ctx_reg(prev_##group##_regs, reg_name)                 \
			!= read_ctx_reg(new_##group##_regs, reg_name)) {        \
			NOTICE("%s: REG=%s is changed from 0x%llx to 0x%llx\n", \
				__func__, #reg_name,                            \
				read_ctx_reg(prev_##group##_regs, reg_name),    \
				read_ctx_reg(new_##group##_regs, reg_name));    \
		}                                                               \
	} while (0)

static void el2_context_check(cpu_context_t *prev, cpu_context_t *new,
			      struct plat_cpu_context *plat)
{
	gp_regs_t *prev_gp_regs = get_gpregs_ctx(prev);
	el3_state_t *prev_el3_state_regs = get_el3state_ctx(prev);
	el1_sys_regs_t *prev_el1_sys_regs = get_sysregs_ctx(prev);
	gp_regs_t *new_gp_regs = get_gpregs_ctx(new);
	el3_state_t *new_el3_state_regs = get_el3state_ctx(new);
	el1_sys_regs_t *new_el1_sys_regs = get_sysregs_ctx(new);

	REG_CHECK(gp, CTX_GPREG_X0);
	REG_CHECK(gp, CTX_GPREG_X1);
	REG_CHECK(gp, CTX_GPREG_X2);
	REG_CHECK(gp, CTX_GPREG_X3);
	REG_CHECK(gp, CTX_GPREG_X4);
	REG_CHECK(gp, CTX_GPREG_X5);
	REG_CHECK(gp, CTX_GPREG_X6);
	REG_CHECK(gp, CTX_GPREG_X7);
	REG_CHECK(gp, CTX_GPREG_X8);
	REG_CHECK(gp, CTX_GPREG_X9);
	REG_CHECK(gp, CTX_GPREG_X10);
	REG_CHECK(gp, CTX_GPREG_X11);
	REG_CHECK(gp, CTX_GPREG_X12);
	REG_CHECK(gp, CTX_GPREG_X13);
	REG_CHECK(gp, CTX_GPREG_X14);
	REG_CHECK(gp, CTX_GPREG_X15);
	REG_CHECK(gp, CTX_GPREG_X16);
	REG_CHECK(gp, CTX_GPREG_X17);
	REG_CHECK(gp, CTX_GPREG_X18);
	REG_CHECK(gp, CTX_GPREG_X19);
	REG_CHECK(gp, CTX_GPREG_X20);
	REG_CHECK(gp, CTX_GPREG_X21);
	REG_CHECK(gp, CTX_GPREG_X22);
	REG_CHECK(gp, CTX_GPREG_X23);
	REG_CHECK(gp, CTX_GPREG_X24);
	REG_CHECK(gp, CTX_GPREG_X25);
	REG_CHECK(gp, CTX_GPREG_X26);
	REG_CHECK(gp, CTX_GPREG_X27);
	REG_CHECK(gp, CTX_GPREG_X28);
	REG_CHECK(gp, CTX_GPREG_X29);
	REG_CHECK(gp, CTX_GPREG_LR);
	REG_CHECK(gp, CTX_GPREG_SP_EL0);

	REG_CHECK(el3_state, CTX_SCR_EL3);
	REG_CHECK(el3_state, CTX_ESR_EL3);
	REG_CHECK(el3_state, CTX_RUNTIME_SP);
	REG_CHECK(el3_state, CTX_SPSR_EL3);
	REG_CHECK(el3_state, CTX_ELR_EL3);
	REG_CHECK(el3_state, CTX_UNUSED);

	REG_CHECK(el1_sys, CTX_SPSR_EL1);
	REG_CHECK(el1_sys, CTX_ELR_EL1);
	REG_CHECK(el1_sys, CTX_SCTLR_EL1);
	REG_CHECK(el1_sys, CTX_ACTLR_EL1);
	REG_CHECK(el1_sys, CTX_CPACR_EL1);
	REG_CHECK(el1_sys, CTX_CSSELR_EL1);
	REG_CHECK(el1_sys, CTX_SP_EL1);
	REG_CHECK(el1_sys, CTX_ESR_EL1);
	REG_CHECK(el1_sys, CTX_TTBR0_EL1);
	REG_CHECK(el1_sys, CTX_TTBR1_EL1);
	REG_CHECK(el1_sys, CTX_MAIR_EL1);
	REG_CHECK(el1_sys, CTX_AMAIR_EL1);
	REG_CHECK(el1_sys, CTX_TPIDR_EL1);
	REG_CHECK(el1_sys, CTX_TPIDR_EL0);
	REG_CHECK(el1_sys, CTX_TPIDRRO_EL0);
	REG_CHECK(el1_sys, CTX_PAR_EL1);
	REG_CHECK(el1_sys, CTX_FAR_EL1);
	REG_CHECK(el1_sys, CTX_AFSR0_EL1);
	REG_CHECK(el1_sys, CTX_AFSR1_EL1);
	REG_CHECK(el1_sys, CTX_CONTEXTIDR_EL1);
	REG_CHECK(el1_sys, CTX_VBAR_EL1);
	REG_CHECK(el1_sys, CTX_PMCR_EL0);
#if CTX_INCLUDE_AARCH32_REGS
	REG_CHECK(el1_sys, CTX_SPSR_ABT);
	REG_CHECK(el1_sys, CTX_SPSR_UND);
	REG_CHECK(el1_sys, CTX_SPSR_IRQ);
	REG_CHECK(el1_sys, CTX_SPSR_FIQ);
	REG_CHECK(el1_sys, CTX_DACR32_EL2);
	REG_CHECK(el1_sys, CTX_IFSR32_EL2);
#endif
	REG_CHECK(el1_sys, CTX_TIMER_SYSREGS_OFF);
#if NS_TIMER_SWITCH
	REG_CHECK(el1_sys, CTX_CNTP_CTL_EL0);
	REG_CHECK(el1_sys, CTX_CNTP_CVAL_EL0);
	REG_CHECK(el1_sys, CTX_CNTV_CTL_EL0);
	REG_CHECK(el1_sys, CTX_CNTV_CVAL_EL0);
	REG_CHECK(el1_sys, CTX_CNTKCTL_EL1);
#endif

	if (plat->saved_sp_el2 != read_sp_el2())
		NOTICE("%s: REG=%s is changed from 0x%llx to 0x%llx\n",
		       __func__, "SP_EL2", plat->saved_sp_el2,
		       (uint64_t)read_sp_el2());
}
#endif

static void el2_exit_sp(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			u_register_t x3, u_register_t x4)
{
	uint32_t cpu_id = plat_core_pos_by_mpidr(read_mpidr());
	cpu_context_t *ns_context = (cpu_context_t *)cm_get_context(NON_SECURE);
	struct plat_cpu_context *backup_context = &plat_context[cpu_id];

	/* Save return value from EL2 */
	backup_context->el2_ret_x0 = smc_fid;
	backup_context->el2_ret_x1 = x1;
	backup_context->el2_ret_x2 = x2;
	backup_context->el2_ret_x3 = x3;
	backup_context->el2_ret_x4 = x4;

	/* Restore original REE context */
#if EL2_CONTEXT_DEBUG_EN
	el2_context_check(&backup_context->_cpu_context, ns_context,
			  backup_context);
#endif
	memcpy(ns_context, &backup_context->_cpu_context,
	       sizeof(cpu_context_t));
	write_sp_el2(backup_context->saved_sp_el2);

	/* Switch C runtime context */
	assert(backup_context->c_rt_ctx != 0);
	tlkd_exit_sp(backup_context->c_rt_ctx, backup_context->el2_ret_x1);

	/* Should never reach here */
	assert(0);
}

uint64_t sip_smc_to_el2(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			u_register_t x3, u_register_t x4)
{
	uint64_t rc = 0;
	uint32_t cpu_id = plat_core_pos_by_mpidr(read_mpidr());
	struct plat_cpu_context *backup_context = &plat_context[cpu_id];
	cpu_context_t *s_context = (cpu_context_t *)cm_get_context(SECURE);
	gp_regs_t *s_gpregs = get_gpregs_ctx(s_context);
	uintptr_t el2_smc_entry = EL2_ENTRY_VA;

	EL2_SMC_DBG("BL31: got smc request from s-el1! (x1=0x%x)\n",
		    (uint32_t)x1);
	EL2_SMC_DBG("BL31: got smc request from s-el1! (x2=0x%x)\n",
		    (uint32_t)x2);
	EL2_SMC_DBG("BL31: got smc request from s-el1! (x3=0x%x)\n",
		    (uint32_t)x3);
	EL2_SMC_DBG("BL31: got smc request from s-el1! (x4=0x%x)\n",
		    (uint32_t)x4);

	/* Save secure context */
	cm_el1_sysregs_context_save(SECURE);

	/* Enter EL2 */
	EL2_SMC_DBG("BL31: enter EL2! (entry=0x%llx)\n",
		    (uint64_t)el2_smc_entry);
	el2_enter_sp(smc_fid, x1, x2, x3, x4, el2_smc_entry);
	EL2_SMC_DBG("BL31: enter EL2! (done)\n");

	/* Restore secure context for entering S-EL1 */
	EL2_SMC_DBG("BL31: return to S-EL1 (ret=0x%x)!\n",
		    (uint32_t)backup_context->el2_ret_x1);
	write_ctx_reg(s_gpregs, CTX_GPREG_X0, backup_context->el2_ret_x1);
	write_ctx_reg(s_gpregs, CTX_GPREG_X1, backup_context->el2_ret_x2);
	write_ctx_reg(s_gpregs, CTX_GPREG_X2, backup_context->el2_ret_x3);
	write_ctx_reg(s_gpregs, CTX_GPREG_X3, backup_context->el2_ret_x4);
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	rc = backup_context->el2_ret_x1;
	return rc;
}

uint64_t sip_smc_to_el2_done(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			     u_register_t x3, u_register_t x4)
{
	EL2_SMC_DBG("BL31: got smc done from el2! (x1=0x%x)\n", (uint32_t)x1);
	EL2_SMC_DBG("BL31: got smc done from el2! (x2=0x%x)\n", (uint32_t)x2);
	EL2_SMC_DBG("BL31: got smc done from el2! (x3=0x%x)\n", (uint32_t)x3);
	EL2_SMC_DBG("BL31: got smc done from el2! (x4=0x%x)\n", (uint32_t)x4);

	/* Exit EL2 */
	EL2_SMC_DBG("BL31: exit EL2!\n");
	el2_exit_sp(smc_fid, x1, x2, x3, x4);
	EL2_SMC_DBG("BL31: exit EL2 (never goes here)!\n");
	return 0;
}
