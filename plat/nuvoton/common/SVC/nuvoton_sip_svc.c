/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2017 Nuvoton Technology Corp.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/pmf/pmf.h>
#include <tools_share/uuid.h>
#include <nuvoton_sip_svc.h>

static int32_t npcm845x_sip_setup(void)
{
	return 0;
}

static uintptr_t npcm845x_sip_handler(unsigned int smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			u_register_t flags)
{
	switch (smc_fid) {
	/* For now, do not support 32 ARCH */
#ifdef NPCM845X_DEBUG
	case NPCM845X_SIP_AARCH32:
		SMC_RET1(handle, npcm845x_kernel_entry_handler(smc_fid, x1, x2, x3, x4));
		break;
#endif /* NPCM845X_DEBUG */

	case NPCM845X_SIP_GET_SOC_INFO:
		SMC_RET1(handle, npcm845x_soc_info_handler(smc_fid, x1, x2, x3));
		break;
	case  NPCM845X_SIP_SRTC:
		return npcm845x_srtc_handler(smc_fid, handle, x1, x2, x3, x4);
	case  NPCM845X_SIP_CPUFREQ:
		SMC_RET1(handle,npcm845x_cpufreq_handler(smc_fid, x1, x2, x3));
		break;
	case  NPCM845X_SIP_WAKEUP_SRC:
		SMC_RET1(handle, npcm845x_wakeup_src_handler(smc_fid, x1, x2, x3));
	case NPCM845X_SIP_OTP_READ:
	case NPCM845X_SIP_OTP_WRITE:
		return npcm845x_otp_handler(smc_fid, handle, x1, x2);
	case NPCM845X_SIP_MISC_SET_TEMP:
		SMC_RET1(handle, npcm845x_misc_set_temp_handler(smc_fid, x1, x2, x3, x4));
	case  NPCM845X_SIP_BUILDINFO:
		SMC_RET1(handle, npcm845x_buildinfo_handler(smc_fid, x1, x2, x3, x4));
	default:
		WARN("Unimplemented npcm845x SiP Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break;
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
		OEN_SIP_START,
		OEN_SIP_END,
		SMC_TYPE_FAST,
		npcm845x_sip_setup,
		npcm845x_sip_handler
);
