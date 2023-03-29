/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRATA_CPUSPEC_H
#define ERRATA_CPUSPEC_H

#include <arch_features.h>
#include <arch_helpers.h>
#include <stdint.h>
#include <stdio.h>

#if __aarch64__
	#include <cortex_a35.h>
	#include <cortex_a510.h>
	#include <cortex_a53.h>
	#include <cortex_a57.h>
	#include <cortex_a55.h>
	#include <cortex_a710.h>
	#include <cortex_a72.h>
	#include <cortex_a73.h>
	#include <cortex_a75.h>
	#include <cortex_a76.h>
	#include <cortex_a77.h>
	#include <cortex_a78.h>
	#include <cortex_a78_ae.h>
	#include <cortex_a78c.h>
	#include <cortex_makalu.h>
	#include <cortex_x1.h>
	#include <cortex_x2.h>
	#include <neoverse_n1.h>
	#include <neoverse_n2.h>
	#include <neoverse_v1.h>
	#include <neoverse_v2.h>

#else
	#include <cortex_a15.h>
	#include <cortex_a17.h>
	#include <cortex_a57.h>
	#include <cortex_a9.h>
#endif

#define MAX_SIZE		16
#define UNDEF_ERRATA		UINT_MAX

#define EXTRACT_REV_VAR(x)	((x & MIDR_REV_MASK) << MIDR_REV_BITS) \
				 | ((x >> MIDR_VAR_SHIFT) & MIDR_VAR_MASK)
/*
 * CPU specific values for errata handling
 */

struct em_cpu{
	unsigned int em_errata_id;
	unsigned char em_rxpx_lo;		/* lowest revision of errata applicable for the cpu */
	unsigned char em_rxpx_hi;		/* highest revision of errata applicable for the cpu */
	unsigned char hardware_mitigated;	/* indicator for hardware mitigation revision */
	bool hw_flag;
	bool arm_interconnect;			/* interconnect flag */
	bool platform_affected;			/* indicate if platform affected */
};

struct em_cpu_list{
	unsigned long cpu_partnumber;	/* field to hold cpu specific part number defined in midr reg */
	struct   em_cpu cpu_errata_list[MAX_SIZE];
};

int32_t verify_errata_implemented(uint32_t errata_id, uint32_t forward_flag,
					void *handle);
#endif /* ERRATA_CPUSPEC_H */
