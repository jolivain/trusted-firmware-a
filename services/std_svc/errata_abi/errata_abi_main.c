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
		{794073, 0x00, 0xFF, ERRATA_A9_794073},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A9_H_INC */

#if CORTEX_A15_H_INC
{
	.cpu_partnumber = CORTEX_A15_MIDR,
	.cpu_errata_list = {
		{816470, 0x30, 0xFF, ERRATA_A15_816470},
		{827671, 0x30, 0xFF, ERRATA_A15_827671},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A15_H_INC */

#if CORTEX_A17_H_INC
{
	.cpu_partnumber = CORTEX_A17_MIDR,
	.cpu_errata_list = {
		{852421, 0x00, 0x12, ERRATA_A17_852421},
		{852423, 0x00, 0x12, ERRATA_A17_852423},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A17_H_INC */

#if CORTEX_A35_H_INC
{
	.cpu_partnumber = CORTEX_A35_MIDR,
	.cpu_errata_list = {
		{855472, 0x00, 0x00, ERRATA_A35_855472},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A35_H_INC */

#if CORTEX_A53_H_INC
{
	.cpu_partnumber = CORTEX_A53_MIDR,
	.cpu_errata_list = {
		{819472, 0x00, 0x01, ERRATA_A53_819472},
		{824069, 0x00, 0x02, ERRATA_A53_824069},
		{826319, 0x00, 0x02, ERRATA_A53_826319},
		{827319, 0x00, 0x02, ERRATA_A53_827319},
		{835769, 0x00, 0x04, ERRATA_A53_835769},
		{836870, 0x00, 0x03, ERRATA_A53_836870},
		{843419, 0x00, 0x04, ERRATA_A53_843419},
		{855873, 0x03, 0xFF, ERRATA_A53_855873},
		{1530924, 0x00, 0xFF, ERRATA_A53_1530924},
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
		{768277, 0x00, 0x00, ERRATA_A55_768277},
		{778703, 0x00, 0x00, ERRATA_A55_778703},
		{798797, 0x00, 0x00, ERRATA_A55_798797},
		{846532, 0x00, 0x01, ERRATA_A55_846532},
		{903758, 0x00, 0x01, ERRATA_A55_903758},
		{1221012, 0x00, 0x10, ERRATA_A55_1221012},
		{1530923, 0x00, 0xFF, ERRATA_A55_1530923},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A55_H_INC */

#if CORTEX_A57_H_INC
{
	.cpu_partnumber = CORTEX_A57_MIDR,
	.cpu_errata_list = {
		{806969, 0x00, 0x00, ERRATA_A57_806969},
		{813419, 0x00, 0x00, ERRATA_A57_813419},
		{813420, 0x00, 0x00, ERRATA_A57_813420},
		{814670, 0x00, 0x00, ERRATA_A57_814670},
		{817169, 0x00, 0x01, ERRATA_A57_817169},
		{826974, 0x00, 0x11, ERRATA_A57_826974},
		{826977, 0x00, 0x11, ERRATA_A57_826977},
		{828024, 0x00, 0x11, ERRATA_A57_828024},
		{829520, 0x00, 0x12, ERRATA_A57_829520},
		{833471, 0x00, 0x12, ERRATA_A57_833471},
		{859972, 0x00, 0x13, ERRATA_A57_859972},
		{1319537, 0x00, 0xFF, ERRATA_A57_1319537},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A57_H_INC */

#if CORTEX_A72_H_INC
{
	.cpu_partnumber = CORTEX_A72_MIDR,
	.cpu_errata_list = {
		{859971, 0x00, 0x03, ERRATA_A72_859971},
		{1319367, 0x00, 0xFF, ERRATA_A72_1319367},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A72_H_INC */

#if CORTEX_A73_H_INC
{
	.cpu_partnumber = CORTEX_A73_MIDR,
	.cpu_errata_list = {
		{852427, 0x00, 0x00, ERRATA_A73_852427},
		{855423, 0x00, 0x01, ERRATA_A73_855423},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A73_H_INC */

#if CORTEX_A75_H_INC
{
	.cpu_partnumber = CORTEX_A75_MIDR,
	.cpu_errata_list = {
		{764081, 0x00, 0x00, ERRATA_A75_764081},
		{790748, 0x00, 0x00, ERRATA_A75_790748},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A75_H_INC */

#if CORTEX_A76_H_INC
{
	.cpu_partnumber = CORTEX_A76_MIDR,
	.cpu_errata_list = {
		{1073348, 0x00, 0x10, ERRATA_A76_1073348},
		{1130799, 0x00, 0x20, ERRATA_A76_1130799},
		{1165522, 0x00, 0xFF, ERRATA_A76_1165522},
		{1220197, 0x00, 0x20, ERRATA_A76_1220197},
		{1257314, 0x00, 0x30, ERRATA_A76_1257314},
		{1262606, 0x00, 0x30, ERRATA_A76_1262606},
		{1262888, 0x00, 0x30, ERRATA_A76_1262888},
		{1275112, 0x00, 0x30, ERRATA_A76_1275112},
		{1791580, 0x00, 0x40, ERRATA_A76_1791580},
		{1868343, 0x00, 0x40, ERRATA_A76_1868343},
		{1946160, 0x30, 0x41, ERRATA_A76_1946160},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A76_H_INC */

#if CORTEX_A77_H_INC
{
	.cpu_partnumber = CORTEX_A77_MIDR,
	.cpu_errata_list = {
		{1508412, 0x00, 0x10, ERRATA_A77_1508412},
		{1791578, 0x00, 0x11, ERRATA_A77_1791578},
		{1800714, 0x00, 0x11, ERRATA_A77_1800714},
		{1925769, 0x00, 0x11, ERRATA_A77_1925769},
		{1946167, 0x00, 0x11, ERRATA_A77_1946167},
		{2356587, 0x00, 0x11, ERRATA_A77_2356587},
		{2743100, 0x00, 0x11, ERRATA_A77_2743100},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A77_H_INC */

#if CORTEX_A78_H_INC
{
	.cpu_partnumber = CORTEX_A78_MIDR,
	.cpu_errata_list = {
		{1688305, 0x00, 0x10, ERRATA_A78_1688305},
		{1821534, 0x00, 0x10, ERRATA_A78_1821534},
		{1941498, 0x00, 0x11, ERRATA_A78_1941498},
		{1951500, 0x10, 0x11, ERRATA_A78_1951500},
		{1952683, 0x00, 0x00, ERRATA_A78_1952683},
		{2132060, 0x00, 0x12, ERRATA_A78_2132060},
		{2242635, 0x10, 0x12, ERRATA_A78_2242635},
		{2376745, 0x00, 0x12, ERRATA_A78_2376745},
		{2395406, 0x00, 0x12, ERRATA_A78_2395406},
		{2712571, 0x00, 0x12, ERRATA_A78_2712571, \
		ERRATA_NON_ARM_INTERCONNECT},
		{2742426, 0x00, 0x12, ERRATA_A78_2742426},
		{2772019, 0x00, 0x12, ERRATA_A78_2772019},
		{2779479, 0x00, 0x12, ERRATA_A78_2779479},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A78_H_INC */

#if CORTEX_A78_AE_H_INC
{
	.cpu_partnumber = CORTEX_A78_AE_MIDR,
	.cpu_errata_list = {
		{1941500, 0x00, 0x01, ERRATA_A78_AE_1941500},
		{1951502, 0x00, 0x01, ERRATA_A78_AE_1951502},
		{2376748, 0x00, 0x01, ERRATA_A78_AE_2376748},
		{2395408, 0x00, 0x01, ERRATA_A78_AE_2395408},
		{2712574, 0x00, 0x02, ERRATA_A78_AE_2712574, \
		ERRATA_NON_ARM_INTERCONNECT},
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
		{2132064, 0x01, 0x02, ERRATA_A78C_2132064},
		{2242638, 0x01, 0x02, ERRATA_A78C_2242638},
		{2376749, 0x01, 0x02, ERRATA_A78C_2376749},
		{2395411, 0x01, 0x02, ERRATA_A78C_2395411},
		{2712575, 0x01, 0x02, ERRATA_A78C_2712575, \
		ERRATA_NON_ARM_INTERCONNECT},
		{2772121, 0x00, 0x02, ERRATA_A78C_2772121},
		{2779484, 0x01, 0x02, ERRATA_A78C_2779484},
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
		{1688305, 0x00, 0x10, ERRATA_X1_1688305},
		{1821534, 0x00, 0x10, ERRATA_X1_1821534},
		{1827429, 0x00, 0x10, ERRATA_X1_1827429},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_X1_H_INC */

#if NEOVERSE_N1_H_INC
{
	.cpu_partnumber = NEOVERSE_N1_MIDR,
	.cpu_errata_list = {
		{1073348, 0x00, 0x10, ERRATA_N1_1073348},
		{1130799, 0x00, 0x20, ERRATA_N1_1130799},
		{1165347, 0x00, 0x20, ERRATA_N1_1165347},
		{1207823, 0x00, 0x20, ERRATA_N1_1207823},
		{1220197, 0x00, 0x20, ERRATA_N1_1220197},
		{1257314, 0x00, 0x30, ERRATA_N1_1257314},
		{1262606, 0x00, 0x30, ERRATA_N1_1262606},
		{1262888, 0x00, 0x30, ERRATA_N1_1262888},
		{1275112, 0x00, 0x30, ERRATA_N1_1275112},
		{1315703, 0x00, 0x30, ERRATA_N1_1315703},
		{1542419, 0x30, 0x40, ERRATA_N1_1542419},
		{1868343, 0x00, 0x40, ERRATA_N1_1868343},
		{1946160, 0x30, 0x41, ERRATA_N1_1946160},
		{2743102, 0x00, 0x41, ERRATA_N1_2743102},
		{UNDEF_ERRATA},	{UNDEF_ERRATA}
	}
},
#endif /* NEOVERSE_N1_H_INC */

#if NEOVERSE_V1_H_INC
{
	.cpu_partnumber = NEOVERSE_V1_MIDR,
	.cpu_errata_list = {
		{1618635, 0x00, 0x0F, ERRATA_V1_1618635},
		{1774420, 0x00, 0x10, ERRATA_V1_1774420},
		{1791573, 0x00, 0x10, ERRATA_V1_1791573},
		{1852267, 0x00, 0x10, ERRATA_V1_1852267},
		{1925756, 0x00, 0x11, ERRATA_V1_1925756},
		{1940577, 0x10, 0x11, ERRATA_V1_1940577},
		{1966096, 0x10, 0x11, ERRATA_V1_1966096},
		{2108267, 0x00, 0x11, ERRATA_V1_2108267},
		{2139242, 0x00, 0x11, ERRATA_V1_2139242},
		{2216392, 0x10, 0x11, ERRATA_V1_2216392},
		{2294912, 0x00, 0x11, ERRATA_V1_2294912},
		{2372203, 0x00, 0x11, ERRATA_V1_2372203},
		{2701953, 0x00, 0x11, ERRATA_V1_2701953, \
		ERRATA_NON_ARM_INTERCONNECT},
		{2743093, 0x00, 0x12, ERRATA_V1_2743093},
		{2779461, 0x00, 0x12, ERRATA_V1_2779461},
		{UNDEF_ERRATA}
	}
},
#endif /* NEOVERSE_V1_H_INC */

#if CORTEX_A710_H_INC
{
	.cpu_partnumber = CORTEX_A710_MIDR,
	.cpu_errata_list = {
		{1987031, 0x00, 0x20, ERRATA_A710_1987031},
		{2008768, 0x00, 0x20, ERRATA_A710_2008768},
		{2017096, 0x00, 0x20, ERRATA_A710_2017096},
		{2055002, 0x10, 0x20, ERRATA_A710_2055002},
		{2058056, 0x00, 0x10, ERRATA_A710_2058056},
		{2081180, 0x00, 0x20, ERRATA_A710_2081180},
		{2083908, 0x20, 0x20, ERRATA_A710_2083908},
		{2136059, 0x00, 0x20, ERRATA_A710_2136059},
		{2147715, 0x20, 0x20, ERRATA_A710_2147715},
		{2216384, 0x00, 0x20, ERRATA_A710_2216384},
		{2267065, 0x00, 0x20, ERRATA_A710_2267065},
		{2282622, 0x00, 0x21, ERRATA_A710_2282622},
		{2291219, 0x00, 0x20, ERRATA_A710_2291219},
		{2371105, 0x00, 0x20, ERRATA_A710_2371105},
		{2701952, 0x00, 0x21, ERRATA_A710_2701952, \
		ERRATA_NON_ARM_INTERCONNECT},
		{2768515, 0x00, 0x21, ERRATA_A710_2768515}
	}
},
#endif /* CORTEX_A710_H_INC */

#if NEOVERSE_N2_H_INC
{
	.cpu_partnumber = NEOVERSE_N2_MIDR,
	.cpu_errata_list = {
		{2002655, 0x00, 0x00, ERRATA_N2_2002655},
		{2025414, 0x00, 0x00, ERRATA_N2_2025414},
		{2067956, 0x00, 0x00, ERRATA_N2_2067956},
		{2138953, 0x00, 0x00, ERRATA_N2_2138953},
		{2138956, 0x00, 0x00, ERRATA_N2_2138953},
		{2138958, 0x00, 0x00, ERRATA_N2_2138958},
		{2189731, 0x00, 0x00, ERRATA_N2_2189731},
		{2242400, 0x00, 0x00, ERRATA_N2_2242400},
		{2242415, 0x00, 0x00, ERRATA_N2_2242415},
		{2280757, 0x00, 0x00, ERRATA_N2_2280757},
		{2326639, 0x00, 0x00, ERRATA_N2_2326639},
		{2376738, 0x00, 0x00, ERRATA_N2_2376738},
		{2388450, 0x00, 0x00, ERRATA_N2_2388450},
		{2728475, 0x00, 0x02, ERRATA_N2_2728475, \
		ERRATA_NON_ARM_INTERCONNECT},
		{2743089, 0x00, 0x02, ERRATA_N2_2743089},
		{UNDEF_ERRATA}
	}
},
#endif /* NEOVERSE_N2_H_INC */

#if CORTEX_X2_H_INC
{
	.cpu_partnumber = CORTEX_X2_MIDR,
	.cpu_errata_list = {
		{2002765, 0x00, 0x20, ERRATA_X2_2002765},
		{2017096, 0x00, 0x20, ERRATA_X2_2017096},
		{2058056, 0x00, 0x20, ERRATA_X2_2058056},
		{2081180, 0x00, 0x20, ERRATA_X2_2081180},
		{2083908, 0x00, 0x20, ERRATA_X2_2083908},
		{2147715, 0x20, 0x20, ERRATA_X2_2147715},
		{2216384, 0x00, 0x20, ERRATA_X2_2216384},
		{2282622, 0x00, 0x21, ERRATA_X2_2282622},
		{2371105, 0x00, 0x21, ERRATA_X2_2371105},
		{2701952, 0x00, 0x21, ERRATA_X2_2701952, \
		ERRATA_NON_ARM_INTERCONNECT},
		{2768515, 0x00, 0x21, ERRATA_X2_2768515},
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
		{1922240, 0x00, 0x00, ERRATA_A510_1922240},
		{2041909, 0x02, 0x02, ERRATA_A510_2041909},
		{2042739, 0x00, 0x02, ERRATA_A510_2042739},
		{2172148, 0x00, 0x10, ERRATA_A510_2172148},
		{2218950, 0x00, 0x10, ERRATA_A510_2218950},
		{2250311, 0x00, 0x10, ERRATA_A510_2250311},
		{2288014, 0x00, 0x10, ERRATA_A510_2288014},
		{2347730, 0x00, 0x11, ERRATA_A510_2347730},
		{2371937, 0x00, 0x11, ERRATA_A510_2371937},
		{2666669, 0x00, 0x11, ERRATA_A510_2666669},
		{2684597, 0x00, 0x12, ERRATA_A510_2684597},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A510_H_INC */

#if NEOVERSE_V2_H_INC
{
	.cpu_partnumber = NEOVERSE_V2_MIDR,
	.cpu_errata_list = {
		{2719103, 0x00, 0x01, ERRATA_V2_2719103, \
		ERRATA_NON_ARM_INTERCONNECT},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* NEOVERSE_V2_H_INC */

#if CORTEX_A715_H_INC
{
	.cpu_partnumber = CORTEX_MAKALU_MIDR,
	.cpu_errata_list = {
		{2701951, 0x00, 0x11, ERRATA_A715_2701951, \
		ERRATA_NON_ARM_INTERCONNECT},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA},	{UNDEF_ERRATA},
		{UNDEF_ERRATA}
	}
},
#endif /* CORTEX_A715_H_INC */
};

/*
 * Function to do binary search and check for the specific errata ID
 * in the array of structures specific to the cpu identified.
 */
int32_t binary_search(struct em_cpu_list *ptr, uint32_t erratum_id, uint8_t rxpx_val)
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
			if ((ptr->cpu_errata_list[mid_index].non_arm_interconnect) && \
			(ptr->cpu_errata_list[mid_index].errata_enabled)) {
				return EM_AFFECTED;
			} else if ((ptr->cpu_errata_list[mid_index].errata_enabled) && \
			(!(ptr->cpu_errata_list[mid_index].non_arm_interconnect))) {
				return EM_HIGHER_EL_MITIGATION;
			} else {
				return EM_AFFECTED;
			}
		} else {
			return EM_UNKNOWN_ERRATUM;
		}
	}
	/* no matching errata ID */
	return EM_UNKNOWN_ERRATUM;
}

/* Function to check if the errata exists for the specific CPU and rxpx */
int32_t verify_errata_implemented(uint32_t errata_id, uint32_t forward_flag, void *handle)
{
	/*
	 * Read MIDR value and extract the revision, variant and partnumber
	 */
	static uint32_t midr_val, cpu_partnum;
	static uint8_t  cpu_rxpx_val;
	int32_t ret_val = EM_UNKNOWN_ERRATUM;
	extern uint8_t cpu_get_rev_var();

	/* Determine the number of cpu listed in the cpu list */
	uint8_t size_cpulist = ARRAY_SIZE(cpu_list);

	/* Read the midr reg to extract cpu, revision and variant info */
	midr_val = read_midr();

	/* Extract revision and variant from the MIDR register */
	cpu_rxpx_val = cpu_get_rev_var();

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
