/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/debugfs/io.h>
#include <lib/pmf/pmf.h>
#include <plat/arm/common/arm_sip_svc.h>
#include <plat/arm/common/plat_arm.h>
#include <tools_share/uuid.h>

/* ARM SiP Service UUID */
DEFINE_SVC_UUID2(arm_sip_svc_uid,
	0x556d75e2, 0x6033, 0xb54b, 0xb5, 0x75,
	0x62, 0x79, 0xfd, 0x11, 0x37, 0xff);

#if USE_DEBUGFS

#define MAX_PATH_LEN	256

#define MOUNT		0
#define CREATE		1
#define OPEN		2
#define CLOSE		3
#define READ		4
#define WRITE		5
#define SEEK		6
#define BIND		7
#define STAT		8
#define INIT		10

/* This is the virtual address to which we map the NS shared buffer */
#define DEBUGFS_SHARED_BUF_VIRT		((void *)0x81000000U)

static union debugfs_parms {
	struct {
		char fname[MAX_PATH_LEN];
	} open;

	struct {
		char srv[MAX_PATH_LEN];
		char where[MAX_PATH_LEN];
		char spec[MAX_PATH_LEN];
	} mount;

	struct {
		char path[MAX_PATH_LEN];
		dir_t dir;
	} stat;

	struct {
		char oldpath[MAX_PATH_LEN];
		char newpath[MAX_PATH_LEN];
	} bind;
} parms;

/* debugfs_access_lock protects shared buffer and internal */
/* FS functions from concurrent acccesses.                 */
static spinlock_t debugfs_access_lock;

static bool debugfs_initialized;

static int64_t debugfs_smc_handler(unsigned int cmd, uint64_t arg2,
				   uint64_t arg3, uint64_t arg4)
{
	int64_t ret = -1;

	spin_lock(&debugfs_access_lock);

	if (debugfs_initialized == true) {
		/* Copy NS shared buffer to internal secure location */
		memcpy(&parms, (void *)DEBUGFS_SHARED_BUF_VIRT,
		       sizeof(union debugfs_parms));
	}

	switch (cmd) {
	case INIT:
		if (debugfs_initialized == false) {
			/* TODO: check PA consistency, whether it is NS area */
			ret = mmap_add_dynamic_region(arg2,
				(uintptr_t)DEBUGFS_SHARED_BUF_VIRT,
				PAGE_SIZE_4KB,
				MT_MEMORY | MT_RW | MT_NS);
			if (ret == 0) {
				debugfs_initialized = true;
			}
		}
		break;

	case MOUNT:
		ret = mount(parms.mount.srv,
			    parms.mount.where,
			    parms.mount.spec);
		break;
	case OPEN:
		ret = open(parms.open.fname, arg2);
		break;
	case CLOSE:
		ret = close(arg2);
		break;
	case READ:
		ret = read(arg2, DEBUGFS_SHARED_BUF_VIRT, arg3);
		break;
	case SEEK:
		ret = seek(arg2, arg3, arg4);
		break;
	case BIND:
		ret = bind(parms.bind.oldpath, parms.bind.newpath);
		break;
	case STAT:
		ret = stat(parms.stat.path, &parms.stat.dir);
		memcpy((void *)DEBUGFS_SHARED_BUF_VIRT, &parms,
		       sizeof(union debugfs_parms));
		break;

	/* Not implemented */
	case CREATE:
		/* Intentional fall-through */

	/* Not implemented */
	case WRITE:
		/* Intentional fall-through */

	default:
		ret = SMC_UNK;
	}

	spin_unlock(&debugfs_access_lock);

	return ret;
}

#endif /* USE_DEBUGFS */

static int arm_sip_setup(void)
{
	if (pmf_setup() != 0)
		return 1;
	return 0;
}

/*
 * This function handles ARM defined SiP Calls
 */
static uintptr_t arm_sip_handler(unsigned int smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			u_register_t flags)
{
	int call_count = 0;

	/*
	 * Dispatch PMF calls to PMF SMC handler and return its return
	 * value
	 */
	if (is_pmf_fid(smc_fid)) {
		return pmf_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
				handle, flags);
	}

	switch (smc_fid) {
	case ARM_SIP_SVC_EXE_STATE_SWITCH: {
		u_register_t pc;

		/* Allow calls from non-secure only */
		if (!is_caller_non_secure(flags))
			SMC_RET1(handle, STATE_SW_E_DENIED);

		/* Validate supplied entry point */
		pc = (u_register_t) ((x1 << 32) | (uint32_t) x2);
		if (arm_validate_ns_entrypoint(pc) != 0)
			SMC_RET1(handle, STATE_SW_E_PARAM);

		/*
		 * Pointers used in execution state switch are all 32 bits wide
		 */
		return (uintptr_t) arm_execution_state_switch(smc_fid,
				(uint32_t) x1, (uint32_t) x2, (uint32_t) x3,
				(uint32_t) x4, handle);
		}
		break; /* satisfy checkpatch */

#if USE_DEBUGFS

	case ARM_SIP_SVC_DEBUGFS: {
		int64_t ret;

		/* Allow calls from non-secure only */
		if (!is_caller_non_secure(flags)) {
			SMC_RET1(handle, STATE_SW_E_DENIED);
		}

		ret = debugfs_smc_handler(x1, x2, x3, x4);
		SMC_RET1(handle, ret);
		}

#endif /* USE_DEBUGFS */

	case ARM_SIP_SVC_CALL_COUNT:
		/* PMF calls */
		call_count += PMF_NUM_SMC_CALLS;

		/* State switch call */
		call_count += 1;

		SMC_RET1(handle, call_count);

	case ARM_SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, arm_sip_svc_uid);

	case ARM_SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, ARM_SIP_SVC_VERSION_MAJOR, ARM_SIP_SVC_VERSION_MINOR);

	default:
		WARN("Unimplemented ARM SiP Service Call: 0x%x \n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

}


/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	arm_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	arm_sip_setup,
	arm_sip_handler
);
