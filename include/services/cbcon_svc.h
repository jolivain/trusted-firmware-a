/*
 * Copyright (c) 2023, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Simple service that provides the address of the cbmem console struct.
 */

#ifndef CBCON_SVC_H
#define CBCON_SVC_H

#include <stdint.h>

#include <lib/smccc.h>

/*
 * SMC function IDs for CBCON Service
 * Upper word bits set: Fast call, SMC64, Standard Secure Svc. Call (OEN = 4)
 */
#define CBCON_FID(func_num)				\
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) |		\
	(SMC_64 << FUNCID_CC_SHIFT) |			\
	(OEN_STD_START << FUNCID_OEN_SHIFT) |		\
	((func_num) << FUNCID_NUM_SHIFT))

/* SMC function IDs for CBMEM Console queries */
#define CBCON_FNUM_GET_ADDR	U(0x190)

#define ARM_CBCON_GET_ADDR	CBCON_FID(CBCON_FNUM_GET_ADDR)

/* CBCON Error Numbers */
#define CBCON_E_SUCCESS		(0)
#define CBCON_E_NOT_IMPLEMENTED	(-1)

/* Public API to verify function id is part of CBCON */
bool is_cbcon_fid(uint32_t smc_fid);

/* Handler to be called to handle CBCON smc calls */
uintptr_t cbcon_smc_handler(
	uint32_t smc_fid,
	u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *cookie,
	void *handle,
	u_register_t flags
);

#endif /* CBCON_SVC_H */
