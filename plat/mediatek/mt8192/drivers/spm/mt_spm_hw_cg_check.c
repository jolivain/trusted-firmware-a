#include <arch_helpers.h>
#include <arch.h>
#include <bakery_lock.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_dpidle.h>
#include <mt_spm_sodi.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_pmic_wrap.h>
#include <plat_pm.h>
#include <platform_def.h>
#include <plat_private.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <mtk_plat_common.h>
#include <mtk_gic_v3_main.h>


enum idle_cond_check_type {
	HWCG_CHECK_SCENARIO_ORIENTED = 0,
	HWCG_CHECK_RESOURCE_ORIENTED,
};

enum idle_cond_check_item {
	IDLE_COND_MTCMOS1 = 0,
	IDLE_COND_MTCMOS2,
	IDLE_COND_INFRA0,
	IDLE_COND_INFRA1,
	IDLE_COND_INFRA2,
	IDLE_COND_INFRA3,
	IDLE_COND_MMSYS0,
	IDLE_COND_MMSYS1,
	NR_CG_GRPS,
};

/* scenario-oriented */
enum mtk_idle_type_id {
	IDLE_TYPE_DP = 0,
	IDLE_TYPE_SO3,
	IDLE_TYPE_SO,
	NR_IDLE_TYPES,
};

static uint32_t idle_cond_mask_scenario[NR_IDLE_TYPES][NR_CG_GRPS] = {
	[IDLE_TYPE_DP] = {
		0xBE7000B8,     /* MTCMOS1 */
		0xBE7000B8,     /* MTCMOS2 */
		0x08040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000C5,     /* INFRA2  */
		0x00000000,     /* INFRA3  */
		0xFFFFFFFF,     /* MMSYS0  */
		0x0001FFFF,     /* MMSYS1  */
	},
	[IDLE_TYPE_SO3] = {
		0xBE7000B0,     /* MTCMOS1 */
		0xBE7000B0,     /* MTCMOS2 */
		0x0A040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000D1,     /* INFRA2  */
		0x08000000,     /* INFRA3  */
		0xFFFFFFFF,     /* MMSYS0  */
		0x0001FFFF,     /* MMSYS1  */
	},
	[IDLE_TYPE_SO] = {
		0xBE7000B0,     /* MTCMOS1 */
		0xBE7000B0,     /* MTCMOS2 */
		0x08040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000C1,     /* INFRA2  */
		0x00000000,     /* INFRA3  */
		0x000DFC00,     /* MMSYS0  */
		0x00003F7C,     /* MMSYS1  */
	},
};

enum mtk_resource_level_id {
	SPM_RES_LEVEL_DRAM,
	SPM_RES_LEVEL_SYSPLL,
	SPM_RES_LEVEL_BUS_26M,
	SPM_RES_LEVEL_PMIC_LP,
	NR_SPM_RES_LEVEL_TYPES,
};

static uint32_t idle_cond_mask_resource[NR_SPM_RES_LEVEL_TYPES][NR_CG_GRPS] = {
	[SPM_RES_LEVEL_DRAM] = {
		0xBE7000B0,     /* MTCMOS1 */
		0xBE7000B0,     /* MTCMOS2 */
		0x08040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000C1,     /* INFRA2  */
		0x00000000,     /* INFRA3  */
		0x03F63000,     /* MMSYS0  */
		0x00000000,     /* MMSYS1  */
	},
	[SPM_RES_LEVEL_SYSPLL] = {
		0xBE7000B0,     /* MTCMOS1 */
		0xBE7000B0,     /* MTCMOS1 */
		0x08040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000C1,     /* INFRA2  */
		0x00000000,     /* INFRA3  */
		0x03F63000,     /* MMSYS0  */
		0x00000000,     /* MMSYS1  */
	},
	[SPM_RES_LEVEL_BUS_26M] = {
		0xBE7000B0,     /* MTCMOS1 */
		0xBE7000B0,     /* MTCMOS1 */
		0x08040802,     /* INFRA0  */
		0x03AFB900,     /* INFRA1  */
		0x000000D1,     /* INFRA2  */
		0x08000000,     /* INFRA3  */
		0x00000000,     /* MMSYS0  */
		0x00000000,     /* MMSYS1  */
	},
	[SPM_RES_LEVEL_PMIC_LP] = {
		0, 0, 0, 0, 0, 0, 0, 0},
};

#define PLL_BIT_UNIVPLL (1 << 0)
#define PLL_BIT_MFGPLL  (1 << 1)
#define PLL_BIT_MSDCPLL (1 << 2)
#define PLL_BIT_TVDPLL  (1 << 3)
#define PLL_BIT_MMPLL   (1 << 4)

static unsigned int idle_pll_cond_mask = {
	PLL_BIT_UNIVPLL | PLL_BIT_MFGPLL |
	PLL_BIT_MSDCPLL | PLL_BIT_TVDPLL |
	PLL_BIT_MMPLL,
};

void spm_set_hardware_cg_check(void)
{
	if (hwcg_check_type == HWCG_CHECK_RESOURCE_ORIENTED) {
	mmio_write_32(INFRA2SPM_DEEPIDLE_CG_CHECK_4_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_SYSPLL][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_DEEPIDLE_CG_CHECK_0_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_SYSPLL][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_DEEPIDLE_CG_CHECK_1_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_SYSPLL][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_DEEPIDLE_CG_CHECK_2_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_SYSPLL][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_DEEPIDLE_CG_CHECK_3_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_SYSPLL][IDLE_COND_INFRA3]);

	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_4_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_1_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_0_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_2_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_3_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_INFRA3]);
	mmio_write_32(TO_SPM_CG_MASK_SODI2_0,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_MMSYS0]);
	mmio_write_32(TO_SPM_CG_MASK_SODI2_1,
	~idle_cond_mask_resource[SPM_RES_LEVEL_DRAM][IDLE_COND_MMSYS1]);

	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_4_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_0_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_1_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_2_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_3_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA3]);
	mmio_write_32(OTHER2SPM_CG_CHECK_MASK,
	~(0x0000001F & idle_pll_cond_mask));

	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_4_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_0_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_1_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_2_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_3_MASK,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_INFRA3]);
	mmio_write_32(TO_SPM_CG_MASK_MCDSR_0,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_MMSYS0]);
	mmio_write_32(TO_SPM_CG_MASK_MCDSR_1,
	~idle_cond_mask_resource[SPM_RES_LEVEL_BUS_26M][IDLE_COND_MMSYS1]);
	} else if (hwcg_check_type == HWCG_CHECK_SCENARIO_ORIENTED) {
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_4_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_0_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_1_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_2_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_SODI_CG_CHECK_3_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_INFRA3]);
	mmio_write_32(TO_SPM_CG_MASK_SODI2_0,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_MMSYS0]);
	mmio_write_32(TO_SPM_CG_MASK_SODI2_1,
	~idle_cond_mask_scenario[IDLE_TYPE_DP][IDLE_COND_MMSYS1]);

	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_4_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO3][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_0_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO3][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_1_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO3][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_2_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO3][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_SODI3_CG_CHECK_3_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO3][IDLE_COND_INFRA3]);
	mmio_write_32(OTHER2SPM_CG_CHECK_MASK,
	~(0x0000001F & idle_pll_cond_mask));

	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_4_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_MTCMOS1]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_0_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_INFRA0]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_1_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_INFRA1]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_2_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_INFRA2]);
	mmio_write_32(INFRA2SPM_MCDSR_CG_CHECK_3_MASK,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_INFRA3]);
	mmio_write_32(TO_SPM_CG_MASK_MCDSR_0,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_MMSYS0]);
	mmio_write_32(TO_SPM_CG_MASK_MCDSR_1,
	~idle_cond_mask_scenario[IDLE_TYPE_SO][IDLE_COND_MMSYS1]);
	}
}

