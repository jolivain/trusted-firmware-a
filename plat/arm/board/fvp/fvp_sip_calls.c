/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <errno.h>

#include <common/debug.h>
#include <common/runtime_svc.h>

#include <plat/arm/common/arm_sip_svc.h>
#include <plat/common/platform.h>

#if ENABLE_RME && SPMD_SPM_AT_SEL2
#include <lib/gpt_rme/gpt_rme.h>
#endif

#if ENABLE_SPMD_LP
#include <services/el3_spmd_logical_sp.h>
#endif


#if (ENABLE_RME == 1) && (defined(SPD_spmd) && SPMD_SPM_AT_SEL2 == 1)
static uint64_t plat_protect_memory(bool protect,
				    bool secure_origin,
				    const uint64_t base,
				    const size_t size,
				    void* handle)
{
	uint64_t ret = 0;
	uint64_t last_updated = 0;

	if (!secure_origin) {
		SMC_RET1(handle, SMC_UNK);
	}

	/*
	 * If protect is true, add memory to secure PAS.
	 * Else unprotect it, making part of non-secure PAS.
	 */
	ret = protect
		? gpt_delegate_pas(base, size, SMC_FROM_SECURE, &last_updated)
		: gpt_undelegate_pas(base, size, SMC_FROM_SECURE, &last_updated);

	switch (ret) {
		case -EINVAL:
			SMC_RET1(handle, SMC_UNK);
		case -EPERM:
			SMC_RET2(handle, SMC_DENIED, last_updated);
		case 0:
			break;
		default:
			ERROR("Unexpected return\n");
	}

	SMC_RET1(handle, SMC_OK);
}
#endif /* ENABLE_RME  && SPMD_SPM_AT_SEL2 */

uintptr_t arm_plat_sip_handler(uint32_t smc_fid,
				u_register_t x1,
				u_register_t x2,
				u_register_t x3,
				u_register_t x4,
				void *cookie,
				void *handle,
				u_register_t flags)
{
	bool secure_origin;

	/* Determine which security state this SMC originated from */
	secure_origin = is_caller_secure(flags);

	switch (smc_fid) {
#if PLAT_TEST_SPM
	case FVP_SIP_SET_INTERRUPT_PENDING:
		if (!secure_origin) {
			SMC_RET1(handle, SMC_UNK);
		}

		VERBOSE("SiP Call- Set interrupt pending %d\n", (uint32_t)x1);
		plat_ic_set_interrupt_pending(x1);

		SMC_RET1(handle, SMC_OK);
		break; /* Not reached */
#endif

#if (ENABLE_RME == 1) && (defined(SPD_spmd) && SPMD_SPM_AT_SEL2 == 1)
	case PLAT_PROTECT_MEM_SMC64:
		/*
		 * TODO: Conflate both, and use flags bitfield to determine whether protect or not.
		 */
		NOTICE("Sip Call - Protect memory\n");
		return plat_protect_memory(true, secure_origin, x1, x2, handle);
		break;
	case PLAT_UNPROTECT_MEM_SMC64:
		NOTICE("Sip Call - Unprotect memory\n");
		return plat_protect_memory(false, secure_origin, x1, x2, handle);
		break;
#endif
	}

#if ENABLE_SPMD_LP
	return plat_spmd_logical_sp_smc_handler(smc_fid, x1, x2, x3, x4,
				cookie, handle, flags);
#else
	WARN("Unimplemented ARM SiP Service Call: 0x%x\n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
#endif
}
