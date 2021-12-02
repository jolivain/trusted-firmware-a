/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mtk_sip_svc.h>
#include <plat_dfd.h>

static bool dfd_enabled;
static uint64_t dfd_base_addr;
static uint64_t dfd_chain_length;
static uint64_t dfd_cache_dump;

static void dfd_setup(uint64_t base_addr, uint64_t chain_length,
		      uint64_t cache_dump)
{
	mmio_write_32(MCUSYS_DFD_MAP, base_addr >> 24);
	mmio_write_32(WDT_DEBUG_CTL, WDT_DEBUG_CTL_VAL_0);

	/* bit[0]: rg_rw_dfd_internal_dump_en */
	/* bit[2]: rg_rw_dfd_clock_stop_en */
	sync_writel(DFD_INTERNAL_CTL, (BIT(0) | BIT(2)));

	/* bit[13]: xreset_b_update_disable */
	mmio_setbits_32(DFD_INTERNAL_CTL, BIT(13));

	/*
	 * bit[10:3]: DFD trigger selection mask :
	 * bit[3]: rg_rw_dfd_trigger_sel[0]: 1'b1 -> enable wdt trigger
	 * bit[4]: rg_rw_dfd_trigger_sel[1]: 1'b1 -> enable HW trigger
	 * bit[5]: rg_rw_dfd_trigger_sel[2]: 1'b1 -> enable SW trigger
	 * bit[6]: rg_rw_dfd_trigger_sel[3]: 1'b1 -> enable SW non-security trigger
	 * bit[7]: rg_rw_dfd_trigger_sel[4]: 1'b1 -> enable timer trigger
	 */
	mmio_setbits_32(DFD_INTERNAL_CTL, BIT(3));

	/* bit[20:19]: rg_dfd_armpll_div_mux_sel switch to PLL2 for DFD */
	mmio_setbits_32(DFD_INTERNAL_CTL, (BIT(19) | BIT(20)));

	/*
	 * bit[0]: rg_rw_dfd_auto_power_on -> 1
	 * bit[2:1]: rg_rw_dfd_auto_power_on_dely -> 1 (10us)
	 * bit[4:2]: rg_rw_dfd_power_on_wait_time -> 1 (20us)
	 */
	mmio_write_32(DFD_INTERNAL_PWR_ON, (BIT(0) | BIT(1) | BIT(3)));

	/* longest scan chain length */
	mmio_write_32(DFD_CHAIN_LENGTH0, chain_length);

	/*
	 * bit[1:0]: rg_rw_dfd_shift_clock_ratio
	 */
	mmio_write_32(DFD_INTERNAL_SHIFT_CLK_RATIO, 0);

	/* total 59 test_so */
	mmio_write_32(DFD_INTERNAL_TEST_SO_0, DFD_INTERNAL_TEST_SO_0_VAL);

	/* only one group of test_so */
	mmio_write_32(DFD_INTERNAL_NUM_OF_TEST_SO_GROUP, 1);

	/* for big circular buffer read failed after reboot */
	mmio_write_32(DFD_TEST_SI_0, DFD_TEST_SI_0_VAL);

	mmio_write_32(DFD_TEST_SI_1, DFD_TEST_SI_1_VAL);

	/* for DFD-3.0 setup */
	sync_writel(DFD_V30_CTL, 1);

	/* set base address */
	mmio_write_32(DFD_V30_BASE_ADDR, (base_addr & 0xFFF00000));

	/* setup global variables for suspend and resume */
	dfd_enabled = true;
	dfd_base_addr = base_addr;
	dfd_chain_length = chain_length;
	dfd_cache_dump = cache_dump;

	if ((cache_dump & DFD_CACHE_DUMP_ENABLE) != 0UL) {
		mmio_write_32(WDT_DEBUG_CTL, WDT_DEBUG_CTL_VAL_1);
		sync_writel(DFD_V35_ENALBE, 1);
		sync_writel(DFD_V35_TAP_NUMBER, DFD_V35_TAP_NUMBER_VAL);
		sync_writel(DFD_V35_TAP_EN, DFD_V35_TAP_EN_VAL);
		sync_writel(DFD_V35_SEQ0_0, DFD_V35_SEQ0_0_VAL);

		if (cache_dump & DFD_PARITY_ERR_TRIGGER) {
			sync_writel(DFD_HW_TRIGGER_MASK, DFD_HW_TRIGGER_MASK_VAL);
			mmio_setbits_32(DFD_INTERNAL_CTL, BIT(4));
		}
	}
	dsbsy();
}

void dfd_resume(void)
{
	if (dfd_enabled == true) {
		dfd_setup(dfd_base_addr, dfd_chain_length, dfd_cache_dump);
	}
}

uint64_t dfd_smc_dispatcher(uint64_t arg0, uint64_t arg1,
			    uint64_t arg2, uint64_t arg3)
{
	uint64_t ret = 0L;

	switch (arg0) {
	case PLAT_MTK_DFD_SETUP_MAGIC:
		INFO("[%s] DFD setup call from kernel\n", __func__);
		dfd_setup(arg1, arg2, arg3);
		break;
	case PLAT_MTK_DFD_READ_MAGIC:
		/* only allow to access DFD register base + 0x200 */
		if (arg1 <= 0x200) {
			ret = mmio_read_32(MISC1_CFG_BASE + arg1);
		}
		break;
	case PLAT_MTK_DFD_WRITE_MAGIC:
		/* only allow to access DFD register base + 0x200 */
		if (arg1 <= 0x200) {
			sync_writel(MISC1_CFG_BASE + arg1, arg2);
		}
		break;
	default:
		ret = MTK_SIP_E_INVALID_PARAM;
		break;
	}

	return ret;
}
