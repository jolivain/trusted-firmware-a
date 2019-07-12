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

static union debugfs_parms {
	struct {
		char fname[MAX_PATH_LEN];
	} open;

	struct mount {
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

#define MOUNT  0
#define CREATE 1
#define OPEN   2
#define CLOSE  3
#define READ   4
#define WRITE  5
#define SEEK   6
#define BIND   7
#define STAT   8

#define ARM_DEBUGFS_ENV_PTR		((void*)0x81000000)

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

#if USE_DEBUGFS

	case ARM_SIP_SVC_DEBUGFS: {
		int64_t ret;

		/* Allow calls from non-secure only */
		if (!is_caller_non_secure(flags)) {
			SMC_RET1(handle, STATE_SW_E_DENIED);
		}

		/* Copy NS shared buffer to internal secure location */
		memcpy(&parms, ARM_DEBUGFS_ENV_PTR,
		       sizeof(union debugfs_parms));

		switch (x1) {
		case MOUNT:
			ret = mount(parms.mount.srv,
				    parms.mount.where,
				    parms.mount.spec);
			break;
		case CREATE:
			/* Not currently implemented */
			ret = -1;
			break;
		case OPEN:
			ret = open(parms.open.fname, x2);
			break;
		case CLOSE:
			ret = close(x2);
			break;
		case READ:
			ret = read(x2, ARM_DEBUGFS_ENV_PTR, x3);
			break;
		case WRITE:
			/* Not implemented */
			ret = -1;
			break;
		case SEEK:
			ret = seek(x2, x3, x4);
			break;
		case BIND:
			ret = bind(parms.bind.oldpath, parms.bind.newpath);
			break;
		case STAT:
			ret = stat(parms.stat.path, &parms.stat.dir);
			memcpy(ARM_DEBUGFS_ENV_PTR, &parms,
			       sizeof(union debugfs_parms));
			break;

		default:
			ret = -1;
		}

		SMC_RET1(handle, ret);
		}

#else

	case ARM_SIP_SVC_DEBUGFS:
		SMC_RET1(handle, -2);
		break;

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
