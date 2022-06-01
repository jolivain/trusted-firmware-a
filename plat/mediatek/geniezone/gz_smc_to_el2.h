/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GZ_SMC_TO_EL2_H
#define GZ_SMC_TO_EL2_H

/* required: tlkd function */
uint64_t tlkd_enter_sp(uint64_t *c_rt_ctx);
void __dead2 tlkd_exit_sp(uint64_t c_rt_ctx, uint64_t ret);

/* smc to el2 */
uint64_t sip_smc_to_el2(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			u_register_t x3, u_register_t x4);
uint64_t sip_smc_to_el2_done(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			     u_register_t x3, u_register_t x4);

#endif /* GZ_SMC_TO_EL2_H */
