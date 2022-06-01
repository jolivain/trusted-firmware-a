/*
 * Copyright (c) 2020, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_GICV3_DEF_H
#define MTK_GICV3_DEF_H

/* GiC driver init constant */
#define PLAT_ARM_GICD_BASE      BASE_GICD_BASE
#define PLAT_ARM_GICC_BASE      BASE_GICC_BASE
#define PLAT_ARM_GICR_BASE      BASE_GICR_BASE
/*
 * arm_gicv3.c needs to assign G1S/G0S array
 * for initialization.
 * Since we implement request_fiq to register
 * interrupt handler and configure interrupt,
 * so arm_interrupt_props is filled with dummy
 * INTID for build pass purpose.
 */
#define MTK_IRQ_DUMMY                 1023
#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(MTK_IRQ_DUMMY, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)
#define PLAT_ARM_G0_IRQ_PROPS(grp) \
	INTR_PROP_DESC(WDT_IRQ_BIT_ID, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(FIQ_SMP_CALL_SGI, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)
/*
 * FIXME: replace plat_arm_calc_core_pos
 * with plat_mediatek_calc_core_pos to leverage ARM gic driver
 */
#define plat_arm_calc_core_pos plat_mediatek_calc_core_pos

#endif /* MTK_GICV3_DEF_H */

