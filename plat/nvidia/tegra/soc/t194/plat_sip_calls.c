/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common/bl_common.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <common/debug.h>
#include <errno.h>
#include <mce.h>
#include <memctrl.h>
#include <common/runtime_svc.h>
#include <tegra_private.h>
#include <tegra_platform.h>
#include <stdbool.h>

extern uint32_t tegra186_system_powerdn_state;

extern bool tegra_fake_system_suspend;

/*******************************************************************************
 * Tegra186 SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_SYSTEM_SHUTDOWN_STATE			0xC2FFFE01
#define TEGRA_SIP_GET_ACTMON_CLK_COUNTERS		0xC2FFFE02
#define TEGRA_SIP_ENABLE_FAKE_SYSTEM_SUSPEND		0xC2FFFE03
#define TEGRA_SIP_MCE_CMD_ENTER_CSTATE			0xC2FFFF00
#define TEGRA_SIP_MCE_CMD_UPDATE_CSTATE_INFO		0xC2FFFF01
#define TEGRA_SIP_MCE_CMD_UPDATE_CROSSOVER_TIME		0xC2FFFF02
#define TEGRA_SIP_MCE_CMD_READ_CSTATE_STATS		0xC2FFFF03
#define TEGRA_SIP_MCE_CMD_WRITE_CSTATE_STATS		0xC2FFFF04
#define TEGRA_SIP_MCE_CMD_IS_SC7_ALLOWED		0xC2FFFF05
#define TEGRA_SIP_MCE_CMD_ONLINE_CORE			0xC2FFFF06
#define TEGRA_SIP_MCE_CMD_CC3_CTRL			0xC2FFFF07
#define TEGRA_SIP_MCE_CMD_ECHO_DATA			0xC2FFFF08
#define TEGRA_SIP_MCE_CMD_READ_VERSIONS			0xC2FFFF09
#define TEGRA_SIP_MCE_CMD_ENUM_FEATURES			0xC2FFFF0A
#define TEGRA_SIP_MCE_CMD_ROC_FLUSH_CACHE_TRBITS	0xC2FFFF0B
#define TEGRA_SIP_MCE_CMD_ENUM_READ_MCA			0xC2FFFF0C
#define TEGRA_SIP_MCE_CMD_ENUM_WRITE_MCA		0xC2FFFF0D
#define TEGRA_SIP_MCE_CMD_ROC_FLUSH_CACHE		0xC2FFFF0E
#define TEGRA_SIP_MCE_CMD_ROC_CLEAN_CACHE		0xC2FFFF0F
#define TEGRA_SIP_MCE_CMD_ENABLE_LATIC			0xC2FFFF10
#define TEGRA_SIP_MCE_CMD_UNCORE_PERFMON_REQ		0xC2FFFF11
#define TEGRA_SIP_MCE_CMD_MISC_CCPLEX			0xC2FFFF12

/*******************************************************************************
 * This function is responsible for handling all T186 SiP calls
 ******************************************************************************/
int plat_sip_handler(uint32_t smc_fid,
		     uint64_t x1,
		     uint64_t x2,
		     uint64_t x3,
		     uint64_t x4,
		     const void *cookie,
		     void *handle,
		     uint64_t flags)
{
	int mce_ret;

	/*
	 * Convert SMC FID to SMC64 until the linux driver uses
	 * SMC64 encoding.
	 */
	smc_fid |= (SMC_64 << FUNCID_CC_SHIFT);

	switch (smc_fid) {

	/*
	 * Micro Coded Engine (MCE) commands reside in the 0x82FFFF00 -
	 * 0x82FFFFFF SiP SMC space
	 */
	case TEGRA_SIP_MCE_CMD_ENTER_CSTATE:
	case TEGRA_SIP_MCE_CMD_UPDATE_CSTATE_INFO:
	case TEGRA_SIP_MCE_CMD_UPDATE_CROSSOVER_TIME:
	case TEGRA_SIP_MCE_CMD_READ_CSTATE_STATS:
	case TEGRA_SIP_MCE_CMD_WRITE_CSTATE_STATS:
	case TEGRA_SIP_MCE_CMD_IS_SC7_ALLOWED:
	case TEGRA_SIP_MCE_CMD_CC3_CTRL:
	case TEGRA_SIP_MCE_CMD_ECHO_DATA:
	case TEGRA_SIP_MCE_CMD_READ_VERSIONS:
	case TEGRA_SIP_MCE_CMD_ENUM_FEATURES:
	case TEGRA_SIP_MCE_CMD_ROC_FLUSH_CACHE_TRBITS:
	case TEGRA_SIP_MCE_CMD_ENUM_READ_MCA:
	case TEGRA_SIP_MCE_CMD_ENUM_WRITE_MCA:
	case TEGRA_SIP_MCE_CMD_ROC_FLUSH_CACHE:
	case TEGRA_SIP_MCE_CMD_ROC_CLEAN_CACHE:
	case TEGRA_SIP_MCE_CMD_ENABLE_LATIC:
	case TEGRA_SIP_MCE_CMD_UNCORE_PERFMON_REQ:
	case TEGRA_SIP_MCE_CMD_MISC_CCPLEX:

		/* clean up the high bits */
		smc_fid &= MCE_CMD_MASK;

		/* execute the command and store the result */
		mce_ret = mce_command_handler(smc_fid, x1, x2, x3);
		write_ctx_reg(get_gpregs_ctx(handle), CTX_GPREG_X0, mce_ret);

		return 0;

	case TEGRA_SIP_SYSTEM_SHUTDOWN_STATE:

		/* clean up the high bits */
		x1 = (uint32_t)x1;

		/*
		 * SC8 is a special Tegra186 system state where the CPUs and
		 * DRAM are powered down but the other subsystem is still
		 * alive.
		 */

		return 0;

	case TEGRA_SIP_ENABLE_FAKE_SYSTEM_SUSPEND:
		/*
		 * System suspend mode is set if the platform ATF is running is
		 * VDK and there is a debug SIP call. This mode ensures that the
		 * debug path is excercied, instead of regular code path to suit
		 * the pre-silicon platform needs. These include replacing the
		 * the call to WFI with calls to system suspend exit procedures.
		 */
		if (tegra_platform_is_virt_dev_kit()) {

			tegra_fake_system_suspend = true;
			return 0;
		}

	default:
		break;
	}

	return -ENOTSUP;
}
