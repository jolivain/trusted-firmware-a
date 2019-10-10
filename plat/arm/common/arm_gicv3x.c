/*
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <platform_def.h>

#include <common/interrupt_props.h>
#include <drivers/arm/gicv3x.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include "../drivers/arm/gic/v3x/gicv3x_private.h"

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way the GICv3.x driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_arm_gic_driver_init
#pragma weak plat_arm_gic_init
#pragma weak plat_arm_gic_cpuif_enable
#pragma weak plat_arm_gic_cpuif_disable
#pragma weak plat_arm_gic_pcpu_init
#pragma weak plat_arm_gic_redistif_on
#pragma weak plat_arm_gic_redistif_off

/* Maximum PPI INTID: 31, 1087 or 1119 */
extern unsigned int max_ppi_intid;

/* Number of GICR_ICxxxx and GICR_ISxxxx registers supported: 1-3 */
extern unsigned int ppi_regs_num;

/*
 * The number of instances of the GICD_IGROUPR<n>, GICD_ISENABLER<n>, etc.
 * interrupt registers
 */
extern unsigned int spi_regs_num;

/*
 * The number of instances of the GICD_IGROUPR<n>E, GICD_ISENABLER<n>E, etc.
 * interrupt registers
 */
extern unsigned int espi_regs_num;

/* Maximum SPI INTID: 32*(ITLinesNumber + 1) - 1 */
extern unsigned int max_spi_intid;

/* Maximum ESPI INTID: 32*(ESPI_range + 1) + 4095 */
extern unsigned int max_espi_intid;

/* The GICv3.x driver only needs to be initialized in EL3 */
static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static const interrupt_prop_t arm_interrupt_props[] = {
	PLAT_ARM_G1S_IRQ_PROPS(INTR_GROUP1S),
	PLAT_ARM_G0_IRQ_PROPS(INTR_GROUP0)
};

/*
 * We save and restore the GICv3.x context on system suspend. Allocate the
 * data in the designated EL3 Secure carve-out memory. The `volatile`
 * is used to prevent the compiler from removing the GICv3.x contexts even
 * though the DEFINE_LOAD_SYM_ADDR creates a dummy reference to it.
 */
static volatile gicv3x_redist_ctx_t rdist_ctx __section("arm_el3_tzc_dram");
static volatile gicv3x_dist_ctx_t dist_ctx __section("arm_el3_tzc_dram");

/* Define accessor function to get reference to the GICv3.x context */
DEFINE_LOAD_SYM_ADDR(rdist_ctx)
DEFINE_LOAD_SYM_ADDR(dist_ctx)

/*
 * MPIDR hashing function for translating MPIDRs read from GICR_TYPER register
 * to core position.
 *
 * Calculating core position is dependent on MPIDR_EL1.MT bit.
 * However, affinity values read from GICR_TYPER don't have an MT field.
 * To reuse the same translation used for CPUs, we insert MT bit read from
 * the PE's MPIDR into that read from GICR_TYPER.
 *
 * Assumptions:
 *
 *   - All CPUs implemented in the system have MPIDR_EL1.MT bit set;
 *   - No CPUs implemented in the system use affinity level 3.
 */
static unsigned int arm_gicv3x_mpidr_hash(u_register_t mpidr)
{
	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);
	return plat_arm_calc_core_pos(mpidr);
}

static const gicv3x_driver_data_t arm_gic_data __unused = {
	.gicd_base = PLAT_ARM_GICD_BASE,
	.gicr_base = 0U,
	.interrupt_props = arm_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(arm_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = arm_gicv3x_mpidr_hash
};

void __init plat_arm_gic_driver_init(void)
{
	/*
	 * The GICv3.x driver is initialized in EL3 and does not need
	 * to be initialized again in S-EL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
#if (!defined(__aarch64__) && defined(IMAGE_BL32)) || \
	(defined(__aarch64__) && defined(IMAGE_BL31))
	gicv3x_driver_init(&arm_gic_data);

	if (gicv3x_rdistif_probe(PLAT_ARM_GICR_BASE) == -1) {
		ERROR("No GICR base frame found for Primary CPU\n");
		panic();
	}

	/* Calculate number of PPI registers */
	ppi_regs_num = ((gicr_read_typer(PLAT_ARM_GICR_BASE) >>
				TYPER_PPI_NUM_SHIFT) & TYPER_PPI_NUM_MASK) + 1;

	/* Calculate the maximum PPI INTID that GIC implementation supports */
	switch (ppi_regs_num) {
	case 1:
		/* 31 */
		max_ppi_intid = MAX_PPI_ID;
		break;
	case 2:
		/* 1087 */
		max_ppi_intid = MIN_EPPI_ID + MAX_PPI_ID;
		break;
	case 3:
		/* 1119 */
		max_ppi_intid = MAX_EPPI_ID;
		break;
	default:
		assert(false);
	}

	if (espi_regs_num == 0) {
		INFO("Max PPI, SPI INTID: %u, %u\n",
			max_ppi_intid, max_spi_intid);
	} else {
		INFO("Max PPI, SPI, ESPI INTID: %u, %u, %u\n",
			max_ppi_intid, max_spi_intid, max_espi_intid);
	}

#endif
}

/******************************************************************************
 * ARM common helper to initialize the GIC. Only invoked by BL31
 *****************************************************************************/
void __init plat_arm_gic_init(void)
{
	gicv3x_distif_init();
	gicv3x_rdistif_init(plat_my_core_pos());
	gicv3x_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to enable the GIC CPU interface
 *****************************************************************************/
void plat_arm_gic_cpuif_enable(void)
{
	gicv3x_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to disable the GIC CPU interface
 *****************************************************************************/
void plat_arm_gic_cpuif_disable(void)
{
	gicv3x_cpuif_disable(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper function to iterate over all GICR frames and discover the
 * corresponding per-CPU redistributor frame as well as initialize the
 * corresponding interface in GICv3x. At the moment, Arm platforms do not have
 * non-contiguous GICR frames.
 *****************************************************************************/
void plat_arm_gic_pcpu_init(void)
{
	int result = gicv3x_rdistif_probe(PLAT_ARM_GICR_BASE);

	if (result == -1) {
		ERROR("No GICR base frame found for CPU 0x%lx\n", read_mpidr());
		panic();
	}
	gicv3x_rdistif_init(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helpers to power GIC redistributor interface
 *****************************************************************************/
void plat_arm_gic_redistif_on(void)
{
	gicv3x_rdistif_on(plat_my_core_pos());
}

void plat_arm_gic_redistif_off(void)
{
	gicv3x_rdistif_off(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to save & restore the GICv3.x on resume from
 * system suspend
 *****************************************************************************/
void plat_arm_gic_save(void)
{
	gicv3x_redist_ctx_t * const rdist_context =
			(gicv3x_redist_ctx_t *)LOAD_ADDR_OF(rdist_ctx);
	gicv3x_dist_ctx_t * const dist_context =
			(gicv3x_dist_ctx_t *)LOAD_ADDR_OF(dist_ctx);

	/*
	 * If an ITS is available, save its context before
	 * the Redistributor using:
	 * gicv3x_its_save_disable(gits_base, &its_ctx[i])
	 * Additionally, an implementation-defined sequence may
	 * be required to save the whole ITS state.
	 */

	/*
	 * Save the GIC Redistributors and ITS contexts before the
	 * Distributor context. As we only handle SYSTEM SUSPEND API,
	 * we only need to save the context of the CPU that is issuing
	 * the SYSTEM SUSPEND call, i.e. the current CPU.
	 */
	gicv3x_rdistif_save(plat_my_core_pos(), rdist_context);

	/* Save the GIC Distributor context */
	gicv3x_distif_save(dist_context);

	/*
	 * From here, all the components of the GIC can be safely powered down
	 * as long as there is an alternate way to handle wakeup interrupt
	 * sources.
	 */
}

void plat_arm_gic_resume(void)
{
	const gicv3x_redist_ctx_t *rdist_context =
			(gicv3x_redist_ctx_t *)LOAD_ADDR_OF(rdist_ctx);
	const gicv3x_dist_ctx_t *dist_context =
			(gicv3x_dist_ctx_t *)LOAD_ADDR_OF(dist_ctx);

	/* Restore the GIC Distributor context */
	gicv3x_distif_init_restore(dist_context);

	/*
	 * Restore the GIC Redistributor and ITS contexts after the
	 * Distributor context. As we only handle SYSTEM SUSPEND API,
	 * we only need to restore the context of the CPU that issued
	 * the SYSTEM SUSPEND call.
	 */
	gicv3x_rdistif_init_restore(plat_my_core_pos(), rdist_context);

	/*
	 * If an ITS is available, restore its context after
	 * the Redistributor using:
	 * gicv3x_its_restore(gits_base, &its_ctx[i])
	 * An implementation-defined sequence may be required to
	 * restore the whole ITS state. The ITS must also be
	 * re-enabled after this sequence has been executed.
	 */
}
