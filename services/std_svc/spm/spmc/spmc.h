/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMC_H
#define SPMC_H

#include <stdint.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>

#include "logical_sp.h"
#include "spm_common.h"

/*
 * Ranges of FF-A IDs for Normal world and Secure world components. The
 * convention matches that used by other SPMCs i.e. Hafnium and OP-TEE.
 */
#define FFA_NWD_ID_BASE	0x0
#define FFA_NWD_ID_LIMIT	0x7FFF
#define FFA_SWD_ID_BASE	0x8000
#define FFA_SWD_ID_LIMIT	0xFFFF
#define FFA_SWD_ID_MASK	0x8000

#define FFA_SPMC_ID		U(FFA_SWD_ID_BASE)	/* First ID is reserved for the SPMC */
#define FFA_SP_ID_BASE		(FFA_SPMC_ID + 1)	/* SP IDs are allocated after the SPMC ID */
#define INV_SP_ID		0x7FFF			/* Align with Hafnium implementation */

/* FFA warm boot types. */
#define FFA_WB_TYPE_S2RAM		0
#define FFA_WB_TYPE_NOTS2RAM		1

/* FFA Related helper macros */
#define FFA_ID_MASK			U(0xFFFF)
#define FFA_PARTITION_ID_SHIFT		U(16)

#define FFA_FEATURES_BIT31_MASK		U(0x1u << 31)


#define FFA_MSG_WAIT_EP_ID(ep_vcpu_ids) \
		((ep_vcpu_ids >> FFA_PARTITION_ID_SHIFT) & FFA_ID_MASK)
#define FFA_RUN_EP_ID(src_dst_ids) \
		((src_dst_ids >> FFA_PARTITION_ID_SHIFT) & FFA_ID_MASK)

#define FFA_PAGE_SIZE (4096)
#define FFA_RXTX_PAGE_COUNT_MASK 0x1F

/*
 * Defines to allow an SP to subscribe for power management messages
 */
#define FFA_PM_MSG_SUB_CPU_OFF		0x1

/*
 * Runtime states of an execution context as per the FF-A v1.1 specification.
 */
enum runtime_states {
	RT_STATE_WAITING,
	RT_STATE_RUNNING,
	RT_STATE_PREEMPTED,
	RT_STATE_BLOCKED
};

/*
 * Runtime model of an execution context as per the FF-A v1.1 specification. Its
 * value is valid only if the execution context is not in the waiting state.
 */
enum runtime_model {
	RT_MODEL_DIR_REQ,
	RT_MODEL_RUN,
	RT_MODEL_INIT,
	RT_MODEL_INTR
};

enum runtime_el {
	EL0 = 0,
	EL1
};

enum mailbox_state {
	/* There is no message in the mailbox. */
	MAILBOX_STATE_EMPTY,

	/* There is a message that has been populated in the mailbox. */
	MAILBOX_STATE_FULL,
};


struct mailbox {
	enum mailbox_state state;

	/* RX/TX Buffers */
	void *rx_buffer;
	const void *tx_buffer;

	/*
	 * Size of RX/TX Buffer
	 */
	uint32_t rxtx_page_count;

	/* Lock access to mailbox */
	spinlock_t lock;
};

/*
 * Execution context members for an SP. This is a bit like struct
 * vcpu in a hypervisor.
 */
typedef struct sp_exec_ctx {
	uint64_t c_rt_ctx;
	cpu_context_t cpu_ctx;
	enum runtime_states rt_state;
	enum runtime_model rt_model;
} sp_exec_ctx_t;

/*
 * Structure to describe the cumulative properties of an SP.
 */
typedef struct secure_partition_desc {
	/*
	 * Execution contexts allocated to this endpoint. Ideally,
	 * we need as many contexts as there are physical cpus only for a S-EL1
	 * SP which is MP-pinned.
	 */
	sp_exec_ctx_t ec[PLATFORM_CORE_COUNT];

	/*
	 * ID of the Secure Partition
	 */
	uint16_t sp_id;

	/*
	 * Runtime EL
	 */
	uint16_t runtime_el;

	/*
	 * Partition UUID
	 */
	uint32_t uuid[4];

	/*
	 * Partition Properties
	 */
	uint32_t properties;

	/*
	 * Supported FFA Version
	 */
	uint32_t ffa_version;

	/*
	 * Execution State
	 */
	uint32_t execution_state;

	/*
	 * Mailbox tracking
	 */
	struct mailbox mailbox;

	/*
	 * Secondary entrypoint. Only valid for a S-EL1 SP.
	 */
	uintptr_t secondary_ep;

	/*
	 * Lock to protect the secondary entrypoint update in a SP descriptor.
	 */
	spinlock_t secondary_ep_lock;

	/*
	 * Store whether the SP has subscribed to any power management messages.
	 */
	uint16_t pwr_mgmt_msgs;

} sp_desc_t;

/*
 * This define identifies the only SP that will be initialised and participate
 * in FF-A communication. The implementation leaves the door open for more SPs
 * to be managed in future but for now it is reasonable to assume that either a
 * single S-EL0 or a single S-EL1 SP will be supported. This define will be used
 * to identify which SP descriptor to initialise and manage during SP runtime.
 */
#define ACTIVE_SP_DESC_INDEX	0

/*
 * Structure to describe the cumulative properties of the Hypervisor and
 * NS-Endpoints.
 */
typedef struct ns_endpoint_desc {
	/*
	 * ID of the NS-Endpoint or Hypervisor
	 */
	uint16_t ns_ep_id;

	/*
	 * Mailbox tracking
	 */
	struct mailbox mailbox;

	/*
	 * Supported FFA Version
	 */
	uint32_t ffa_version;

} ns_ep_desc_t;

/**
 * Holds information returned for each partition by the FFA_PARTITION_INFO_GET
 * interface.
 */
struct ffa_partition_info_v1_0 {
	uint16_t ep_id;
	uint16_t execution_ctx_count;
	uint32_t properties;
};

/* Extended structure for v1.1. */
struct ffa_partition_info_v1_1 {
	uint16_t ep_id;
	uint16_t execution_ctx_count;
	uint32_t properties;
	uint32_t uuid[4];
};

/* Setup Function for different SP types. */
void spmc_sp_common_setup(sp_desc_t *sp, entry_point_info_t *ep_info);
void spmc_el1_sp_setup(sp_desc_t *sp, entry_point_info_t *ep_info);

/*
 * Helper function to perform a synchronous entry into a SP.
 */
uint64_t spmc_sp_synchronous_entry(sp_exec_ctx_t *ec);

/*
 * Helper function to obtain the descriptor of the current SP on a physical cpu.
 */
sp_desc_t *spmc_get_current_sp_ctx(void);

/*
 * Helper function to obtain the index of the execution context of an SP on a
 * physical cpu.
 */
unsigned int get_ec_index(sp_desc_t *sp);

uint64_t spmc_ffa_error_return(void *handle, int error_code);

/*
 * Ensure a partition ID does not clash and follow the secure world convention.
 */
bool validate_partition_id(uint16_t partition_id);

/*
 * Helper function to obtain the array storing the EL3
 * Logical Partition descriptors.
 */

el3_lp_desc_t *get_el3_lp_array(void);

/*
 * Helper function to obtain the RX/TX buffer pair descriptor of the Hypervisor
 * or the last SP that was run.
 */
struct mailbox *spmc_get_mbox_desc(bool secure_origin);

#endif /* SPMC_H */
