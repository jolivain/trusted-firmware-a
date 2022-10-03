/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OPTEED_PRIVATE_H
#define OPTEED_PRIVATE_H

#include <platform_def.h>

#include <arch.h>
#include <bl31/interrupt_mgmt.h>
#include <context.h>
#include <lib/psci/psci.h>

/*******************************************************************************
 * OPTEE PM state information e.g. OPTEE is suspended, uninitialised etc
 * and macros to access the state information in the per-cpu 'state' flags
 ******************************************************************************/
#define OPTEE_PSTATE_OFF			1
#define OPTEE_PSTATE_ON				2
#define OPTEE_PSTATE_SUSPEND	3
#define OPTEE_PSTATE_UNKNOWN	0
#define OPTEE_PSTATE_SHIFT		0
#define OPTEE_PSTATE_MASK		0x3
#define get_optee_pstate(state)	((state >> OPTEE_PSTATE_SHIFT) & \
				 OPTEE_PSTATE_MASK)
#define clr_optee_pstate(state)	(state &= ~(OPTEE_PSTATE_MASK \
					    << OPTEE_PSTATE_SHIFT))
#define set_optee_pstate(st, pst) do {					       \
					clr_optee_pstate(st);		       \
					st |= (pst & OPTEE_PSTATE_MASK) <<     \
						OPTEE_PSTATE_SHIFT;	       \
				} while (0)


/*******************************************************************************
 * OPTEE execution state information i.e. aarch32 or aarch64
 ******************************************************************************/
#define OPTEE_AARCH32		MODE_RW_32
#define OPTEE_AARCH64		MODE_RW_64

/*******************************************************************************
 * The OPTEED should know the type of OPTEE
 ******************************************************************************/
#define OPTEE_TYPE_UP		PSCI_TOS_NOT_UP_MIG_CAP
#define OPTEE_TYPE_UPM		PSCI_TOS_UP_MIG_CAP
#define OPTEE_TYPE_MP		PSCI_TOS_NOT_PRESENT_MP

/*******************************************************************************
 * OPTEE migrate type information as known to the OPTEED. We assume that
 * the OPTEED is dealing with an MP Secure Payload.
 ******************************************************************************/
#define OPTEE_MIGRATE_INFO		OPTEE_TYPE_MP

/*******************************************************************************
 * Number of cpus that the present on this platform. TODO: Rely on a topology
 * tree to determine this in the future to avoid assumptions about mpidr
 * allocation
 ******************************************************************************/
#define OPTEED_CORE_COUNT		PLATFORM_CORE_COUNT

/*******************************************************************************
 * Constants that allow assembler code to preserve callee-saved registers of the
 * C runtime context while performing a security state switch.
 ******************************************************************************/
#define OPTEED_C_RT_CTX_X19		0x0
#define OPTEED_C_RT_CTX_X20		0x8
#define OPTEED_C_RT_CTX_X21		0x10
#define OPTEED_C_RT_CTX_X22		0x18
#define OPTEED_C_RT_CTX_X23		0x20
#define OPTEED_C_RT_CTX_X24		0x28
#define OPTEED_C_RT_CTX_X25		0x30
#define OPTEED_C_RT_CTX_X26		0x38
#define OPTEED_C_RT_CTX_X27		0x40
#define OPTEED_C_RT_CTX_X28		0x48
#define OPTEED_C_RT_CTX_X29		0x50
#define OPTEED_C_RT_CTX_X30		0x58
#define OPTEED_C_RT_CTX_SIZE		0x60
#define OPTEED_C_RT_CTX_ENTRIES		(OPTEED_C_RT_CTX_SIZE >> DWORD_SHIFT)

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <lib/cassert.h>

typedef uint32_t optee_vector_isn_t;

typedef struct optee_vectors {
	optee_vector_isn_t yield_smc_entry;
	optee_vector_isn_t fast_smc_entry;
	optee_vector_isn_t cpu_on_entry;
	optee_vector_isn_t cpu_off_entry;
	optee_vector_isn_t cpu_resume_entry;
	optee_vector_isn_t cpu_suspend_entry;
	optee_vector_isn_t fiq_entry;
	optee_vector_isn_t system_off_entry;
	optee_vector_isn_t system_reset_entry;
} optee_vectors_t;

/*
 * The number of arguments to save during a SMC call for OPTEE.
 * Currently only x1 and x2 are used by OPTEE.
 */
#define OPTEE_NUM_ARGS	0x2

/* AArch64 callee saved general purpose register context structure. */
DEFINE_REG_STRUCT(c_rt_regs, OPTEED_C_RT_CTX_ENTRIES);

/*
 * Compile time assertion to ensure that both the compiler and linker
 * have the same double word aligned view of the size of the C runtime
 * register context.
 */
CASSERT(OPTEED_C_RT_CTX_SIZE == sizeof(c_rt_regs_t),	\
	assert_spd_c_rt_regs_size_mismatch);

/*******************************************************************************
 * Structure which helps the OPTEED to maintain the per-cpu state of OPTEE.
 * 'state'          - collection of flags to track OPTEE state e.g. on/off
 * 'mpidr'          - mpidr to associate a context with a cpu
 * 'c_rt_ctx'       - stack address to restore C runtime context from after
 *                    returning from a synchronous entry into OPTEE.
 * 'cpu_ctx'        - space to maintain OPTEE architectural state
 ******************************************************************************/
typedef struct optee_context {
	uint32_t state;
	uint64_t mpidr;
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
} optee_context_t;

/* OPTEED power management handlers */
extern const spd_pm_ops_t opteed_pm;

/*******************************************************************************
 * Structures used for loading the OPTEE image via an SMC. The SMC call will
 * send a PA pointing to the optee_image_info struct and image data. The image
 * segments will be copied to the entry point at the specified offsets. Any gaps
 * in the range will be filled with zeros.
 * struct optee_image_info
 * 'magic'          - contains the magic identifier bytes MAGIC_OPTEE_SMC_IMAGE
 * 'version'        - version of the data, only 0 is currently supported
 * 'arch'           - machine architecture, corresponds to a OPTEE_AARCH* value
 * 'num_segments'   - number of optee_segment_t items in the segments flex array
 * 'image_info_size'- total size of the optee_image_info structure
 * 'entry_point'    - entry point for execution of OPTEE
 * 'segments'       - array of all the segments which make up the OPTEED image
 *
 * struct optee_segment
 * 'segment_pa'        - physical address to load this segment to
 * 'segment_data_size' - size in bytes of this segment to be copied
 * 'segment_mem_size'  - size in bytes of the segment in memory, anything beyond
 *                       segment_data_size should be zero'd out
 *
 * The PA received in the SMC will point to memory that is laid out as follows:
 * struct optee_image_info (with optee_segment flex array)
 * segment 1
 * segment 2
 * ...
 ******************************************************************************/
#define MAGIC_OPTEE_SMC_IMAGE 0x4F505445
typedef struct optee_segment {
	uint64_t segment_pa;
	uint64_t segment_data_size;
	uint64_t segment_mem_size;
} optee_segment_t;
typedef struct optee_image_info {
	uint32_t magic;
	uint8_t version;
	uint8_t arch;
	uint16_t num_segments;
	uint64_t image_info_size;
	uint64_t entry_point;
	optee_segment_t segments[];
} optee_image_info_t;

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct optee_vectors;

/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
uint64_t opteed_enter_sp(uint64_t *c_rt_ctx);
void __dead2 opteed_exit_sp(uint64_t c_rt_ctx, uint64_t ret);
uint64_t opteed_synchronous_sp_entry(optee_context_t *optee_ctx);
void __dead2 opteed_synchronous_sp_exit(optee_context_t *optee_ctx, uint64_t ret);
void opteed_init_optee_ep_state(struct entry_point_info *optee_entry_point,
				uint32_t rw,
				uint64_t pc,
				uint64_t pageable_part,
				uint64_t mem_limit,
				uint64_t dt_addr,
				optee_context_t *optee_ctx);
void opteed_cpu_on_finish_handler(u_register_t unused);

extern optee_context_t opteed_sp_context[OPTEED_CORE_COUNT];
extern uint32_t opteed_rw;
extern struct optee_vectors *optee_vector_table;
#endif /*__ASSEMBLER__*/

#endif /* OPTEED_PRIVATE_H */
