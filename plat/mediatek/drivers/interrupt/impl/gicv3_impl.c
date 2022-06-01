/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
/* XXX: Manually export GiC v3 private API */
#include <v3/gicv3_private.h>
/* MTK header files */
#include <drivers/interrupt.h>
#include <lib/mtk_init/mtk_init.h>
#include <mtk_mmap_pool.h>
#include <platform_def.h>

#include "mtk_gic_v3_private.h"

static int gicv3_configure_interrupt(uint32_t fiq_id, struct intr_flag flag);
static uint32_t gicv3_ack_interrupt(void);
static void gicv3_eoi_interrupt(uint32_t fiq_id);
static int gicv3_validate_intr_num(uint32_t fiq_id);
static uint64_t gicv3_dump_interrupt(uint32_t int_id);
static int gicv3_enable_intr(uint32_t fiq_id);

static struct interrupt_controller gicv3_intr_contr = {
	.config_intr = gicv3_configure_interrupt,
	.enable_intr = gicv3_enable_intr,
	.ack_intr = gicv3_ack_interrupt,
	.eoi_intr = gicv3_eoi_interrupt,
	.validate_intr = gicv3_validate_intr_num,
	.dump_intr = gicv3_dump_interrupt,
};

static const mmap_region_t gicv3_mmap[] MTK_MMAP_SECTION = {
	MAP_REGION_FLAT(MT_GIC_BASE, MTK_GIC_REG_SIZE,
	MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};
DECLARE_MTK_MMAP_REGIONS(gicv3_mmap);

static int gicv3_configure_interrupt(uint32_t fiq_id, struct intr_flag flag)
{
	interrupt_prop_t intr_prop;

	intr_prop.intr_num = fiq_id;
	intr_prop.intr_pri = GIC_HIGHEST_SEC_PRIORITY;
	intr_prop.intr_grp = INTR_GROUP0;

	if (IS_INTR_LEVEL(flag.trigger_type))
		intr_prop.intr_cfg = GIC_INTR_CFG_LEVEL;
	else
		intr_prop.intr_cfg = GIC_INTR_CFG_EDGE;

	gicv3_secure_spis_config_props(BASE_GICD_BASE, &intr_prop, 1);
	return 0;
}

static uint32_t gicv3_ack_interrupt(void)
{
	return plat_ic_acknowledge_interrupt();
}

static void gicv3_eoi_interrupt(uint32_t fiq_id)
{
	plat_ic_end_of_interrupt(fiq_id);
}

static int gicv3_validate_intr_num(uint32_t fiq_id)
{
	/* Only register SPI and SGI, PPI  */
	if (!IS_SPI(fiq_id) && !IS_SGI_PPI(fiq_id)) {
		ERROR("Interrupt id is not valid:%u\n", fiq_id);
		return -EINTR_BAD_INTID;
	}
	return 0;
}

static uint64_t gicv3_dump_interrupt(uint32_t irq)
{
	int cpu;
	uint32_t bit, result, addr;
	uint64_t rc = 0, route;

	INFO("irq = %u\n", irq);

	/* only for SPI */
	if (!IS_SPI(irq))
		return 0;

	/* get mask */
	addr = BASE_GICD_BASE + GICD_ISENABLER + irq / 32 * 4;
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(addr) & bit) ? 1 : 0);
	INFO("enable = %x\n", result);
	rc |= result;

	/* get group */
	addr = BASE_GICD_BASE + GICD_IGROUPR + irq / 32 * 4;
	bit = 1 << (irq % 32);
	/* 0x1: irq, 0x0: fiq */
	result = ((mmio_read_32(addr) & bit) ? 1 : 0);
	INFO("group = %x\n", result);
	rc |=  result << 1;

	/* get group modifier */
	addr = BASE_GICD_BASE + GICD_IGRPMODR + irq / 32 * 4;
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(addr) & bit) ? 1 : 0);
	INFO("group modifier = %x\n", result);
	rc |=  result << 2;

	/* get priority */
	addr = BASE_GICD_BASE + GICD_IPRIORITYR + irq / 4 * 4;
	bit = 0xff << ((irq % 4) * 8);
	result = ((mmio_read_32(addr) & bit) >> ((irq % 4) * 8));
	INFO("priority = %x\n", result);
	rc |= result << 3;

	/* get sensitivity */
	addr = BASE_GICD_BASE + GICD_ICFGR + irq / 16 * 4;
	bit = 0x3 << ((irq % 16) * 2);
	/* 0x2: edge, 0x1: level */
	result = ((mmio_read_32(addr) & bit) >> ((irq % 16) * 2));
	INFO("sensitivity = %x (edge:0x1, level:0x0)\n", result >> 1);
	rc |= (result >> 1) << 11;

	/* get pending status */
	addr = BASE_GICD_BASE + GICD_ISPENDR + irq / 32 * 4;
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(addr) & bit) ? 1 : 0);
	INFO("pending status = %x\n", result);
	rc |= result << 12;

	/* get active status */
	addr = BASE_GICD_BASE + GICD_ISACTIVER + irq / 32 * 4;
	bit = 1 << (irq % 32);
	result = ((mmio_read_32(addr) & bit) ? 1 : 0);
	INFO("active status = %x\n", result);
	rc |= result << 13;

	/* get router, occupy 16bit in rc [14:29] */
	addr = BASE_GICD_BASE + GICD_IROUTER + (irq * 8);
	route = mmio_read_64(addr);
	cpu = plat_core_pos_by_mpidr(route);

	if (cpu >= PLATFORM_CORE_COUNT || cpu < 0) {
		ERROR("%s cannot get mpidr (%d)\n", __func__, cpu);
		goto end;
	}
	if (route & (IROUTER_IRM_MASK << IROUTER_IRM_SHIFT))
		rc |= 0xffff << 14;
	else
		rc |= (1UL << cpu) << 14;

	INFO("affinity = %x\n", cpu);
end:
	return rc;
}

static int gicv3_enable_intr(uint32_t fiq_id)
{
	plat_ic_enable_interrupt(fiq_id);
	return 0;
}

static void gicv3_enable_group0(void)
{
	gicd_set_ctlr(BASE_GICD_BASE, CTLR_ENABLE_G0_BIT, RWP_TRUE);
}

static int init_mtk_intr(void)
{
	setup_interrupt_controller(&gicv3_intr_contr);
	/* Initialize the gic cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();
	/* Enable GICD_CTRL group0 */
	gicv3_enable_group0();
	return 0;
}
MTK_ARCH_INIT(init_mtk_intr);
