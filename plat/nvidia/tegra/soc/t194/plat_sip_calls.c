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

extern bool tegra_fake_system_suspend;

/*******************************************************************************
 * Tegra186 SiP SMCs
 ******************************************************************************/
#define TEGRA_SIP_ENABLE_FAKE_SYSTEM_SUSPEND		0xC2FFFE03U

/*******************************************************************************
 * This function is responsible for handling all T186 SiP calls
 ******************************************************************************/
int32_t plat_sip_handler(uint32_t smc_fid,
		     uint64_t x1,
		     uint64_t x2,
		     uint64_t x3,
		     uint64_t x4,
		     const void *cookie,
		     void *handle,
		     uint64_t flags)
{
	int32_t ret = -ENOTSUP;

	(void)x1;
	(void)x4;
	(void)cookie;
	(void)flags;

	if (smc_fid == TEGRA_SIP_ENABLE_FAKE_SYSTEM_SUSPEND) {
		/*
		 * System suspend mode is set if the platform ATF is
		 * running on VDK and there is a debug SIP call. This mode
		 * ensures that the debug path is exercised, instead of
		 * regular code path to suit the pre-silicon platform needs.
		 * This includes replacing the call to WFI, with calls to
		 * system suspend exit procedures.
		 */
		if (tegra_platform_is_virt_dev_kit()) {
			tegra_fake_system_suspend = true;
			ret = 0;
		}
	}

	return ret;
}
