/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* C lib */

/* TF-A header */
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <smccc_helpers.h>

/* MTK header */
#include <mtk_sip_svc.h>

/* GZ header */
#include "gz_secio.h"
#include "gz_smc_to_el2.h"
#include "mtk_hyp_secio.h"

#if CONFIG_MTK_GZ_SECIO
u_register_t sip_hyp_secio_write(u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *handle,
	struct smccc_res *smccc_ret)
{
	return sip_secio_write(x1, x2, x3, x4, handle);
}
DECLARE_SMC_HANDLER(MTK_SIP_HYP_SECIO_WRITE, sip_hyp_secio_write);

u_register_t sip_hyp_secio_read(u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *handle,
	struct smccc_res *smccc_ret)
{
	uint32_t val = 0;
	uint64_t rc = sip_secio_read(x1, x2, x3, x4, handle, &val);

	smccc_ret->a1 = (uint64_t)val;
	return rc;
}
DECLARE_SMC_HANDLER(MTK_SIP_HYP_SECIO_READ, sip_hyp_secio_read);
#endif /* CONFIG_MTK_GZ_SECIO */

#if CONFIG_MTK_GZ_SMC
u_register_t sip_hyp_el3_eret_done(u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *handle,
	struct smccc_res *smccc_ret)
{
	uint64_t smc_fid = SMC_GET_GP(handle, CTX_GPREG_X0);

	return sip_smc_to_el2_done(smc_fid, x1, x2, x3, x4);
}
DECLARE_SMC_HANDLER(MTK_SIP_HYP_EL3_ERET_DONE, sip_hyp_el3_eret_done);

u_register_t sip_hyp_smc_to_el2(u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *handle,
	struct smccc_res *smccc_ret)
{
	uint64_t smc_fid = SMC_GET_GP(handle, CTX_GPREG_X0);

	return sip_smc_to_el2(smc_fid, x1, x2, x3, x4);
}
DECLARE_SMC_HANDLER(MTK_SIP_HYP_SMC_TO_EL2_RET, sip_hyp_smc_to_el2);
DECLARE_SMC_HANDLER(MTK_SIP_HYP_SMC_TO_EL2_RKP, sip_hyp_smc_to_el2);
DECLARE_SMC_HANDLER(MTK_SIP_HYP_SMC_TO_EL2_TEE, sip_hyp_smc_to_el2);
#endif /* CONFIG_MTK_GZ_SMC */
