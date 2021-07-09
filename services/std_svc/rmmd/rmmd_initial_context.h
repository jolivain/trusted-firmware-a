/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMMD_INITIAL_CONTEXT_H
#define RMMD_INITIAL_CONTEXT_H

#include <arch.h>

#define NS_SCTLR_EL2		NS_SCTLR_EL2_RES1

#define NS_HCR_EL2		(					\
					HCR_API_BIT			| \
					HCR_APK_BIT			| \
					HCR_RW_BIT			\
				)
/*
 * SPSR_EL2
 *   M=0x9 (0b1001 EL2h)
 *   M[4]=0
 *   DAIF=0xF Exceptions masked on entry.
 *   BTYPE=0  BTI not yet supported.
 *   SSBS=0   Not yet supported.
 *   IL=0     Not an illegal exception return.
 *   SS=0     Not single stepping.
 *   PAN=1    RMM shouldn't access realm memory.
 *   UAO=0
 *   DIT=0
 *   TCO=0
 *   NZCV=0
 */
#define REALM_SPSR_EL2		(					\
					SPSR_M_EL2H			| \
					(0xF << SPSR_DAIF_SHIFT)	| \
					SPSR_PAN_BIT			\
				)
#define REALM_TRFCR_EL2	0x0

#define REALM_TTBR0_EL2	0x0

#endif /* RMMD_INITIAL_CONTEXT_H */
