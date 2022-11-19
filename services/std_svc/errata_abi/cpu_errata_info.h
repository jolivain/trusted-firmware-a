/*
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRATA_CPUSPEC_H
#define ERRATA_CPUSPEC_H

#include <stdio.h>
#include <stdint.h>

#define MAX_SIZE  		15
#define UNDEF_ERRATA		UINT_MAX

#define EXTRACT_REV_VAR(x)	((x & MIDR_REV_MASK) << MIDR_REV_BITS) \
				 | ((x >> MIDR_VAR_SHIFT) & MIDR_VAR_MASK)

#define CORTEX_A9_MIDR		U(0x410FC090)
#define CORTEX_A510_MIDR	U(0x410FD460)
#define CORTEX_A710_MIDR	U(0x410FD470)
#define CORTEX_A15_MIDR		U(0x410FC0F0)
#define CORTEX_A17_MIDR		U(0x410FC0E0)
#define CORTEX_A35_MIDR		U(0x410FD040)
#define CORTEX_A53_MIDR 	U(0x410FD030)
#define CORTEX_A55_MIDR 	U(0x410fd050)
#define CORTEX_A57_MIDR 	U(0x410FD070)
#define CORTEX_A72_MIDR 	U(0x410FD080)
#define CORTEX_A73_MIDR		U(0x410FD090)
#define CORTEX_A75_MIDR         U(0x410fd0a0)
#define CORTEX_A76_MIDR 	U(0x410fd0b0)
#define CORTEX_A77_MIDR 	U(0x410FD0D0)
#define CORTEX_A78_MIDR 	U(0x410FD410)
#define CORTEX_A78_AE_MIDR 	U(0x410FD420)
#define CORTEX_A78C_MIDR 	U(0x410FD4B1)
#define CORTEX_X1_MIDR 		U(0x411fd440)
#define NEOVERSE_N1_MIDR 	U(0x410fd0c0)
#define NEOVERSE_V1_MIDR 	U(0x410FD400)
#define NEOVERSE_N2_MIDR 	U(0x410FD490)
#define CORTEX_X2_MIDR 		U(0x410FD480)
#define NEOVERSE_V2_MIDR 	U(0x410FD4F0)
#define CORTEX_MAKALU_MIDR 	U(0x410FD4D0)

/*
 * CPU specific values for errata handling
 */

struct em_cpu{
 	unsigned int em_errata_id;
 	unsigned char em_rxpx_lo;		/* lowest revision of errata applicable for the cpu */
	unsigned char em_rxpx_hi;		/* highest revision of errata applicable for the cpu */
	unsigned char hardware_mitigated;	/* indicator for hardware mitigation revision */
	bool hw_flag;
};

struct em_cpu_list{
 	unsigned long cpu_partnumber;		/* field to hold cpu specific part number defined in midr reg */
 	struct   em_cpu cpu_errata_list[MAX_SIZE];
};

int32_t verify_errata_implemented(uint32_t errata_id, uint32_t forward_flag,
				  void *handle);
#endif /* ERRATA_CPUSPEC_H */
