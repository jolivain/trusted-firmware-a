/*
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gic_common.h>

#include "../common/gic_common_private.h"
#include "gicv3x_private.h"

/* Redisributor memory map size: 128k for GICv3, 256k for GICv4 */
extern unsigned int gicr_base_offset;

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

void gicv3x_distif_pre_save(unsigned int proc_num)
{
	arm_gicv3_distif_pre_save(proc_num);
}

void gicv3x_distif_post_restore(unsigned int proc_num)
{
	arm_gicv3_distif_post_restore(proc_num);
}

/******************************************************************************
 * This function marks the core as awake in the Redistributor and
 * ensures that the interface is active.
 *****************************************************************************/
void gicv3x_rdistif_mark_core_awake(uintptr_t gicr_base)
{
	/*
	 * The WAKER_PS_BIT should be changed to 0
	 * only when WAKER_CA_BIT is 1.
	 */
	assert((gicr_read_waker(gicr_base) & WAKER_CA_BIT) != 0U);

	/* Mark the connected core as awake */
	gicr_write_waker(gicr_base, gicr_read_waker(gicr_base) & ~WAKER_PS_BIT);

	/* Wait till the WAKER_CA_BIT changes to 0 */
	while ((gicr_read_waker(gicr_base) & WAKER_CA_BIT) != 0U)
		;
}

/******************************************************************************
 * This function marks the core as asleep in the Redistributor and ensures
 * that the interface is quiescent.
 *****************************************************************************/
void gicv3x_rdistif_mark_core_asleep(uintptr_t gicr_base)
{
	/* Mark the connected core as asleep */
	gicr_write_waker(gicr_base, gicr_read_waker(gicr_base) | WAKER_PS_BIT);

	/* Wait till the WAKER_CA_BIT changes to 1 */
	while ((gicr_read_waker(gicr_base) & WAKER_CA_BIT) == 0U)
		;
}

/******************************************************************************
 * This function probes the Redistributor frames when the driver is initialised
 * and saves their base addresses. These base addresses are used later to
 * initialise each Redistributor interface.
 *****************************************************************************/
void gicv3x_rdistif_base_addrs_probe(uintptr_t *rdistif_base_addrs,
					unsigned int rdistif_num,
					uintptr_t gicr_base,
					mpidr_hash_fn mpidr_to_core_pos)
{
	u_register_t mpidr;
	unsigned int proc_num;
	uint64_t typer_val;
	uintptr_t rdistif_base = gicr_base;

	assert(rdistif_base_addrs != NULL);

	/*
	 * Iterate over the Redistributor frames. Store the base address of each
	 * frame in the platform provided array. Use the "Processor Number"
	 * field to index into the array if the platform has not provided a hash
	 * function to convert an MPIDR (obtained from the "Affinity Value"
	 * field into a linear index.
	 */
	do {
		typer_val = gicr_read_typer(rdistif_base);

		if (mpidr_to_core_pos != NULL) {
			mpidr = mpidr_from_gicr_typer(typer_val);
			proc_num = mpidr_to_core_pos(mpidr);
		} else {
			proc_num = (typer_val >> TYPER_PROC_NUM_SHIFT) &
				TYPER_PROC_NUM_MASK;
		}

		if (proc_num < rdistif_num)
			rdistif_base_addrs[proc_num] = rdistif_base;
		rdistif_base += (uintptr_t)gicr_base_offset;
	} while ((typer_val & TYPER_LAST_BIT) == 0U);
}

/******************************************************************************
 * Helper function to configure the default attributes of (E)SPIs
 *****************************************************************************/
void gicv3x_spis_config_defaults(uintptr_t gicd_base)
{
	unsigned int i, regs_num;

	/*
	 * Treat all (E)SPIs as G1NS by default
	 *
	 * The number of implemented GICD_IGROUPR<n> registers is
	 * (GICD_TYPER.ITLinesNumber + 1).
	 */
	for (i = (MIN_SPI_ID >> IGROUPR_SHIFT); i < spi_regs_num; ++i) {
		GICD_WRITE_NUM(IGROUPR, gicd_base, i, ~0U);
	}

	/*
	 * The number of implemented GICD_IGROUPR<n>E registers is
	 * (GICD_TYPER.ESPI_range + 1).
	 */
	for (i = 0; i < espi_regs_num; ++i) {
		GICD_WRITE_NUM(IGROUPRE, gicd_base, i, ~0U);
	}

	/*
	 * Setup the default (E)SPI priorities writing 4 at a time.
	 *
	 * The number of implemented GICD_IPRIORITYR<n> registers is
	 * 8*(GICD_TYPER.ITLinesNumber + 1).
	 */
	regs_num = spi_regs_num << 3;
	for (i = (MIN_SPI_ID >> IPRIORITYR_SHIFT); i < regs_num; ++i) {
		GICD_WRITE_NUM(IPRIORITYR, gicd_base, i,
						GICD_IPRIORITYR_DEF_VAL);
	}

	/*
	 * The number of implemented GICD_IPRIORITYR<n>E registers is
	 * 8*(GICD_TYPER.ESPI_range + 1).
	 */
	regs_num = espi_regs_num << 3;
	for (i = 0; i < regs_num; ++i) {
		GICD_WRITE_NUM(IPRIORITYRE, gicd_base, i,
						GICD_IPRIORITYR_DEF_VAL);
	}

	/*
	 * Treat all (E)SPIs as level triggered by default,
	 * writing 16 at a time.
	 *
	 * The number of implemented GICD_ICFGR<n> registers is
	 * 2*(GICD_TYPER.ITLinesNumber + 1).
	 */
	regs_num = spi_regs_num << 1;
	for (i = (MIN_SPI_ID >> ICFGR_SHIFT); i < regs_num; ++i) {
		GICD_WRITE_NUM(ICFGR, gicd_base, i, 0U);
	}

	/*
	 * The number of implemented GICD_ICFGR<n>E registers is
	 * 2*(GICD_TYPER.ESPI_range + 1).
	 */
	regs_num = espi_regs_num << 1;
	for (i = 0; i < regs_num; ++i) {
		GICD_WRITE_NUM(ICFGRE, gicd_base, i, 0U);
	}
}

/******************************************************************************
 * Helper function to configure properties of secure (E)SPIs
 *****************************************************************************/
unsigned int gicv3x_secure_spis_config_props(
					uintptr_t gicd_base,
					const interrupt_prop_t *interrupt_props,
					unsigned int interrupt_props_num)
{
	const interrupt_prop_t *current_prop;
	unsigned long long gic_affinity_val;
	unsigned int ctlr_enable = 0U;

	/* Make sure there's a valid property array */
	if (interrupt_props_num > 0U)
		assert(interrupt_props != NULL);

	for (unsigned int i = 0; i < interrupt_props_num; ++i) {
		current_prop = &interrupt_props[i];

		unsigned int id = current_prop->intr_num;

		/* Skip SGI and (E)PPI interrupt */
		if (!(((id >= MIN_SPI_ID) && (id <= max_spi_intid)) ||
		      ((id >= MIN_ESPI_ID) && (id <= max_espi_intid))))
			continue;

		/* Configure this interrupt as a secure interrupt */
		gicd_clr_igroupr(gicd_base, id);

		/* Configure this interrupt as G0 or a G1S interrupt */
		assert((current_prop->intr_grp == INTR_GROUP0) ||
				(current_prop->intr_grp == INTR_GROUP1S));
		if (current_prop->intr_grp == INTR_GROUP1S) {
			gicd_set_igrpmodr(gicd_base, id);
			ctlr_enable |= CTLR_ENABLE_G1S_BIT;
		} else {
			gicd_clr_igrpmodr(gicd_base, id);
			ctlr_enable |= CTLR_ENABLE_G0_BIT;
		}

		/* Set interrupt configuration */
		gicd_set_icfgr(gicd_base, id, current_prop->intr_cfg);

		/* Set the priority of this interrupt */
		gicd_set_ipriorityr(gicd_base, id, current_prop->intr_pri);

		/* Target SPIs to the primary CPU */
		gic_affinity_val =
				gicd_irouter_val_from_mpidr(read_mpidr(), 0U);
		gicd_write_irouter(gicd_base, id, gic_affinity_val);

		/* Enable this interrupt */
		gicd_set_isenabler(gicd_base, id);
	}

	return ctlr_enable;
}

/******************************************************************************
 * Helper function to configure the default attributes of (E)SPIs.
 *****************************************************************************/
void gicv3x_ppi_sgi_config_defaults(uintptr_t gicr_base)
{
	unsigned int i, regs_num;

	/*
	 * Disable all SGIs (imp. def.)/(E)PPIs before configuring them.
	 * This is a more scalable approach as it avoids clearing
	 * the enable bits in the GICD_CTLR.
	 */
	for (i = 0; i < ppi_regs_num; ++i) {
		gicr_write_icenabler(gicr_base, i, ~0U);
	}

	gicr_wait_for_pending_write(gicr_base);

	/* Treat all SGIs/(E)PPIs as G1NS by default */
	for (i = 0; i < ppi_regs_num; ++i) {
		gicr_write_igroupr(gicr_base, i, ~0U);
	}

	/* 4 interrupt IDs per GICR_IPRIORITYR register */
	regs_num = ppi_regs_num << 3;
	for (i = 0; i < regs_num; ++i) {
		/* Setup the default SGI/(E)PPI priorities doing 4 at a time */
		gicr_write_ipriorityr(gicr_base, i, GICD_IPRIORITYR_DEF_VAL);
	}

	/* 16 interrupt IDs per GICR_ICFGR register */
	regs_num = ppi_regs_num << 1;
	for (i = (MIN_PPI_ID >> ICFGR_SHIFT); i < regs_num; ++i) {
		/* Configure all (E)PPIs as level triggered by default */
		gicr_write_icfgr(gicr_base, i, 0U);
	}
}

/******************************************************************************
 * Helper function to configure properties of
 * secure G0 and G1S (E)PPIs and SGIs
 *****************************************************************************/
unsigned int gicv3x_secure_ppi_sgi_config_props(
					uintptr_t gicr_base,
					const interrupt_prop_t *interrupt_props,
					unsigned int interrupt_props_num)
{
	const interrupt_prop_t *current_prop;
	unsigned int ctlr_enable = 0U;

	/* Make sure there's a valid property array */
	if (interrupt_props_num > 0U)
		assert(interrupt_props != NULL);

	for (unsigned int i = 0; i < interrupt_props_num; ++i) {
		current_prop = &interrupt_props[i];

		unsigned int id = current_prop->intr_num;

		/* Skip (E)SPI interrupt */
		if ((id > MAX_PPI_ID) &&
		  !((id >= MIN_EPPI_ID) && (id <= max_ppi_intid)))
			continue;

		/* Configure this interrupt as a secure interrupt */
		gicr_clr_igroupr(gicr_base, id);

		/* Configure this interrupt as G0 or a G1S interrupt */
		assert((current_prop->intr_grp == INTR_GROUP0) ||
				(current_prop->intr_grp == INTR_GROUP1S));

		if (current_prop->intr_grp == INTR_GROUP1S) {
			gicr_set_igrpmodr(gicr_base, id);
			ctlr_enable |= CTLR_ENABLE_G1S_BIT;
		} else {
			gicr_clr_igrpmodr(gicr_base, id);
			ctlr_enable |= CTLR_ENABLE_G0_BIT;
		}

		/* Set the priority of this interrupt */
		gicr_set_ipriorityr(gicr_base, id, current_prop->intr_pri);

		/*
		 * Set interrupt configuration for (E)PPIs.
		 * Configuration for SGIs are ignored.
		 */
		if (((id >= MIN_PPI_ID) && (id <= MAX_PPI_ID)) ||
		    ((id >= MIN_EPPI_ID) && (id <= max_ppi_intid))) {
			gicr_set_icfgr(gicr_base, id, current_prop->intr_cfg);
		}

		/* Enable this interrupt */
		gicr_set_isenabler(gicr_base, id);
	}

	return ctlr_enable;
}
