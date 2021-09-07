/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PNCD_PRIVATE_H
#define PNCD_PRIVATE_H

#include <arch.h>
#include <context.h>

#include <bl31/interrupt_mgmt.h>
#include <lib/psci/psci.h>
#include <platform_def.h>

/*******************************************************************************
 * Secure Payload PM state information e.g. SP is suspended, uninitialised etc
 * and macros to access the state information in the per-cpu 'state' flags
 ******************************************************************************/
#define PNC_PSTATE_OFF		0
#define PNC_PSTATE_ON		1
#define PNC_PSTATE_SUSPEND	2
#define PNC_PSTATE_SHIFT	0
#define PNC_PSTATE_MASK	    0x3
#define get_pnc_pstate(state)	((state >> PNC_PSTATE_SHIFT) & PNC_PSTATE_MASK)
#define clr_pnc_pstate(state)	(state &= ~(PNC_PSTATE_MASK \
					    << PNC_PSTATE_SHIFT))
#define set_pnc_pstate(st, pst)	do {					       \
					clr_pnc_pstate(st);		       \
					st |= (pst & PNC_PSTATE_MASK) <<       \
						PNC_PSTATE_SHIFT;	       \
				} while (0);


/*
 * This flag is used by the PNCD to determine if the PNC is servicing a standard
 * SMC request prior to programming the next entry into the PNC e.g. if PNC
 * execution is preempted by a non-secure interrupt and handed control to the
 * normal world. If another request which is distinct from what the PNC was
 * previously doing arrives, then this flag will be help the PNCD to either
 * reject the new request or service it while ensuring that the previous context
 * is not corrupted.
 */
#define STD_SMC_ACTIVE_FLAG_SHIFT	2
#define STD_SMC_ACTIVE_FLAG_MASK	1
#define get_std_smc_active_flag(state)	((state >> STD_SMC_ACTIVE_FLAG_SHIFT) \
					 & STD_SMC_ACTIVE_FLAG_MASK)
#define set_std_smc_active_flag(state)	(state |=                             \
					 1 << STD_SMC_ACTIVE_FLAG_SHIFT)
#define clr_std_smc_active_flag(state)	(state &=                             \
					 ~(STD_SMC_ACTIVE_FLAG_MASK           \
					   << STD_SMC_ACTIVE_FLAG_SHIFT))

/*******************************************************************************
 * Secure Payload execution state information i.e. aarch32 or aarch64
 ******************************************************************************/
#define PNC_AARCH32		MODE_RW_32
#define PNC_AARCH64		MODE_RW_64

/*******************************************************************************
 * The SPD should know the type of Secure Payload.
 ******************************************************************************/
#define PNC_TYPE_UP		PSCI_TOS_NOT_UP_MIG_CAP
#define PNC_TYPE_UPM	PSCI_TOS_UP_MIG_CAP
#define PNC_TYPE_MP		PSCI_TOS_NOT_PRESENT_MP

/*******************************************************************************
 * Secure Payload migrate type information as known to the SPD. We assume that
 * the SPD is dealing with an MP Secure Payload.
 ******************************************************************************/
#define PNC_MIGRATE_INFO		PNC_TYPE_MP

/*******************************************************************************
 * Number of cpus that the present on this platform. TODO: Rely on a topology
 * tree to determine this in the future to avoid assumptions about mpidr
 * allocation
 ******************************************************************************/
#define PNCD_CORE_COUNT		PLATFORM_CORE_COUNT

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define PNCD_C_RT_CTX_X19		0x0
#define PNCD_C_RT_CTX_X20		0x8
#define PNCD_C_RT_CTX_X21		0x10
#define PNCD_C_RT_CTX_X22		0x18
#define PNCD_C_RT_CTX_X23		0x20
#define PNCD_C_RT_CTX_X24		0x28
#define PNCD_C_RT_CTX_X25		0x30
#define PNCD_C_RT_CTX_X26		0x38
#define PNCD_C_RT_CTX_X27		0x40
#define PNCD_C_RT_CTX_X28		0x48
#define PNCD_C_RT_CTX_X29		0x50
#define PNCD_C_RT_CTX_X30		0x58
#define PNCD_C_RT_CTX_SIZE		0x60
#define PNCD_C_RT_CTX_ENTRIES		(PNCD_C_RT_CTX_SIZE >> DWORD_SHIFT)

/*******************************************************************************
 * Constants that allow assembler code to preserve caller-saved registers of the
 * SP context while performing a PNC preemption.
 * Note: These offsets have to match with the offsets for the corresponding
 * registers in cpu_context as we are using memcpy to copy the values from
 * cpu_context to sp_ctx.
 ******************************************************************************/
#define PNCD_SP_CTX_X0		0x0
#define PNCD_SP_CTX_X1		0x8
#define PNCD_SP_CTX_X2		0x10
#define PNCD_SP_CTX_X3		0x18
#define PNCD_SP_CTX_X4		0x20
#define PNCD_SP_CTX_X5		0x28
#define PNCD_SP_CTX_X6		0x30
#define PNCD_SP_CTX_X7		0x38
#define PNCD_SP_CTX_X8		0x40
#define PNCD_SP_CTX_X9		0x48
#define PNCD_SP_CTX_X10		0x50
#define PNCD_SP_CTX_X11		0x58
#define PNCD_SP_CTX_X12		0x60
#define PNCD_SP_CTX_X13		0x68
#define PNCD_SP_CTX_X14		0x70
#define PNCD_SP_CTX_X15		0x78
#define PNCD_SP_CTX_X16		0x80
#define PNCD_SP_CTX_X17		0x88
#define PNCD_SP_CTX_SIZE	0x90
#define PNCD_SP_CTX_ENTRIES		(PNCD_SP_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <lib/cassert.h>

/*
 * The number of arguments to save during a SMC call for PNC.
 * Currently only x1 and x2 are used by PNC.
 */
#define PNC_NUM_ARGS	0x2

/* AArch64 callee saved general purpose register context structure. */
DEFINE_REG_STRUCT(c_rt_regs, PNCD_C_RT_CTX_ENTRIES);

/*
 * Compile time assertion to ensure that both the compiler and linker
 * have the same double word aligned view of the size of the C runtime
 * register context.
 */
CASSERT(sizeof(c_rt_regs_t) == PNCD_C_RT_CTX_SIZE, assert_spd_c_rt_regs_size_mismatch);

/* SEL1 Secure payload (SP) caller saved register context structure. */
DEFINE_REG_STRUCT(sp_ctx_regs, PNCD_SP_CTX_ENTRIES);

/*
 * Compile time assertion to ensure that both the compiler and linker
 * have the same double word aligned view of the size of the C runtime
 * register context.
 */
CASSERT(sizeof(sp_ctx_regs_t) == PNCD_SP_CTX_SIZE, assert_spd_sp_regs_size_mismatch);

/*******************************************************************************
 * Structure which helps the SPD to maintain the per-cpu state of the SP.
 * 'saved_spsr_el3' - temporary copy to allow S-EL1 interrupt handling when
 *                    the PNC has been preempted.
 * 'saved_elr_el3'  - temporary copy to allow S-EL1 interrupt handling when
 *                    the PNC has been preempted.
 * 'state'          - collection of flags to track SP state e.g. on/off
 * 'mpidr'          - mpidr to associate a context with a cpu
 * 'c_rt_ctx'       - stack address to restore C runtime context from after
 *                    returning from a synchronous entry into the SP.
 * 'cpu_ctx'        - space to maintain SP architectural state
 * 'saved_PNC_args' - space to store arguments for PNC arithmetic operations
 *                    which will queried using the PNC_GET_ARGS SMC by PNC.
 * 'sp_ctx'         - space to save the SEL1 Secure Payload(SP) caller saved
 *                    register context after it has been preempted by an EL3
 *                    routed NS interrupt and when a Secure Interrupt is taken
 *                    to SP.
 ******************************************************************************/
typedef struct pnc_context {
	uint64_t saved_elr_el3;
	uint32_t saved_spsr_el3;
	uint32_t state;
	uint64_t mpidr;
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
	uint64_t saved_pnc_args[PNC_NUM_ARGS];
#if PNCD_ROUTE_IRQ_TO_EL3
	sp_ctx_regs_t sp_ctx;
#endif
} pnc_context_t;

/* Helper macros to store and retrieve pnc args from pnc_context */
#define store_pnc_args(pnc_ctx, x1, x2)		do {\
				pnc_ctx->saved_pnc_args[0] = x1;\
				pnc_ctx->saved_pnc_args[1] = x2;\
			} while (0)

#define get_pnc_args(pnc_ctx, x1, x2)	do {\
				x1 = pnc_ctx->saved_pnc_args[0];\
				x2 = pnc_ctx->saved_pnc_args[1];\
			} while (0)

/* PNCD power management handlers */
extern const spd_pm_ops_t pncd_pm;

/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
uint64_t pncd_enter_sp(uint64_t *c_rt_ctx);
void __dead2 pncd_exit_sp(uint64_t c_rt_ctx, uint64_t ret);
uint64_t pncd_synchronous_sp_entry(pnc_context_t *pnc_ctx);
void __dead2 pncd_synchronous_sp_exit(pnc_context_t *pnc_ctx, uint64_t ret);
void pncd_init_pnc_ep_state(struct entry_point_info *pnc_ep,
				uint32_t rw,
				uint64_t pc,
				pnc_context_t *pnc_ctx);

/* Generic Timer functions */
void pnc_generic_timer_start(void);
void pnc_generic_timer_handler(void);
void pnc_generic_timer_stop(void);
void pnc_generic_timer_save(void);
void pnc_generic_timer_restore(void);

extern pnc_context_t pncd_sp_context[PNCD_CORE_COUNT];
#endif /*__ASSEMBLER__*/

#endif /* PNCD_PRIVATE_H */
