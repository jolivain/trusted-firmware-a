/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PSCI_H
#define PSCI_H

#include <platform_def.h>	/* for PLAT_NUM_PWR_DOMAINS */

#include <common/bl_common.h>
#include <lib/bakery_lock.h>
#include <lib/psci/psci_lib.h>	/* To maintain compatibility for SPDs */
#include <lib/utils_def.h>

/*******************************************************************************
 * Number of power domains whose state this PSCI implementation can track
 ******************************************************************************/
#ifdef PLAT_NUM_PWR_DOMAINS
#define PSCI_NUM_PWR_DOMAINS	PLAT_NUM_PWR_DOMAINS
#else
#define PSCI_NUM_PWR_DOMAINS	((2U) * PLATFORM_CORE_COUNT)
#endif

#define PSCI_NUM_NON_CPU_PWR_DOMAINS	(PSCI_NUM_PWR_DOMAINS - \
					 PLATFORM_CORE_COUNT)

/* This is the power level corresponding to a CPU */
#define PSCI_CPU_PWR_LVL	(0U)

/*
 * The maximum power level supported by PSCI. Since PSCI CPU_SUSPEND
 * uses the old power_state parameter format which has 2 bits to specify the
 * power level, this constant is defined to be 3.
 */
#define PSCI_MAX_PWR_LVL	(3U)

/*******************************************************************************
 * Defines for runtime services function ids
 ******************************************************************************/
#define PSCI_VERSION			(0x84000000U)
#define PSCI_CPU_SUSPEND_AARCH32	(0x84000001U)
#define PSCI_CPU_SUSPEND_AARCH64	(0xc4000001U)
#define PSCI_CPU_OFF			(0x84000002U)
#define PSCI_CPU_ON_AARCH32		(0x84000003U)
#define PSCI_CPU_ON_AARCH64		(0xc4000003U)
#define PSCI_AFFINITY_INFO_AARCH32	(0x84000004U)
#define PSCI_AFFINITY_INFO_AARCH64	(0xc4000004U)
#define PSCI_MIG_AARCH32		(0x84000005U)
#define PSCI_MIG_AARCH64		(0xc4000005U)
#define PSCI_MIG_INFO_TYPE		(0x84000006U)
#define PSCI_MIG_INFO_UP_CPU_AARCH32	(0x84000007U)
#define PSCI_MIG_INFO_UP_CPU_AARCH64	(0xc4000007U)
#define PSCI_SYSTEM_OFF			(0x84000008U)
#define PSCI_SYSTEM_RESET		(0x84000009U)
#define PSCI_FEATURES			(0x8400000AU)
#define PSCI_NODE_HW_STATE_AARCH32	(0x8400000dU)
#define PSCI_NODE_HW_STATE_AARCH64	(0xc400000dU)
#define PSCI_SYSTEM_SUSPEND_AARCH32	(0x8400000EU)
#define PSCI_SYSTEM_SUSPEND_AARCH64	(0xc400000EU)
#define PSCI_STAT_RESIDENCY_AARCH32	(0x84000010U)
#define PSCI_STAT_RESIDENCY_AARCH64	(0xc4000010U)
#define PSCI_STAT_COUNT_AARCH32		(0x84000011U)
#define PSCI_STAT_COUNT_AARCH64		(0xc4000011U)
#define PSCI_SYSTEM_RESET2_AARCH32	(0x84000012U)
#define PSCI_SYSTEM_RESET2_AARCH64	(0xc4000012U)
#define PSCI_MEM_PROTECT		(0x84000013U)
#define PSCI_MEM_CHK_RANGE_AARCH32	(0x84000014U)
#define PSCI_MEM_CHK_RANGE_AARCH64	(0xc4000014U)

/*
 * Number of PSCI calls (above) implemented
 */
#if ENABLE_PSCI_STAT
#define PSCI_NUM_CALLS			(22U)
#else
#define PSCI_NUM_CALLS			(18U)
#endif

/* The macros below are used to identify PSCI calls from the SMC function ID */
#define PSCI_FID_MASK			(0xffe0U)
#define PSCI_FID_VALUE			(0U)
#define is_psci_fid(_fid) \
	(((_fid) & PSCI_FID_MASK) == PSCI_FID_VALUE)

/*******************************************************************************
 * PSCI Migrate and friends
 ******************************************************************************/
#define PSCI_TOS_UP_MIG_CAP	(0U)
#define PSCI_TOS_NOT_UP_MIG_CAP	(1U)
#define PSCI_TOS_NOT_PRESENT_MP	(2U)

/*******************************************************************************
 * PSCI CPU_SUSPEND 'power_state' parameter specific defines
 ******************************************************************************/
#define PSTATE_ID_SHIFT		(0U)

#if PSCI_EXTENDED_STATE_ID
#define PSTATE_VALID_MASK	(0xB0000000U)
#define PSTATE_TYPE_SHIFT	(30U)
#define PSTATE_ID_MASK		(0xfffffffU)
#else
#define PSTATE_VALID_MASK	(0xFCFE0000U)
#define PSTATE_TYPE_SHIFT	(16U)
#define PSTATE_PWR_LVL_SHIFT	(24U)
#define PSTATE_ID_MASK		(0xffffU)
#define PSTATE_PWR_LVL_MASK	(0x3U)

#define psci_get_pstate_pwrlvl(pstate)	(((pstate) >> PSTATE_PWR_LVL_SHIFT) & \
					PSTATE_PWR_LVL_MASK)
#define psci_make_powerstate(state_id, type, pwrlvl) \
			(((state_id) & PSTATE_ID_MASK) << PSTATE_ID_SHIFT) |\
			(((type) & PSTATE_TYPE_MASK) << PSTATE_TYPE_SHIFT) |\
			(((pwrlvl) & PSTATE_PWR_LVL_MASK) << PSTATE_PWR_LVL_SHIFT)
#endif /* __PSCI_EXTENDED_STATE_ID__ */

#define PSTATE_TYPE_STANDBY	(0x0U)
#define PSTATE_TYPE_POWERDOWN	(0x1U)
#define PSTATE_TYPE_MASK	(0x1U)

/*******************************************************************************
 * PSCI CPU_FEATURES feature flag specific defines
 ******************************************************************************/
/* Features flags for CPU SUSPEND power state parameter format. Bits [1:1] */
#define FF_PSTATE_SHIFT		(1U)
#define FF_PSTATE_ORIG		(0U)
#define FF_PSTATE_EXTENDED	(1U)
#if PSCI_EXTENDED_STATE_ID
#define FF_PSTATE		FF_PSTATE_EXTENDED
#else
#define FF_PSTATE		FF_PSTATE_ORIG
#endif

/* Features flags for CPU SUSPEND OS Initiated mode support. Bits [0:0] */
#define FF_MODE_SUPPORT_SHIFT		(0U)
#define FF_SUPPORTS_OS_INIT_MODE	(1U)

/*******************************************************************************
 * PSCI version
 ******************************************************************************/
#define PSCI_MAJOR_VER		((1U) << 16)
#define PSCI_MINOR_VER		(0x1U)

/*******************************************************************************
 * PSCI error codes
 ******************************************************************************/
#define PSCI_E_SUCCESS		0
#define PSCI_E_NOT_SUPPORTED	-1
#define PSCI_E_INVALID_PARAMS	-2
#define PSCI_E_DENIED		-3
#define PSCI_E_ALREADY_ON	-4
#define PSCI_E_ON_PENDING	-5
#define PSCI_E_INTERN_FAIL	-6
#define PSCI_E_NOT_PRESENT	-7
#define PSCI_E_DISABLED		-8
#define PSCI_E_INVALID_ADDRESS	-9

#define PSCI_INVALID_MPIDR	~((u_register_t)0)

/*
 * SYSTEM_RESET2 macros
 */
#define PSCI_RESET2_TYPE_VENDOR_SHIFT	(31U)
#define PSCI_RESET2_TYPE_VENDOR		((1U) << PSCI_RESET2_TYPE_VENDOR_SHIFT)
#define PSCI_RESET2_TYPE_ARCH		((0U) << PSCI_RESET2_TYPE_VENDOR_SHIFT)
#define PSCI_RESET2_SYSTEM_WARM_RESET	(PSCI_RESET2_TYPE_ARCH | (0U))

#ifndef __ASSEMBLER__

#include <stdint.h>

/* Function to help build the psci capabilities bitfield */

static inline unsigned int define_psci_cap(unsigned int x)
{
	return (1U) << (x & U(0x1f));
}


/* Power state helper functions */

static inline unsigned int psci_get_pstate_id(unsigned int power_state)
{
	return ((power_state) >> PSTATE_ID_SHIFT) & PSTATE_ID_MASK;
}

static inline unsigned int psci_get_pstate_type(unsigned int power_state)
{
	return ((power_state) >> PSTATE_TYPE_SHIFT) & PSTATE_TYPE_MASK;
}

static inline unsigned int psci_check_power_state(unsigned int power_state)
{
	return ((power_state) & PSTATE_VALID_MASK);
}

/*
 * These are the states reported by the PSCI_AFFINITY_INFO API for the specified
 * CPU. The definitions of these states can be found in Section 5.7.1 in the
 * PSCI specification (ARM DEN 0022C).
 */
typedef enum {
	AFF_STATE_ON = (0U),
	AFF_STATE_OFF = (1U),
	AFF_STATE_ON_PENDING = (2U)
} aff_info_state_t;

/*
 * These are the power states reported by PSCI_NODE_HW_STATE API for the
 * specified CPU. The definitions of these states can be found in Section 5.15.3
 * of PSCI specification (ARM DEN 0022C).
 */
#define HW_ON		0
#define HW_OFF		1
#define HW_STANDBY	2

/*
 * Macro to represent invalid affinity level within PSCI.
 */
#define PSCI_INVALID_PWR_LVL	(PLAT_MAX_PWR_LVL + (1U))

/*
 * Type for representing the local power state at a particular level.
 */
typedef uint8_t plat_local_state_t;

/* The local state macro used to represent RUN state. */
#define PSCI_LOCAL_STATE_RUN	(0U)

/*
 * Function to test whether the plat_local_state is RUN state
 */
static inline int is_local_state_run(unsigned int plat_local_state)
{
	return (plat_local_state == PSCI_LOCAL_STATE_RUN) ? 1 : 0;
}

/*
 * Function to test whether the plat_local_state is RETENTION state
 */
static inline int is_local_state_retn(unsigned int plat_local_state)
{
	return ((plat_local_state > PSCI_LOCAL_STATE_RUN) &&
		(plat_local_state <= PLAT_MAX_RET_STATE)) ? 1 : 0;
}

/*
 * Function to test whether the plat_local_state is OFF state
 */
static inline int is_local_state_off(unsigned int plat_local_state)
{
	return ((plat_local_state > PLAT_MAX_RET_STATE) &&
		(plat_local_state <= PLAT_MAX_OFF_STATE)) ? 1 : 0;
}

/*****************************************************************************
 * This data structure defines the representation of the power state parameter
 * for its exchange between the generic PSCI code and the platform port. For
 * example, it is used by the platform port to specify the requested power
 * states during a power management operation. It is used by the generic code to
 * inform the platform about the target power states that each level should
 * enter.
 ****************************************************************************/
typedef struct psci_power_state {
	/*
	 * The pwr_domain_state[] stores the local power state at each level
	 * for the CPU.
	 */
	plat_local_state_t pwr_domain_state[PLAT_MAX_PWR_LVL + (1U)];
} psci_power_state_t;

/*******************************************************************************
 * Structure used to store per-cpu information relevant to the PSCI service.
 * It is populated in the per-cpu data array. In return we get a guarantee that
 * this information will not reside on a cache line shared with another cpu.
 ******************************************************************************/
typedef struct psci_cpu_data {
	/* State as seen by PSCI Affinity Info API */
	aff_info_state_t aff_info_state;

	/*
	 * Highest power level which takes part in a power management
	 * operation.
	 */
	unsigned int target_pwrlvl;

	/* The local power state of this CPU */
	plat_local_state_t local_state;
} psci_cpu_data_t;

/*******************************************************************************
 * Structure populated by platform specific code to export routines which
 * perform common low level power management functions
 ******************************************************************************/
typedef struct plat_psci_ops {
	void (*cpu_standby)(plat_local_state_t cpu_state);
	int (*pwr_domain_on)(u_register_t mpidr);
	void (*pwr_domain_off)(const psci_power_state_t *target_state);
	void (*pwr_domain_suspend_pwrdown_early)(
				const psci_power_state_t *target_state);
	void (*pwr_domain_suspend)(const psci_power_state_t *target_state);
	void (*pwr_domain_on_finish)(const psci_power_state_t *target_state);
	void (*pwr_domain_on_finish_late)(
				const psci_power_state_t *target_state);
	void (*pwr_domain_suspend_finish)(
				const psci_power_state_t *target_state);
	void __dead2 (*pwr_domain_pwr_down_wfi)(
				const psci_power_state_t *target_state);
	void __dead2 (*system_off)(void);
	void __dead2 (*system_reset)(void);
	int (*validate_power_state)(unsigned int power_state,
				    psci_power_state_t *req_state);
	int (*validate_ns_entrypoint)(uintptr_t ns_entrypoint);
	void (*get_sys_suspend_power_state)(
				    psci_power_state_t *req_state);
	int (*get_pwr_lvl_state_idx)(plat_local_state_t pwr_domain_state,
				    int pwrlvl);
	int (*translate_power_state_by_mpidr)(u_register_t mpidr,
				    unsigned int power_state,
				    psci_power_state_t *output_state);
	int (*get_node_hw_state)(u_register_t mpidr, unsigned int power_level);
	int (*mem_protect_chk)(uintptr_t base, u_register_t length);
	int (*read_mem_protect)(int *val);
	int (*write_mem_protect)(int val);
	int (*system_reset2)(int is_vendor,
				int reset_type, u_register_t cookie);
} plat_psci_ops_t;

/*******************************************************************************
 * Function & Data prototypes
 ******************************************************************************/
unsigned int psci_version(void);
int psci_cpu_on(u_register_t target_cpu,
		uintptr_t entrypoint,
		u_register_t context_id);
int psci_cpu_suspend(unsigned int power_state,
		     uintptr_t entrypoint,
		     u_register_t context_id);
int psci_system_suspend(uintptr_t entrypoint, u_register_t context_id);
int psci_cpu_off(void);
int psci_affinity_info(u_register_t target_affinity,
		       unsigned int lowest_affinity_level);
int psci_migrate(u_register_t target_cpu);
int psci_migrate_info_type(void);
u_register_t psci_migrate_info_up_cpu(void);
int psci_node_hw_state(u_register_t target_cpu,
		       unsigned int power_level);
int psci_features(unsigned int psci_fid);
void __dead2 psci_power_down_wfi(void);
void psci_arch_setup(void);

#endif /*__ASSEMBLER__*/

#endif /* PSCI_H */
