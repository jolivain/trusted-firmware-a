/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/smmu_v3.h>
#include <drivers/arm/sp805.h>
#if FVP_USE_SP804_TIMER
#include <drivers/arm/sp804_delay_timer.h>
#else
#include <drivers/generic_delay_timer.h>
#endif
#include <lib/mmio.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/common/arm_def.h>
#include <plat/common/platform.h>
#include "fvp_private.h"

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	arm_bl1_early_platform_setup();

	/* Initialize the platform config for future decision making */
	fvp_config_setup();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	fvp_interconnect_init();
	/*
	 * Enable coherency in Interconnect for the primary CPU's cluster.
	 */
	fvp_interconnect_enable();
}

void plat_arm_secure_wdt_start(void)
{
	sp805_start(ARM_SP805_TWDG_BASE, ARM_TWDG_LOAD_VAL);
}

void plat_arm_secure_wdt_stop(void)
{
	sp805_stop(ARM_SP805_TWDG_BASE);
}

void bl1_platform_setup(void)
{
	arm_bl1_platform_setup();

#if FVP_USE_SP804_TIMER
	/* Enable the clock override for SP804 timer 0, which means that no
	 * clock dividers are applied and the raw (35MHz) clock will be used.
	 */
	mmio_write_32(V2M_SP810_BASE, FVP_SP810_CTRL_TIM0_OV);

	/* Initialize delay timer driver using SP804 dual timer 0 */
	sp804_timer_init(V2M_SP804_TIMER0_BASE,
			SP804_TIMER_CLKMULT, SP804_TIMER_CLKDIV);
#else
	generic_delay_timer_init();

	/* Enable System level generic timer */
	mmio_write_32(ARM_SYS_CNTCTL_BASE + CNTCR_OFF,
			CNTCR_FCREQ(0U) | CNTCR_EN);
#endif /* FVP_USE_SP804_TIMER */

	/* On FVP RevC, initialize SMMUv3 */
	if ((arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3) != 0U)
		smmuv3_security_init(PLAT_FVP_SMMUV3_BASE);
}
