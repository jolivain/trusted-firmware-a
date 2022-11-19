/*
 * Copyright (c) 2022-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <arch_features.h>
#include <arch/aarch64/arch_helpers.h>
#include <lib/smccc.h>
#include <lib/utils_def.h>
#include <smccc_helpers.h>
#include <services/errata_abi_svc.h>
#include <smccc_helpers.h>
#include "cpu_errata_info.h"

/* Global pointer that points to the specific structure based on the MIDR part number */
struct em_cpu_list *cpu_ptr = NULL;

/* Structure array that holds CPU specific errata information */
struct em_cpu_list cpu_list[] = {
#if CORTEX_A9_H_INC
{
    .cpu_pn = CORTEX_A9_MIDR,
    .cpu_errata_list = {
	{794073, 0x00, 0xFF},
	{UINT_MAX},{UINT_MAX},
	{UINT_MAX},{UINT_MAX},
	{UINT_MAX},{UINT_MAX},
	{UINT_MAX},{UINT_MAX},
	{UINT_MAX},{UINT_MAX},
	{UINT_MAX},{UINT_MAX},
	{UINT_MAX},{UINT_MAX}
    }
},
#endif

#if CORTEX_A15_H_INC
{
    .cpu_pn = CORTEX_A15_MIDR,
    .cpu_errata_list = {
	{816470, 0x30, 0xFF},
	{827671, 0x30, 0xFF},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
    }
},
#endif

#if CORTEX_A17_H_INC
{
    .cpu_pn = CORTEX_A17_MIDR,
    .cpu_errata_list = {
	{852421, 0x00, 0x12},
	{852423, 0x00, 0x12},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
    }
},
#endif

#if CORTEX_A35_H_INC
{
    .cpu_pn = CORTEX_A35_MIDR,
    .cpu_errata_list = {
	{855472, 0x00, 0x00},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX}
    }
},
#endif

#if CORTEX_A53_H_INC
{
    .cpu_pn = CORTEX_A53_MIDR,
    .cpu_errata_list = {
	{819472, 0x00, 0x01},
	{824069, 0x00, 0x02},
	{826319, 0x00, 0x02},
	{827319, 0x00, 0x02},
	{835769, 0x00, 0x04},
	{836870, 0x00, 0x03, 0x04, true},
	{843419, 0x00, 0x04},
	{855873, 0x03, 0xFF},
	{1530924, 0x00, 0xFF},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX}
    }
},
#endif

#if CORTEX_A55_H_INC
{
    .cpu_pn = CORTEX_A55_MIDR,
    .cpu_errata_list = {
	{768277, 0x00, 0x00},
	{778703, 0x00, 0x00},
	{798797, 0x00, 0x00},
	{846532, 0x00, 0x01},
	{903758, 0x00, 0x01},
	{1221012, 0x00, 0x10},
	{1530923, 0x00, 0xFF},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX}
    }
},
#endif

#if CORTEX_A57_H_INC
{
    .cpu_pn = CORTEX_A57_MIDR,
    .cpu_errata_list = {
	{806969, 0x00, 0x00},
	{813419, 0x00, 0x00},
	{813420, 0x00, 0x00},
	{814670, 0x00, 0x00},
	{817169, 0x00, 0x01},
	{826974, 0x00, 0x11},
	{826977, 0x00, 0x11},
	{828024, 0x00, 0x11},
	{829520, 0x00, 0x12},
	{833471, 0x00, 0x12},
	{859972, 0x00, 0x13},
	{1319537, 0x00, 0xFF},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
    }
},
#endif

#if CORTEX_A72_H_INC
{
    .cpu_pn = CORTEX_A72_MIDR,
    .cpu_errata_list = {
	{859971, 0x00, 0x03},
	{1319367, 0x00, 0xFF},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
    }
},
#endif

#if CORTEX_A73_H_INC
{
    .cpu_pn = CORTEX_A73_MIDR,
    .cpu_errata_list = {
	{852427, 0x00, 0x00},
	{855423, 0x00, 0x01},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
    }
},
#endif

#if CORTEX_A75_H_INC
{
    .cpu_pn = CORTEX_A75_MIDR,
    .cpu_errata_list = {
	{764081, 0x00, 0x00},
	{790748, 0x00, 0x00},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
    }
},
#endif

#if CORTEX_A76_H_INC
{
    .cpu_pn = CORTEX_A76_MIDR,
    .cpu_errata_list = {
	{1073348, 0x00, 0x10},
	{1130799, 0x00, 0x20},
	{1165522, 0x00, 0xFF},
	{1220197, 0x00, 0x20},
	{1257314, 0x00, 0x30},
	{1262606, 0x00, 0x30},
	{1262888, 0x00, 0x30},
	{1275112, 0x00, 0x30},
	{1791580, 0x00, 0x40},
	{1868343, 0x00, 0x40},
	{1946160, 0x30, 0x41},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX}
    }
},
#endif

#if CORTEX_A77_H_INC
{
    .cpu_pn = CORTEX_A77_MIDR,
    .cpu_errata_list = {
	{1508412, 0x00, 0x10},
	{1791578, 0x00, 0x11},
	{1925769, 0x00, 0x11},
	{1946167, 0x00, 0x11},
	{2356587, 0x00, 0x11},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX}
    }
},
#endif

#if CORTEX_A78_H_INC
{
    .cpu_pn = CORTEX_A78_MIDR,
    .cpu_errata_list = {
	{1688305, 0x00, 0x10},
	{1821534, 0x00, 0x10},
	{1941498, 0x00, 0x11},
	{1951500, 0x10, 0x11},
	{1952683, 0x00, 0x10},
	{2132060, 0x00, 0x12},
	{2242635, 0x10, 0x12},
	{2376745, 0x00, 0x12},
	{2395406, 0x00, 0x12},
	#if ERRATA_NON_ARM_INTERCONNECT
	{2712571, 0x00, 0x12, 0x00, false, \
	ERRATA_NON_ARM_INTERCONNECT, ERRATA_A78_2712571},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
	#else
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX}
	#endif
    }
},
#endif

#if CORTEX_A78_AE_H_INC
{
    .cpu_pn = CORTEX_A78_AE_MIDR,
    .cpu_errata_list = {
	{1941500, 0x00, 0x01},
	{1951502, 0x00, 0x01},
	{2376748, 0x00, 0x01},
	{2395408, 0x00, 0x01},
	#if ERRATA_NON_ARM_INTERCONNECT
	{2712574, 0x00, 0x02, 0x00, false, \
	ERRATA_NON_ARM_INTERCONNECT, ERRATA_A78_AE_2712574},
	{UINT_MAX}, {UINT_MAX},
        {UINT_MAX}, {UINT_MAX},
        {UINT_MAX}, {UINT_MAX},
        {UINT_MAX}, {UINT_MAX},
        {UINT_MAX}, {UINT_MAX},
	#else
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
	#endif
    }
},
#endif

#if CORTEX_A78C_H_INC
{
    .cpu_pn = CORTEX_A78C_MIDR,
    .cpu_errata_list = {
	{2132064, 0x01, 0x02},
	{2242638, 0x01, 0x02},
	{2376749, 0x01, 0x02},
	{2395411, 0x01, 0x02},
	#if ERRATA_NON_ARM_INTERCONNECT
	{2712575, 0x01, 0x02, 0x00, false, \
	ERRATA_NON_ARM_INTERCONNECT, ERRATA_A78C_2712575},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX}
	#else
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
	#endif
    }
},
#endif

#if CORTEX_X1_H_INC
{
    .cpu_pn = CORTEX_X1_MIDR,
    .cpu_errata_list = {
	{1688305, 0x00, 0x10},
	{1821534, 0x00, 0x10},
	{1827429, 0x00, 0x10},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX}
    }
},
#endif

#if NEOVERSE_N1_H_INC
{
    .cpu_pn = NEOVERSE_N1_MIDR,
    .cpu_errata_list = {
	{1073348, 0x00, 0x10},
	{1130799, 0x00, 0x20},
	{1165347, 0x00, 0x20},
	{1207823, 0x00, 0x20},
	{1220197, 0x00, 0x20},
	{1257314, 0x00, 0x30},
	{1262606, 0x00, 0x30},
	{1262888, 0x00, 0x30},
	{1275112, 0x00, 0x30},
	{1315703, 0x00, 0x30},
	{1542419, 0x30, 0x40},
	{1868343, 0x00, 0x40},
	{1946160, 0x30, 0x41},
	{UINT_MAX}, {UINT_MAX}
    }
},
#endif

#if NEOVERSE_V1_H_INC
{
    .cpu_pn = NEOVERSE_V1_MIDR,
    .cpu_errata_list = {
	{1618635, 0x00, 0x10},
	{1774420, 0x00, 0x11},
	{1791573, 0x00, 0x11},
	{1852267, 0x00, 0x11},
	{1925756, 0x00, 0x11},
	{1940577, 0x10, 0x11},
	{1966096, 0x10, 0x11},
	{2108267, 0x00, 0x11},
	{2139242, 0x00, 0x11},
	{2216392, 0x10, 0x11},
	{2294912, 0x00, 0x11},
	{2372203, 0x00, 0x11},
	#if ERRATA_NON_ARM_INTERCONNECT
	{2701953, 0x00, 0x012, 0x00, false, \
	ERRATA_NON_ARM_INTERCONNECT, ERRATA_V1_2701953},
	{UINT_MAX}, {UINT_MAX}
	#else
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
	#endif
    }
},
#endif

#if CORTEX_A710_H_INC
{
    .cpu_pn = CORTEX_A710_MIDR,
    .cpu_errata_list = {
	{1987031, 0x00, 0x20},
	{2008768, 0x00, 0x21},
	{2017096, 0x00, 0x20},
	{2055002, 0x10, 0x20},
	{2058056, 0x00, 0x10},
	{2081180, 0x00, 0x20},
	{2083908, 0x20, 0x20},
	{2136059, 0x00, 0x21},
	{2147715, 0x20, 0x21},
	{2216384, 0x00, 0x21},
	{2267065, 0x00, 0x21},
	{2282622, 0x00, 0x21},
	{2371105, 0x00, 0x21},
	#if ERRATA_NON_ARM_INTERCONNECT
	{2701952, 0x00, 0x21, 0x00, false, \
	ERRATA_NON_ARM_INTERCONNECT, ERRATA_A710_2701952},
	{UINT_MAX}
	#else
	{UINT_MAX}, {UINT_MAX}
	#endif
    }
},
#endif

#if NEOVERSE_N2_H_INC
{
    .cpu_pn = NEOVERSE_N2_MIDR,
    .cpu_errata_list = {
	{2002655, 0x00, 0x00},
	{2025414, 0x00, 0x00},
	{2067956, 0x00, 0x00},
	{2138953, 0x00, 0x00},
	{2138956, 0x00, 0x00},
	{2138958, 0x00, 0x00},
	{2189731, 0x00, 0x00},
	{2242400, 0x00, 0x00},
	{2242415, 0x00, 0x00},
	{2280757, 0x00, 0x00},
	{2376738, 0x00, 0x01},
	{2388450, 0x00, 0x01},
	#if ERRATA_NON_ARM_INTERCONNECT
	{2728475, 0x00, 0x03, 0x00, false, \
	ERRATA_NON_ARM_INTERCONNECT, ERRATA_N2_2728475},
	{UINT_MAX}, {UINT_MAX}
	#else
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
	#endif
    }
},
#endif

#if CORTEX_X2_H_INC
{
    .cpu_pn = CORTEX_X2_MIDR,
    .cpu_errata_list = {
	{2002765, 0x00, 0x20},
	{2017096, 0x00, 0x21},
	{2058056, 0x00, 0x20},
	{2081180, 0x00, 0x21},
	{2083908, 0x00, 0x20},
	{2147715, 0x20, 0x21},
	{2216384, 0x00, 0x21},
	{2371105, 0x00, 0x21},
	#if ERRATA_NON_ARM_INTERCONNECT
	{2701952, 0x00, 0x21, 0x00, false, \
	ERRATA_NON_ARM_INTERCONNECT, ERRATA_X2_2701952},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX}
	#else
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
	#endif
    }
},
#endif

#if CORTEX_A510_H_INC
{
    .cpu_pn = CORTEX_A510_MIDR,
    .cpu_errata_list = {
	{1922240, 0x00, 0x01},
	{2041909, 0x02, 0x03},
	{2042739, 0x00, 0x03},
	{2172148, 0x00, 0x11},
	{2218950, 0x00, 0x11},
	{2250311, 0x00, 0x11},
	{2288014, 0x00, 0x11},
	{2371937, 0x00, 0x12},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}
    }
},
#endif

#if NEOVERSE_V2_H_INC
{
	.cpu_pn = NEOVERSE_V2_MIDR,
	.cpu_errata_list = {
	#if ERRATA_NON_ARM_INTERCONNECT
	{2719103, 0x00, 0x02, 0x00, false, \
	ERRATA_NON_ARM_INTERCONNECT, ERRATA_V2_2719103},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX},
	{UINT_MAX}, {UINT_MAX}
	#endif
	}
},
#endif

#if CORTEX_A715_H_INC
{
	.cpu_pn = CORTEX_MAKALU_MIDR,
	.cpu_errata_list = {
	#if ERRATA_NON_ARM_INTERCONNECT
	{2701951, 0x00, 0x11, 0x00, false, \
	ERRATA_NON_ARM_INTERCONNECT, ERRATA_A715_2701951},
	{UINT_MAX}, {UINT_MAX},
        {UINT_MAX}, {UINT_MAX},
        {UINT_MAX}, {UINT_MAX},
        {UINT_MAX}, {UINT_MAX},
        {UINT_MAX}, {UINT_MAX},
        {UINT_MAX}, {UINT_MAX},
        {UINT_MAX}, {UINT_MAX}
	#endif
	}
},
#endif
{
}
};

/* Function to do binary search and check for the specific errata ID
 * in the array of structures specific to the cpu identified.
 */
int32_t bin_search(struct em_cpu_list *ptr, uint32_t erratum_id,
			uint8_t rxpx_val)
{
	int low_index = 0U, mid_index = 0U, high_index = 0U;

	high_index = ARRAY_SIZE(ptr->cpu_errata_list); /*Max size = 15*/

	high_index = (high_index > 0) ? (high_index - 1) : high_index;

	while (low_index <= high_index) {
		mid_index = (low_index + high_index) / 2;
		if (erratum_id < ptr->cpu_errata_list[mid_index].em_errata_id) {
			high_index = mid_index - 1;
		} else if (erratum_id > ptr->cpu_errata_list \
						[mid_index].em_errata_id) {
			low_index = mid_index + 1;
		} else {
			if (erratum_id == ptr->cpu_errata_list \
						[mid_index].em_errata_id) {
/* erratum ID is in the list and errata has been fixed in hw specific
				revision and above*/
				if ((ptr->cpu_errata_list[mid_index]. \
				    hardware_mitigated >= rxpx_val) && \
				    (ptr->cpu_errata_list[mid_index]. \
				    hw_flag)) {
					return EM_NOT_AFFECTED;
				}
/* rxpx in range, if non-arm ip and errata flag active, return affected,
if errata flag not active, return unknown erratum */
				else if ((rxpx_val >= ptr->cpu_errata_list \
					   [mid_index].em_rxpx_lo) && \
					   (rxpx_val <= ptr->cpu_errata_list \
					   [mid_index].em_rxpx_hi)) {
						if (ptr->cpu_errata_list \
						   [mid_index].arm_interconnect){
							int ret_val;
							ret_val = (ptr->cpu_errata_list \
							[mid_index].platform_affected \
							== true) ? EM_AFFECTED  \
							: EM_UNKNOWN_ERRATUM;
							return ret_val;
							} else {
							return EM_HIGHER_EL_MITIGATION;
							}
				} else {
					return EM_UNKNOWN_ERRATUM;
				}
			}
		}
	}
	/* no matching errata ID */
	return EM_UNKNOWN_ERRATUM;
}

/* Function to check if the errata exists for the specific CPU and rxpx */
int32_t verify_errata_implemented(uint32_t errata_id,
					 uint32_t forward_flag,
					 void *handle)
{
	/* Read MIDR value and extract the revision
	 * variant and cpu info. */
	static uint32_t midr_val;
	static uint8_t  cpu_rxpx_val;
	int32_t ret_val = EM_UNKNOWN_ERRATUM;

	/* Determine the number of cpu listed in the cpu list */
	uint16_t size_cpulist = ARRAY_SIZE(cpu_list);

	/* Read the midr reg to extract cpu, revision and variant info */
	midr_val = read_midr();

	/* Extract revision and variant from the MIDR register */
	cpu_rxpx_val = ((midr_val & MIDR_REV_MASK) << MIDR_REV_BITS) \
			| ((midr_val >> MIDR_VAR_SHIFT) & MIDR_VAR_MASK);

	/* Extract the midr value and check if the midr is in the cpu list*/
	midr_val = ((midr_val) >> MIDR_PN_SHIFT) &  MIDR_PN_MASK;

	for (uint8_t i = 0; i < size_cpulist; i++) {
		cpu_ptr = &cpu_list[i];
		uint16_t midr_extracted = (cpu_ptr->cpu_pn >> MIDR_PN_SHIFT) & MIDR_PN_MASK;
		if (midr_extracted == midr_val) {
			/* If the midr value is in the cpu list, binary search
			   for the errata ID and specific revision in the list */
			ret_val = bin_search(cpu_ptr, errata_id, cpu_rxpx_val);
			break;
		}
	}
	return ret_val;
}

/* Predicate indicating that a function id is part of EM_ABI */
bool is_errata_fid(uint32_t smc_fid)
{
	return ((smc_fid == ARM_EM_VERSION) ||
		(smc_fid == ARM_EM_FEATURES) ||
		(smc_fid == ARM_EM_CPU_ERRATUM_FEATURES));

}

uintptr_t errata_abi_smc_handler(uint32_t smc_fid, u_register_t x1,
			   u_register_t x2, u_register_t x3, u_register_t x4,
			   void *cookie, void *handle, u_register_t flags)
{
	int32_t ret_id = EM_UNKNOWN_ERRATUM;
	bool pstate_rw;
	uint8_t read_el_state;

	switch (smc_fid) {
	case ARM_EM_VERSION:
		SMC_RET1(handle, MAKE_SMCCC_VERSION(
			EM_VERSION_MAJOR, EM_VERSION_MINOR
		));
		break; /* unreachable */
	case ARM_EM_FEATURES:
		if (is_errata_fid((uint32_t)x1)) {
			SMC_RET1(handle, EM_SUCCESS);
		} else {
			SMC_RET1(handle, EM_NOT_SUPPORTED);
		}
		break; /* unreachable */
	case ARM_EM_CPU_ERRATUM_FEATURES:
	{
		/*If processor is in AARCH64 state and forward flag != 0
		  and calling EL = EL1, return Invalid Parameters */
		pstate_rw = (GET_RW(read_spsr_el3()) == MODE_RW_64) ? true : false;

		if(true == pstate_rw) {
			read_el_state = GET_EL(read_spsr_el3());
			if(((uint32_t)x2 != 0) && \
				(read_el_state == MODE_EL1)) {
				SMC_RET1(handle, EM_INVALID_PARAMETERS);
			} else {
				ret_id = verify_errata_implemented((uint32_t)x1, \
                                                (uint32_t)x2, handle);
				SMC_RET1(handle, ret_id);
			}
		}
		else {
			/*Processor is in AARCH32 state,  */
			read_el_state = GET_EL(read_spsr_el3());
			if((((uint32_t)x2 != 0) && (read_el_state == MODE32_hyp)) \
			   || (((uint32_t)x2 == 0) && (read_el_state != MODE32_hyp))) {
					ret_id = verify_errata_implemented((uint32_t)x1, \
                                                (uint32_t)x2, handle);
					SMC_RET1(handle, ret_id);
			} else {
				SMC_RET1(handle, EM_INVALID_PARAMETERS);
			}
		}
		break; /*unreachable*/
	}
	default:
		{
		   WARN("Unimplemented Errata abi Service Call: 0x%x\n",
			smc_fid);
		   SMC_RET1(handle, EM_UNKNOWN_ERRATUM);
		   break; /* unreachable */
		}
	}
}
