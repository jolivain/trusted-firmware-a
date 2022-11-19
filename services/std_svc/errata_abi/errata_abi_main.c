/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include "cpu_errata_info.h"
#include <lib/smccc.h>
#include <lib/utils_def.h>
#include <services/errata_abi_svc.h>
#include <smccc_helpers.h>

/*
 * Global pointer that points to the specific
 * structure based on the MIDR part number
 */
struct em_cpu_list *cpu_ptr;

/* Structure array that holds CPU specific errata information */
struct em_cpu_list cpu_list[] = {
#if CORTEX_A9_H_INC
{
	.cpu_partnumber = CORTEX_A9_MIDR,
	.cpu_errata_list = {
		{794073, 0x00, 0xFF},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A9_H_INC */

#if CORTEX_A15_H_INC
{
	.cpu_partnumber = CORTEX_A15_MIDR,
	.cpu_errata_list = {
		{816470, 0x30, 0xFF},
		{827671, 0x30, 0xFF},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A15_H_INC */

#if CORTEX_A17_H_INC
{
	.cpu_partnumber = CORTEX_A17_MIDR,
	.cpu_errata_list = {
		{852421, 0x00, 0x12},
		{852423, 0x00, 0x12},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A17_H_INC */

#if CORTEX_A35_H_INC
{
	.cpu_partnumber = CORTEX_A35_MIDR,
	.cpu_errata_list = {
		{855472, 0x00, 0x00},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A35_H_INC */

#if CORTEX_A53_H_INC
{
	.cpu_partnumber = CORTEX_A53_MIDR,
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
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A53_H_INC */

#if CORTEX_A55_H_INC
{
	.cpu_partnumber = CORTEX_A55_MIDR,
	.cpu_errata_list = {
		{768277, 0x00, 0x00},
		{778703, 0x00, 0x00},
		{798797, 0x00, 0x00},
		{846532, 0x00, 0x01},
		{903758, 0x00, 0x01},
		{1221012, 0x00, 0x10},
		{1530923, 0x00, 0xFF},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A55_H_INC */

#if CORTEX_A57_H_INC
{
	.cpu_partnumber = CORTEX_A57_MIDR,
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
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A57_H_INC */

#if CORTEX_A72_H_INC
{
	.cpu_partnumber = CORTEX_A72_MIDR,
	.cpu_errata_list = {
		{859971, 0x00, 0x03},
		{1319367, 0x00, 0xFF},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A72_H_INC */

#if CORTEX_A73_H_INC
{
	.cpu_partnumber = CORTEX_A73_MIDR,
	.cpu_errata_list = {
		{852427, 0x00, 0x00},
		{855423, 0x00, 0x01},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A73_H_INC */

#if CORTEX_A75_H_INC
{
	.cpu_partnumber = CORTEX_A75_MIDR,
	.cpu_errata_list = {
		{764081, 0x00, 0x00},
		{790748, 0x00, 0x00},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A75_H_INC */

#if CORTEX_A76_H_INC
{
	.cpu_partnumber = CORTEX_A76_MIDR,
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
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A76_H_INC */

#if CORTEX_A77_H_INC
{
	.cpu_partnumber = CORTEX_A77_MIDR,
	.cpu_errata_list = {
		{1508412, 0x00, 0x10},
		{1791578, 0x00, 0x11},
		{1800714, 0x00, 0x11},
		{1925769, 0x00, 0x11},
		{1946167, 0x00, 0x11},
		{2356587, 0x00, 0x11},
		{2743100, 0x00, 0x11},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A77_H_INC */

#if CORTEX_A78_H_INC
{
	.cpu_partnumber = CORTEX_A78_MIDR,
	.cpu_errata_list = {
		{1688305, 0x00, 0x10},
		{1821534, 0x00, 0x10},
		{1941498, 0x00, 0x11},
		{1951500, 0x10, 0x11},
		{1952683, 0x00, 0x00, 0x10, true},
		{2132060, 0x00, 0x12},
		{2242635, 0x10, 0x12},
		{2376745, 0x00, 0x12},
		{2395406, 0x00, 0x12},
		{2742426, 0x00, 0x12},
		{2772019, 0x00, 0x12},
		{2779479, 0x00, 0x12},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A78_H_INC */

#if CORTEX_A78_AE_H_INC
{
	.cpu_partnumber = CORTEX_A78_AE_MIDR,
	.cpu_errata_list = {
		{1941500, 0x00, 0x01},
		{1951502, 0x00, 0x01},
		{2376748, 0x00, 0x01},
		{2395408, 0x00, 0x01},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A78_AE_H_INC */

#if CORTEX_A78C_H_INC
{
	.cpu_partnumber = CORTEX_A78C_MIDR,
	.cpu_errata_list = {
		{2132064, 0x01, 0x02},
		{2242638, 0x01, 0x02},
		{2376749, 0x01, 0x02},
		{2395411, 0x01, 0x02},
		{2772121, 0x00, 0x02},
		{2779484, 0x01, 0x02},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A78C_H_INC */

#if CORTEX_X1_H_INC
{
	.cpu_partnumber = CORTEX_X1_MIDR,
	.cpu_errata_list = {
		{1688305, 0x00, 0x10},
		{1821534, 0x00, 0x10},
		{1827429, 0x00, 0x10},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_X1_H_INC */

#if NEOVERSE_N1_H_INC
{
	.cpu_partnumber = NEOVERSE_N1_MIDR,
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
		{2743102, 0x00, 0x41},
		{UNDEF_ERRATA}
	}
},
#endif /* NEOVERSE_N1_H_INC */

#if NEOVERSE_V1_H_INC
{
	.cpu_partnumber = NEOVERSE_V1_MIDR,
	.cpu_errata_list = {
		{1618635, 0x00, 0x0F, 0x10, true},
		{1774420, 0x00, 0x10, 0x11, true},
		{1791573, 0x00, 0x10, 0x11, true},
		{1852267, 0x00, 0x10, 0x11, true},
		{1925756, 0x00, 0x11},
		{1940577, 0x10, 0x11},
		{1966096, 0x10, 0x11},
		{2108267, 0x00, 0x11},
		{2139242, 0x00, 0x11},
		{2216392, 0x10, 0x11},
		{2294912, 0x00, 0x11},
		{2372203, 0x00, 0x11},
		{2743093, 0x00, 0x12},
		{2779461, 0x00, 0x12},
		{UNDEF_ERRATA}
	}
},
#endif /* NEOVERSE_V1_H_INC */

#if CORTEX_A710_H_INC
{
	.cpu_partnumber = CORTEX_A710_MIDR,
	.cpu_errata_list = {
		{1987031, 0x00, 0x20},
		{2008768, 0x00, 0x20, 0x21, true},
		{2017096, 0x00, 0x20},
		{2055002, 0x10, 0x20},
		{2058056, 0x00, 0x10},
		{2081180, 0x00, 0x20},
		{2083908, 0x20, 0x20},
		{2136059, 0x00, 0x20, 0x21, true},
		{2147715, 0x20, 0x20, 0x21, true},
		{2216384, 0x00, 0x20, 0x21, true},
		{2267065, 0x00, 0x20, 0x21, true},
		{2282622, 0x00, 0x21},
		{2291219, 0x00, 0x20, 0x21, true},
		{2371105, 0x00, 0x20, 0x21, true},
		{2768515, 0x00, 0x21}
	}
},
#endif /* CORTEX_A710_H_INC */

#if NEOVERSE_N2_H_INC
{
	.cpu_partnumber = NEOVERSE_N2_MIDR,
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
		{2326639, 0x00, 0x00, 0x01, true},
		{2376738, 0x00, 0x00, 0x01, true},
		{2388450, 0x00, 0x00, 0x01, true},
		{2743089, 0x00, 0x02, 0x03, true},
		{UNDEF_ERRATA}
	}
},
#endif /* NEOVERSE_N2_H_INC */

#if CORTEX_X2_H_INC
{
	.cpu_partnumber = CORTEX_X2_MIDR,
	.cpu_errata_list = {
		{2002765, 0x00, 0x20},
		{2017096, 0x00, 0x20, 0x21, true},
		{2058056, 0x00, 0x20},
		{2081180, 0x00, 0x20, 0x21, true},
		{2083908, 0x00, 0x20},
		{2147715, 0x20, 0x20, 0x21, true},
		{2216384, 0x00, 0x20, 0x21, true},
		{2282622, 0x00, 0x21},
		{2371105, 0x00, 0x21},
		{2768515, 0x00, 0x21},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_X2_H_INC */

#if CORTEX_A510_H_INC
{
	.cpu_partnumber = CORTEX_A510_MIDR,
	.cpu_errata_list = {
		{1922240, 0x00, 0x00, 0x01, true},
		{2041909, 0x02, 0x02, 0x03, true},
		{2042739, 0x00, 0x02, 0x03, true},
		{2172148, 0x00, 0x10, 0x11, true},
		{2218950, 0x00, 0x10, 0x11, true},
		{2250311, 0x00, 0x10, 0x11, true},
		{2288014, 0x00, 0x10, 0x11, true},
		{2347730, 0x00, 0x11, 0x12, true},
		{2371937, 0x00, 0x11, 0x12, true},
		{2666669, 0x00, 0x11, 0x12, true},
		{2684597, 0x00, 0x12, 0x13, true},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A510_H_INC */
};

/*
 * Function to do binary search and check for the specific errata ID
 * in the array of structures specific to the cpu identified.
 */
int32_t binary_search(struct em_cpu_list *ptr, uint32_t erratum_id,
			uint8_t rxpx_val)
{
	int low_index = 0U, mid_index = 0U, high_index = 0U;

	high_index = MAX_ERRATA_ENTRIES - 1;

	while (low_index <= high_index) {
		mid_index = (low_index + high_index) / 2;
		if (erratum_id < ptr->cpu_errata_list[mid_index].em_errata_id) {
			high_index = mid_index - 1;
		} else if (erratum_id > ptr->cpu_errata_list[mid_index].em_errata_id) {
			low_index = mid_index + 1;
		} else if (erratum_id == ptr->cpu_errata_list[mid_index].em_errata_id) {
		/*
		 * errata has been fixed in the hardware for specified revision and above.
		 */
			if ((rxpx_val >= ptr->cpu_errata_list[mid_index].hardware_mitigated) && \
			    (ptr->cpu_errata_list[mid_index].hw_flag)) {
				return EM_NOT_AFFECTED;
			}

			if (RXPX_RANGE(rxpx_val, ptr->cpu_errata_list[mid_index].em_rxpx_lo, \
						ptr->cpu_errata_list[mid_index].em_rxpx_hi)) {
				return EM_HIGHER_EL_MITIGATION;
			}
			return EM_UNKNOWN_ERRATUM;
		}
	}
	/* no matching errata ID */
	return EM_UNKNOWN_ERRATUM;
}

/* Function to check if the errata exists for the specific CPU and rxpx */
int32_t verify_errata_implemented(uint32_t errata_id, uint32_t forward_flag,
					void *handle)
{
	/*
	 * Read MIDR value and extract the revision, variant and partnumber
	 */
	static uint32_t midr_val, cpu_partnum;
	static uint8_t  cpu_rxpx_val;
	int32_t ret_val = EM_UNKNOWN_ERRATUM;

	/* Determine the number of cpu listed in the cpu list */
	uint8_t size_cpulist = ARRAY_SIZE(cpu_list);

	/* Read the midr reg to extract cpu, revision and variant info */
	midr_val = read_midr();

	/* Extract revision and variant from the MIDR register */
	cpu_rxpx_val = EXTRACT_REV_VAR(midr_val);

	/* Extract the cpu partnumber and check if the cpu is in the cpu list */
	cpu_partnum = EXTRACT_PARTNUM(midr_val);

	for (uint8_t i = 0; i < size_cpulist; i++) {
		cpu_ptr = &cpu_list[i];
		uint16_t partnum_extracted = EXTRACT_PARTNUM(cpu_ptr->cpu_partnumber);

		if (partnum_extracted == cpu_partnum) {
			/*
			 * If the midr value is in the cpu list, binary search
			 * for the errata ID and specific revision in the list.
			 */
			ret_val = binary_search(cpu_ptr, errata_id, cpu_rxpx_val);
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

bool validate_spsr_mode(void)
{
	/* In AArch64, if the caller is EL1, return true */

	#if __aarch64__
		if (GET_EL(read_spsr_el3()) == MODE_EL1) {
			return true;
		}
		return false;
	#else

	/* In AArch32, if in system/svc mode, return true */
		uint8_t read_el_state = GET_M32(read_spsr());

		if ((read_el_state == (MODE32_svc)) || (read_el_state == MODE32_sys)) {
			return true;
		}
		return false;
	#endif /* __aarch64__ */
}

uintptr_t errata_abi_smc_handler(uint32_t smc_fid, u_register_t x1,
				u_register_t x2, u_register_t x3, u_register_t x4,
				void *cookie, void *handle, u_register_t flags)
{
	int32_t ret_id = EM_UNKNOWN_ERRATUM;

	switch (smc_fid) {
	case ARM_EM_VERSION:
		SMC_RET1(handle, MAKE_SMCCC_VERSION(
			EM_VERSION_MAJOR, EM_VERSION_MINOR
		));
		break; /* unreachable */
	case ARM_EM_FEATURES:
		if (is_errata_fid((uint32_t)x1)) {
			SMC_RET1(handle, EM_SUCCESS);
		}

		SMC_RET1(handle, EM_NOT_SUPPORTED);
		break; /* unreachable */
	case ARM_EM_CPU_ERRATUM_FEATURES:

		/*
		 * If the forward flag is greater than zero and the calling EL
		 * is EL1 in AArch64 or in system mode or svc mode in case of AArch32,
		 * return Invalid Parameters.
		 */
		if (((uint32_t)x2 != 0) && (true == validate_spsr_mode())) {
			SMC_RET1(handle, EM_INVALID_PARAMETERS);
		}
		ret_id = verify_errata_implemented((uint32_t)x1, (uint32_t)x2, handle);
		SMC_RET1(handle, ret_id);
		break; /* unreachable */
	default:
		{
		   WARN("Unimplemented Errata ABI Service Call: 0x%x\n", smc_fid);
		   SMC_RET1(handle, EM_UNKNOWN_ERRATUM);
		   break; /* unreachable */
		}
	}
}
