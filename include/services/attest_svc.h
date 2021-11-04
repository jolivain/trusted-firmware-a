/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ATTEST_SVC_H
#define ATTEST_SVC_H

#include <lib/smccc.h>

/*
 * The macros below are used to identify Attestation calls from the SMC
 * function ID.
 */
#define ATTEST_FNUM_MIN_VALUE	U(0x1B2)
#define ATTEST_FNUM_MAX_VALUE	U(0x1B3)
#define is_attest_fid(fid) __extension__ ({			\
	__typeof__(fid) _fid = (fid);				\
	((GET_SMC_NUM(_fid) >= ATTEST_FNUM_MIN_VALUE) &&	\
	 (GET_SMC_NUM(_fid) <= ATTEST_FNUM_MAX_VALUE) &&	\
	 (GET_SMC_TYPE(_fid) == SMC_TYPE_FAST)	   &&		\
	 (GET_SMC_CC(_fid) == SMC_64)              &&		\
	 (GET_SMC_OEN(_fid) == OEN_STD_START)      &&		\
	 ((_fid & 0x00FE0000) == 0U)); })

/* Get Attestation fastcall std FID from function number */
#define ATTEST_FID(smc_cc, func_num)			\
	((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT)	|	\
	 ((smc_cc) << FUNCID_CC_SHIFT)		|	\
	 (OEN_STD_START << FUNCID_OEN_SHIFT)	|	\
	 ((func_num) << FUNCID_NUM_SHIFT))

#define ATTEST_FNUM_GET_REALM_KEY	U(0x1B2)
#define ATTEST_FNUM_GET_PLAT_TOKEN	U(0x1B3)

/*
 * arg0  == Function ID.
 * arg1  == Platform attestation token buffer Physical address. (The challenge
 *          object is passed in this buffer.)
 * arg2  == Platform attestation token buffer size.
 * arg3  == Challenge object size.
 * ret0  == Status / error.
 * ret1  == Size of the platform token.
 *
 */
#define ATTEST_GET_REALM_KEY	ATTEST_FID(SMC_64, ATTEST_FNUM_GET_REALM_KEY)
#define ATTEST_GET_PLAT_TOKEN	ATTEST_FID(SMC_64, ATTEST_FNUM_GET_PLAT_TOKEN)

#endif /* ATTEST_SVC_H */
