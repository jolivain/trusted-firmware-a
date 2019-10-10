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
#include <drivers/arm/gicv3x.h>
#include <lib/spinlock.h>

#include "gicv3x_private.h"

const gicv3x_driver_data_t *gicv3x_driver_data;

/* Redisributor memory map size: 128k for GICv3, 256k for GICv4 */
unsigned int gicr_base_offset;

/*
 * The number of instances of the GICD_IGROUPR<n>, GICD_ISENABLER<n>, etc.
 * interrupt registers
 */
unsigned int spi_regs_num;

/* Maximum SPI INTID: 32*(ITLinesNumber + 1) - 1 */
unsigned int max_spi_intid;

/*
 * The number of instances of the GICD_IGROUPRE<n>, GICD_ISENABLERE<n>, etc.
 * interrupt registers
 */
unsigned int espi_regs_num;

/* Maximum ESPI INTID: 32*(ESPI_range + 1) + 4095 */
unsigned int max_espi_intid;

/* Number of GICR_ICxxxx and GICR_ISxxxx registers supported: 1-3 */
unsigned int ppi_regs_num;

/* Maximum PPI INTID: 31, 1087 or 1119 */
unsigned int max_ppi_intid;

/*
 * Spinlock to guard registers needing read-modify-write. APIs protected by this
 * spinlock are used either at boot time (when only a single CPU is active), or
 * when the system is fully coherent.
 */
static spinlock_t gic_lock;

/*
 * Redistributor power operations are weakly bound so that they can be
 * overridden
 */
#pragma weak gicv3x_rdistif_off
#pragma weak gicv3x_rdistif_on

/* Helper macros to save and restore GICR registers to and from the context */
#define SAVE_GICR_REG(name)	\
	rdist_ctx->gicr_##name[i] = gicr_read_##name(gicr_base, i)

#define RESTORE_GICR_REG(name)	\
	gicr_write_##name(gicr_base, i, rdist_ctx->gicr_##name[i])

/* Helper macros to save and restore GICD registers to and from the context */
#define SAVE_GICD_REG(REG, name)	\
	dist_ctx->gicd_##name[i] = GICD_READ_NUM(REG, gicd_base, i)

#define SAVE_GICD_EREG(REG, name)			\
	dist_ctx->gicd_##name[i + spi_regs_num] =	\
				GICD_READ_NUM(REG##E, gicd_base, i)

#define RESTORE_GICD_REG(REG, name)	\
	GICD_WRITE_NUM(REG, gicd_base, i, dist_ctx->gicd_##name[i])

#define RESTORE_GICD_EREG(REG, name)		\
	GICD_WRITE_NUM(REG##E, gicd_base, i,	\
				dist_ctx->gicd_##name[i + spi_regs_num])

/******************************************************************************
 * This function initialises the ARM GICv3 driver in EL3 with provided platform
 * inputs.
 *****************************************************************************/
void __init gicv3x_driver_init(const gicv3x_driver_data_t *plat_driver_data)
{
	unsigned int gic_version;
	unsigned int gicv2_compat;

	assert(plat_driver_data != NULL);
	assert(plat_driver_data->gicd_base != 0U);
	assert(plat_driver_data->rdistif_num != 0U);
	assert(plat_driver_data->rdistif_base_addrs != NULL);

	assert(IS_IN_EL3());

	assert((plat_driver_data->interrupt_props_num != 0U) ?
	       (plat_driver_data->interrupt_props != NULL) : 1);

	/* Check for system register support */
#ifdef __aarch64__
	assert((read_id_aa64pfr0_el1() &
			(ID_AA64PFR0_GIC_MASK << ID_AA64PFR0_GIC_SHIFT)) != 0U);
#else
	assert((read_id_pfr1() &
			(ID_PFR1_GIC_MASK << ID_PFR1_GIC_SHIFT)) != 0U);
#endif /* __aarch64__ */

	gic_version = gicd_read_pidr2(plat_driver_data->gicd_base);
	gic_version >>=	PIDR2_ARCH_REV_SHIFT;
	gic_version &= PIDR2_ARCH_REV_MASK;

	/* Set Redistributor base offset */
	if (gic_version == ARCH_REV_GICV3) {
		gicr_base_offset = 1U << GICR_PCPUBASE_SHIFT_V3;
	} else if (gic_version == ARCH_REV_GICV4) {
		gicr_base_offset = 1U << GICR_PCPUBASE_SHIFT_V4;
	} else {
		/* The GIC version should be 3 or 4 */
		assert(false);
	}

	/*
	 * Find out whether the GIC supports the GICv2 compatibility mode.
	 * The ARE_S bit resets to 0 if supported.
	 */
	gicv2_compat = gicd_read_ctlr(plat_driver_data->gicd_base);
	gicv2_compat >>= CTLR_ARE_S_SHIFT;
	gicv2_compat = gicv2_compat & CTLR_ARE_S_MASK;

	if (plat_driver_data->gicr_base != 0U) {
		/*
		 * Find the base address of each implemented
		 * Redistributor interface.
		 * The number of interfaces should be equal to
		 * the number of CPUs in the system.
		 * The memory for saving these addresses has
		 * to be allocated by the platform port.
		 */
		gicv3x_rdistif_base_addrs_probe(
					plat_driver_data->rdistif_base_addrs,
					plat_driver_data->rdistif_num,
					plat_driver_data->gicr_base,
					plat_driver_data->mpidr_to_core_pos);

#if !HW_ASSISTED_COHERENCY
		/*
		 * Flush the rdistif_base_addrs[] contents
		 * linked to the GICv3x driver.
		 */
		flush_dcache_range((uintptr_t)(
			plat_driver_data->rdistif_base_addrs),
			plat_driver_data->rdistif_num *
			sizeof(*(plat_driver_data->rdistif_base_addrs)));
#endif
	}

	gicv3x_driver_data = plat_driver_data;

	unsigned int gicd_typer_reg = gicd_read_typer(
						plat_driver_data->gicd_base);

	/*
	 * Number of SPI related registers, GICD_IGROUPR<n>, etc. =
	 * (ITLinesNumber + 1).
	 */
	spi_regs_num = (gicd_typer_reg & TYPER_IT_LINES_NO_MASK) + 1;

	/*
	 * Calculate the maximum SPI INTID that GIC implementation supports.
	 * Maximum SPI INTID is 32*(ITLinesNumber + 1) - 1.
	 */
	max_spi_intid = (spi_regs_num << 5) - 1;

	/* Filter out special INTIDs 1020-1023 */
	if (max_spi_intid > MAX_SPI_ID) {
		max_spi_intid = MAX_SPI_ID;
	}

	/*
	 * Calculate number of ESPI related registers, GICD_IGROUPR<n>E, etc. =
	 * (ESPI_range + 1) and maximum ESPI INTID if GICD_TYPER.ESPI == 1.
	 * Maximum Extended SPI INTID is 32*(ESPI_range + 1) + 4095.
	 */
	if ((gicd_typer_reg & TYPER_ESPI) != 0U) {
		espi_regs_num = ((gicd_typer_reg >> TYPER_ESPI_RANGE_SHIFT) &
			 TYPER_ESPI_RANGE_MASK) + 1;
		max_espi_intid = (espi_regs_num << 5) + MIN_ESPI_ID - 1U;
	} else {
		espi_regs_num = 0;
		max_espi_intid = MAX_SPI_ID;
	}

	/*
	 * The GIC driver data is initialized by the primary CPU with caches
	 * enabled. When the secondary CPU boots up, it initializes the
	 * GICC/GICR interface with the caches disabled. Hence flush the
	 * driver data to ensure coherency. This is not required if the
	 * platform has HW_ASSISTED_COHERENCY enabled.
	 */
#if !HW_ASSISTED_COHERENCY
	flush_dcache_range((uintptr_t)&gicv3x_driver_data,
		sizeof(gicv3x_driver_data));
	flush_dcache_range((uintptr_t)gicv3x_driver_data,
		sizeof(*gicv3x_driver_data));
#endif
	INFO("GICv%u.x with%s legacy support detected\n", gic_version,
		(gicv2_compat == 0) ? "" : "out");
	INFO("ARM GICv%u.x driver initialized in EL3\n", gic_version);
}

/******************************************************************************
 * This function initialises the GIC distributor interface based upon the data
 * provided by the platform while initialising the driver.
 *****************************************************************************/
void __init gicv3x_distif_init(void)
{
	unsigned int bitmap;

	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);

	assert(IS_IN_EL3());

	/*
	 * Clear the "enable" bits for G0/G1S/G1NS interrupts before configuring
	 * the ARE_S bit. The Distributor might generate a system error
	 * otherwise.
	 */
	gicd_clr_ctlr(gicv3x_driver_data->gicd_base,
		      CTLR_ENABLE_G0_BIT |
		      CTLR_ENABLE_G1S_BIT |
		      CTLR_ENABLE_G1NS_BIT,
		      RWP_TRUE);

	/*
	 * Set the ARE_S and ARE_NS bit now that interrupts have been
	 * disabled
	 */
	gicd_set_ctlr(gicv3x_driver_data->gicd_base,
			CTLR_ARE_S_BIT | CTLR_ARE_NS_BIT, RWP_TRUE);

	/* Set the default attribute of all (E)SPIs */
	gicv3x_spis_config_defaults(gicv3x_driver_data->gicd_base);

	bitmap = gicv3x_secure_spis_config_props(
			gicv3x_driver_data->gicd_base,
			gicv3x_driver_data->interrupt_props,
			gicv3x_driver_data->interrupt_props_num);

	/* Enable the secure (E)SPIs now that they have been configured */
	gicd_set_ctlr(gicv3x_driver_data->gicd_base, bitmap, RWP_TRUE);
}

/******************************************************************************
 * This function initialises the GIC Redistributor interface of the calling CPU
 * (identified by the 'proc_num' parameter) based upon the data provided by the
 * platform while initialising the driver.
 *****************************************************************************/
void gicv3x_rdistif_init(unsigned int proc_num)
{
	uintptr_t gicr_base;
	unsigned int bitmap = 0U;
	uint32_t ctlr;

	assert(gicv3x_driver_data != NULL);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);

	ctlr = gicd_read_ctlr(gicv3x_driver_data->gicd_base);
	assert((ctlr & CTLR_ARE_S_BIT) != 0U);

	assert(IS_IN_EL3());

	/* Power on redistributor */
	gicv3x_rdistif_on(proc_num);

	gicr_base = gicv3x_driver_data->rdistif_base_addrs[proc_num];
	assert(gicr_base != 0U);

	/* Set the default attribute of all SGIs and PPIs */
	gicv3x_ppi_sgi_config_defaults(gicr_base);

	bitmap = gicv3x_secure_ppi_sgi_config_props(gicr_base,
			gicv3x_driver_data->interrupt_props,
			gicv3x_driver_data->interrupt_props_num);

	/* Enable interrupt groups as required, if not already */
	if ((ctlr & bitmap) != bitmap)
		gicd_set_ctlr(gicv3x_driver_data->gicd_base, bitmap, RWP_TRUE);
}

/******************************************************************************
 * Functions to perform power operations on GIC Redistributor
 *****************************************************************************/
void gicv3x_rdistif_off(unsigned int proc_num)
{
}

void gicv3x_rdistif_on(unsigned int proc_num)
{
}

/******************************************************************************
 * This function enables the GIC CPU interface of the calling CPU using only
 * system register accesses.
 *****************************************************************************/
void gicv3x_cpuif_enable(unsigned int proc_num)
{
	uintptr_t gicr_base;
	unsigned int scr_el3;
	unsigned int icc_sre_el3;

	assert(gicv3x_driver_data != NULL);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);
	assert(IS_IN_EL3());

	/* Mark the connected core as awake */
	gicr_base = gicv3x_driver_data->rdistif_base_addrs[proc_num];
	gicv3x_rdistif_mark_core_awake(gicr_base);

	/*
	 * Disable the legacy interrupt bypass.
	 *
	 * Enable system register access for EL3 and allow lower exception
	 * levels to configure the same for themselves. If the legacy mode is
	 * not supported, the SRE bit is RAO/WI.
	 */
	icc_sre_el3 = ICC_SRE_DIB_BIT | ICC_SRE_DFB_BIT |
		      ICC_SRE_EN_BIT | ICC_SRE_SRE_BIT;

	write_icc_sre_el3(read_icc_sre_el3() | icc_sre_el3);

	scr_el3 = (uint32_t)read_scr_el3();

	/*
	 * Switch to NS state to write Non secure ICC_SRE_EL1 and
	 * ICC_SRE_EL2 registers
	 */
	write_scr_el3(scr_el3 | SCR_NS_BIT);
	isb();

	write_icc_sre_el2(read_icc_sre_el2() | icc_sre_el3);
	write_icc_sre_el1(ICC_SRE_SRE_BIT);
	isb();

	/* Switch to secure state */
	write_scr_el3(scr_el3 & ~SCR_NS_BIT);
	isb();

	/* Write the secure ICC_SRE_EL1 register */
	write_icc_sre_el1(ICC_SRE_SRE_BIT);
	isb();

	/* Program the idle priority in the PMR */
	write_icc_pmr_el1(GIC_PRI_MASK);

	/* Enable Group0 interrupts */
	write_icc_igrpen0_el1(IGRPEN1_EL1_ENABLE_G0_BIT);

	/* Enable Group1 Secure interrupts */
	write_icc_igrpen1_el3(read_icc_igrpen1_el3() |
				IGRPEN1_EL3_ENABLE_G1S_BIT);
	isb();
}

/******************************************************************************
 * This function disables the GIC CPU interface of the calling CPU using
 * only system register accesses.
 *****************************************************************************/
void gicv3x_cpuif_disable(unsigned int proc_num)
{
	uintptr_t gicr_base;

	assert(gicv3x_driver_data != NULL);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);

	assert(IS_IN_EL3());

	/* Disable legacy interrupt bypass */
	write_icc_sre_el3(read_icc_sre_el3() |
			  (ICC_SRE_DIB_BIT | ICC_SRE_DFB_BIT));

	/* Disable Group0 interrupts */
	write_icc_igrpen0_el1(read_icc_igrpen0_el1() &
			      ~IGRPEN1_EL1_ENABLE_G0_BIT);

	/* Disable Group1 Secure and Non-Secure interrupts */
	write_icc_igrpen1_el3(read_icc_igrpen1_el3() &
			      ~(IGRPEN1_EL3_ENABLE_G1NS_BIT |
			      IGRPEN1_EL3_ENABLE_G1S_BIT));

	/* Synchronise accesses to group enable registers */
	isb();

	/* Mark the connected core as asleep */
	gicr_base = gicv3x_driver_data->rdistif_base_addrs[proc_num];
	assert(gicr_base != 0U);
	gicv3x_rdistif_mark_core_asleep(gicr_base);
}

/******************************************************************************
 * This function returns the id of the highest priority pending interrupt at
 * the GIC CPU interface
 *****************************************************************************/
unsigned int gicv3x_get_pending_interrupt_id(void)
{
	unsigned int id;

	assert(IS_IN_EL3());
	id = read_icc_hppir0_el1() & HPPIR0_EL1_INTID_MASK;

	/*
	 * If the ID is special identifier corresponding to G1S or G1NS
	 * interrupt, then read the highest pending group 1 interrupt.
	 */
	if ((id == PENDING_G1S_INTID) || (id == PENDING_G1NS_INTID))
		return read_icc_hppir1_el1() & HPPIR1_EL1_INTID_MASK;

	return id;
}

/******************************************************************************
 * This function returns the type of the highest priority pending interrupt at
 * the GIC CPU interface. The return values can be one of the following:
 * 0 - 1019                     : The interrupt type is secure Group 0.
 * PENDING_G1S_INTID 1020	: The interrupt type is secure Group 1.
 * PENDING_G1NS_INTID 1021      : The interrupt type is non secure Group 1.
 * GIC_SPURIOUS_INTERRUPT 1023  : There is no pending interrupt with sufficient
 *				  priority to be signaled to the PE.
 *****************************************************************************/
unsigned int gicv3x_get_pending_interrupt_type(void)
{
	assert(IS_IN_EL3());
	return (unsigned int)read_icc_hppir0_el1() & HPPIR0_EL1_INTID_MASK;
}

/******************************************************************************
 * This function returns the type of the interrupt id depending upon the group
 * this interrupt has been configured under by the interrupt controller i.e.
 * group0 or group1 Secure / Non Secure. The return value can be one of the
 * following :
 *    INTR_GROUP0  : The interrupt type is a Secure Group 0 interrupt
 *    INTR_GROUP1S : The interrupt type is a Secure Group 1 secure interrupt
 *    INTR_GROUP1NS: The interrupt type is a Secure Group 1 non secure
 *                   interrupt.
 *****************************************************************************/
unsigned int gicv3x_get_interrupt_type(unsigned int id, unsigned int proc_num)
{
	unsigned int igroup, grpmodr;
	uintptr_t gicr_base;

	assert(IS_IN_EL3());
	assert(gicv3x_driver_data != NULL);

	/* Ensure the parameters are valid */
	assert(proc_num < gicv3x_driver_data->rdistif_num);

	/* All LPI interrupts are Group 1 non secure */
	if (id >= MIN_LPI_ID)
		return INTR_GROUP1NS;

	/* SGI: 0-15, PPI: 16-31, EPPI: 1056-1119 */
	if ((id <= MAX_PPI_ID) ||
	   ((id >= MIN_EPPI_ID) && (id <= max_ppi_intid))) {
		assert(gicv3x_driver_data->rdistif_base_addrs != NULL);
		gicr_base = gicv3x_driver_data->rdistif_base_addrs[proc_num];
		igroup = gicr_get_igroupr(gicr_base, id);
		grpmodr = gicr_get_igrpmodr(gicr_base, id);

	/* SPI: 32-1019, ESPI: 4096-5119 */
	} else  if ((id <= max_spi_intid) ||
		   ((id >= MIN_ESPI_ID) && (id <= max_espi_intid))) {
		assert(gicv3x_driver_data->gicd_base != 0U);
		igroup = gicd_get_igroupr(gicv3x_driver_data->gicd_base, id);
		grpmodr = gicd_get_igrpmodr(gicv3x_driver_data->gicd_base, id);
	} else {
		assert(false);
		panic();
	}

	/*
	 * If the IGROUP bit is set, then it is a Group 1 Non secure interrupt
	 */
	if (igroup != 0U)
		return INTR_GROUP1NS;

	/* If the GRPMOD bit is set, then it is a Group 1 Secure interrupt */
	if (grpmodr != 0U)
		return INTR_GROUP1S;

	/* Else it is a Group 0 Secure interrupt */
	return INTR_GROUP0;
}

/*****************************************************************************
 * Function to save and disable the GIC ITS register context. The power
 * management of GIC ITS is implementation-defined and this function doesn't
 * save any memory structures required to support ITS. As the sequence to save
 * this state is implementation defined, it should be executed in platform
 * specific code. Calling this function alone and then powering down the GIC
 * and ITS without implementing the aforementioned platform specific code will
 * corrupt the ITS state.
 *
 * This function must be invoked after the GIC CPU interface is disabled.
 *****************************************************************************/
void gicv3x_its_save_disable(uintptr_t gits_base,
				gicv3x_its_ctx_t * const its_ctx)
{
	assert(gicv3x_driver_data != NULL);
	assert(IS_IN_EL3());
	assert(its_ctx != NULL);
	assert(gits_base != 0U);

	its_ctx->gits_ctlr = gits_read_ctlr(gits_base);

	/* Disable the ITS */
	gits_write_ctlr(gits_base,
			its_ctx->gits_ctlr & ~GITS_CTLR_ENABLED_BIT);

	/* Wait for quiescent state */
	gits_wait_for_quiescent_bit(gits_base);

	its_ctx->gits_cbaser = gits_read_cbaser(gits_base);
	its_ctx->gits_cwriter = gits_read_cwriter(gits_base);

	for (unsigned int i = 0; i < ARRAY_SIZE(its_ctx->gits_baser); ++i)
		its_ctx->gits_baser[i] = gits_read_baser(gits_base, i);
}

/*****************************************************************************
 * Function to restore the GIC ITS register context. The power
 * management of GIC ITS is implementation defined and this function doesn't
 * restore any memory structures required to support ITS. The assumption is
 * that these structures are in memory and are retained during system suspend.
 *
 * This must be invoked before the GIC CPU interface is enabled.
 *****************************************************************************/
void gicv3x_its_restore(uintptr_t gits_base,
			const gicv3x_its_ctx_t * const its_ctx)
{
	assert(gicv3x_driver_data != NULL);
	assert(IS_IN_EL3());
	assert(its_ctx != NULL);
	assert(gits_base != 0U);

	/* Assert that the ITS is disabled and quiescent */
	assert((gits_read_ctlr(gits_base) & GITS_CTLR_ENABLED_BIT) == 0U);
	assert((gits_read_ctlr(gits_base) & GITS_CTLR_QUIESCENT_BIT) != 0U);

	gits_write_cbaser(gits_base, its_ctx->gits_cbaser);
	gits_write_cwriter(gits_base, its_ctx->gits_cwriter);

	for (unsigned int i = 0; i < ARRAY_SIZE(its_ctx->gits_baser); ++i)
		gits_write_baser(gits_base, i, its_ctx->gits_baser[i]);

	/* Restore the ITS CTLR but leave the ITS disabled */
	gits_write_ctlr(gits_base,
			its_ctx->gits_ctlr & ~GITS_CTLR_ENABLED_BIT);
}

/*****************************************************************************
 * Function to save the GIC Redistributor register context. This function
 * must be invoked after CPU interface disable and prior to Distributor save.
 *****************************************************************************/
void gicv3x_rdistif_save(unsigned int proc_num,
				gicv3x_redist_ctx_t * const rdist_ctx)
{
	uintptr_t gicr_base;
	unsigned int i, regs_num;

	assert(gicv3x_driver_data != NULL);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);
	assert(IS_IN_EL3());
	assert(rdist_ctx != NULL);

	gicr_base = gicv3x_driver_data->rdistif_base_addrs[proc_num];

	/*
	 * Wait for any write to GICR_CTLR to complete before trying
	 * to save any state
	 */
	gicr_wait_for_pending_write(gicr_base);

	rdist_ctx->gicr_ctlr = gicr_read_ctlr(gicr_base);

	rdist_ctx->gicr_propbaser = gicr_read_propbaser(gicr_base);
	rdist_ctx->gicr_pendbaser = gicr_read_pendbaser(gicr_base);

	/* 32 interrupt IDs per register */
	for (i = 0; i < ppi_regs_num; ++i) {
		SAVE_GICR_REG(igroupr);
		SAVE_GICR_REG(isenabler);
		SAVE_GICR_REG(ispendr);
		SAVE_GICR_REG(isactiver);
		SAVE_GICR_REG(igrpmodr);
	}

	/* 16 interrupt IDs per GICR_ICFGR register */
	regs_num = ppi_regs_num << 1;
	for (i = 0; i < regs_num; ++i) {
		SAVE_GICR_REG(icfgr);
	}

	rdist_ctx->gicr_nsacr = gicr_read_nsacr(gicr_base);

	/* 4 interrupt IDs per GICR_IPRIORITYR register */
	regs_num = ppi_regs_num << 3;
	for (i = 0; i < regs_num; ++i) {
		SAVE_GICR_REG(ipriorityr);
	}

	/*
	 * Call the pre-save hook that implements the IMP DEF sequence that may
	 * be required on some GIC implementations. As this may need to access
	 * the Redistributor registers, we pass it proc_num.
	 */
	gicv3x_distif_pre_save(proc_num);
}

/*****************************************************************************
 * Function to restore the GIC Redistributor register context. We disable
 * LPI and per-CPU interrupts before we start restore of the Redistributor.
 * This function must be invoked after Distributor restore but prior to
 * CPU interface enable. The pending and active interrupts are restored
 * after the interrupts are fully configured and enabled.
 *****************************************************************************/
void gicv3x_rdistif_init_restore(unsigned int proc_num,
				const gicv3x_redist_ctx_t * const rdist_ctx)
{
	uintptr_t gicr_base;
	unsigned int i, num_regs;

	assert(gicv3x_driver_data != NULL);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);
	assert(IS_IN_EL3());
	assert(rdist_ctx != NULL);

	gicr_base = gicv3x_driver_data->rdistif_base_addrs[proc_num];

	/* Power on redistributor */
	gicv3x_rdistif_on(proc_num);

	/*
	 * Call the post-restore hook that implements the IMP DEF sequence that
	 * may be required on some GIC implementations. As this may need to
	 * access the Redistributor registers, we pass it proc_num.
	 */
	gicv3x_distif_post_restore(proc_num);

	/*
	 * Disable all SGIs (imp. def.)/PPIs and EPPIs before configuring them.
	 * This is a more scalable approach as it avoids clearing the enable
	 * bits in the GICD_CTLR.
	 */
	for (i = 0; i < ppi_regs_num; ++i) {
		gicr_write_icenabler(gicr_base, i, ~0U);
	}

	/* Wait for pending writes to GICR_ICENABLER */
	gicr_wait_for_pending_write(gicr_base);

	/*
	 * Disable the LPIs to avoid unpredictable behavior when writing to
	 * GICR_PROPBASER and GICR_PENDBASER.
	 */
	gicr_write_ctlr(gicr_base,
			rdist_ctx->gicr_ctlr & ~(GICR_CTLR_EN_LPIS_BIT));

	/* Restore registers' content */
	gicr_write_propbaser(gicr_base, rdist_ctx->gicr_propbaser);
	gicr_write_pendbaser(gicr_base, rdist_ctx->gicr_pendbaser);

	/* 32 interrupt IDs per register */
	for (i = 0; i < ppi_regs_num; ++i) {
		RESTORE_GICR_REG(igroupr);
		RESTORE_GICR_REG(igrpmodr);
	}

	/* 4 interrupt IDs per GICR_IPRIORITYR register */
	num_regs = ppi_regs_num << 3;
	for (i = 0; i < num_regs; ++i) {
		RESTORE_GICR_REG(ipriorityr);
	}

	/* 16 interrupt IDs per GICR_ICFGR register */
	num_regs = ppi_regs_num << 1;
	for (i = 0; i < num_regs; ++i) {
		RESTORE_GICR_REG(icfgr);
	}

	gicr_write_nsacr(gicr_base, rdist_ctx->gicr_nsacr);

	/*
	 * Restore after group and priorities are set
	 * 32 interrupt IDs per register
	 */
	for (i = 0; i < ppi_regs_num; ++i) {
		RESTORE_GICR_REG(ispendr);
		RESTORE_GICR_REG(isactiver);
	}

	/*
	 * Wait for all writes to the Distributor to complete before enabling
	 * the SGI and PPIs.
	 */
	gicr_wait_for_upstream_pending_write(gicr_base);

	/* 32 interrupt IDs per GICR_ISENABLER register */
	for (i = 0; i < ppi_regs_num; ++i) {
		RESTORE_GICR_REG(isenabler);
	}

	/*
	 * Restore GICR_CTLR.Enable_LPIs bit and wait for pending writes in
	 * case the first write to GICR_CTLR was still in flight (this write
	 * only restores GICR_CTLR.Enable_LPIs and no waiting is required for
	 * this bit).
	 */
	gicr_write_ctlr(gicr_base, rdist_ctx->gicr_ctlr);
	gicr_wait_for_pending_write(gicr_base);
}

/*****************************************************************************
 * Function to save the GIC Distributor register context. This function
 * must be invoked after CPU interface disable and Redistributor save.
 *****************************************************************************/
void gicv3x_distif_save(gicv3x_dist_ctx_t * const dist_ctx)
{
	unsigned int regs_num, i;

	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);
	assert(IS_IN_EL3());
	assert(dist_ctx != NULL);

	uintptr_t gicd_base = gicv3x_driver_data->gicd_base;

	/* Wait for pending write to complete */
	gicd_wait_for_pending_write(gicd_base);

	/* Save the GICD_CTLR */
	dist_ctx->gicd_ctlr = gicd_read_ctlr(gicd_base);

	/*
	 * Save GICD registers for INTIDs 32 - 1019(max), depending on
	 * maximum SPI INTID that the GIC implementation supports.
	 */
	/* 32 interrupt IDs per register */
	for (i = 0; i < spi_regs_num; ++i) {
		SAVE_GICD_REG(IGROUPR, igroupr);
		SAVE_GICD_REG(ISENABLER, isenabler);
		SAVE_GICD_REG(ISPENDR, ispendr);
		SAVE_GICD_REG(ISACTIVER, isactiver);
		SAVE_GICD_REG(IGRPMODR, igrpmodr);
	}

	/*
	 * Save GICD registers for INTIDs 4096 - 5119(max), depending on
	 * maximum ESPI INTID that the GIC implementation supports.
	 */
	/* 32 interrupt IDs per register */
	for (i = 0; i < espi_regs_num; ++i) {
		SAVE_GICD_EREG(IGROUPR, igroupr);
		SAVE_GICD_EREG(ISENABLER, isenabler);
		SAVE_GICD_EREG(ISPENDR, ispendr);
		SAVE_GICD_EREG(ISACTIVER, isactiver);
		SAVE_GICD_EREG(IGRPMODR, igrpmodr);
	}

	/* 16 interrupt IDs 32 - 1019 per register */
	regs_num = spi_regs_num << 1;
	for (i = 0; i < regs_num; ++i) {
		SAVE_GICD_REG(ICFGR, icfgr);
		SAVE_GICD_REG(NSACR, nsacr);
	}

	/* 16 interrupt IDs 4096 - 5119 per register */
	regs_num = espi_regs_num << 1;
	for (i = 0; i < regs_num; ++i) {
		SAVE_GICD_EREG(ICFGR, icfgr);
		SAVE_GICD_EREG(NSACR, nsacr);
	}

	/* 4 interrupt IDs 32 - 1019 per GICR_IPRIORITYR register */
	regs_num = spi_regs_num << 3;
	for (i = 0; i < regs_num; ++i) {
		SAVE_GICD_REG(IPRIORITYR, ipriorityr);
	}

	/* 4 interrupt IDs 4096 - 5119 per GICR_IPRIORITYRE register */
	regs_num = espi_regs_num << 3;
	for (i = 0; i < regs_num; ++i) {
		SAVE_GICD_EREG(IPRIORITYR, ipriorityr);
	}

	/* Save GICD_IROUTER for INTIDs 32 - 5119(max) */
	for (i = MIN_SPI_ID; i < max_spi_intid; ++i) {
		dist_ctx->gicd_irouter[i - MIN_SPI_ID] =
					gicd_read_irouter(gicd_base, i);
	}

	/*
	 * GICD_ITARGETSR<n> and GICD_SPENDSGIR<n> are RAZ/WI when
	 * GICD_CTLR.ARE_(S|NS) bits are set which is the case for our
	 * GICv3.x driver.
	 */
}

/******************************************************************************
 * Function to restore the GIC Distributor register context. We disable G0, G1S
 * and G1NS interrupt groups before we start restore of the Distributor. This
 * function must be invoked prior to Redistributor restore and CPU interface
 * enable. The pending and active interrupts are restored after the interrupts
 * are fully configured and enabled.
 *****************************************************************************/
void gicv3x_distif_init_restore(const gicv3x_dist_ctx_t * const dist_ctx)
{
	unsigned int regs_num, i;

	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);
	assert(IS_IN_EL3());
	assert(dist_ctx != NULL);

	uintptr_t gicd_base = gicv3x_driver_data->gicd_base;

	/*
	 * Clear the "enable" bits for G0/G1S/G1NS interrupts before
	 * configuring the ARE_S bit.
	 * The Distributor might generate a system error otherwise.
	 */
	gicd_clr_ctlr(gicd_base,
		      CTLR_ENABLE_G0_BIT |
		      CTLR_ENABLE_G1S_BIT |
		      CTLR_ENABLE_G1NS_BIT,
		      RWP_TRUE);

	/*
	 * Set the ARE_S and ARE_NS bit now that interrupts have been
	 * disabled
	 */
	gicd_set_ctlr(gicd_base, CTLR_ARE_S_BIT | CTLR_ARE_NS_BIT, RWP_TRUE);

	/*
	 * Restore GICD registers for INTIDs 32 - 1019(max), depending on
	 * maximum SPI INTID that the GIC implementation supports.
	 */
	/* 32 interrupt IDs per register */
	for (i = 0; i < spi_regs_num; ++i) {
		RESTORE_GICD_REG(IGROUPR, igroupr);
		RESTORE_GICD_REG(IGRPMODR, igrpmodr);
	}

	/*
	 * Restore GICD registers for INTIDs 4096 - 5119(max), depending on
	 * maximum ESPI INTID that the GIC implementation supports.
	 */
	/* 32 interrupt IDs per register */
	for (i = 0; i < espi_regs_num; ++i) {
		RESTORE_GICD_EREG(IGROUPR, igroupr);
		RESTORE_GICD_EREG(IGRPMODR, igrpmodr);
	}

	/* 16 interrupt IDs 32 - 1019 per register */
	regs_num = spi_regs_num << 1;
	for (i = 0; i < regs_num; ++i) {
		RESTORE_GICD_REG(ICFGR, icfgr);
		RESTORE_GICD_REG(NSACR, nsacr);
	}

	/* 16 interrupt IDs 4096 - 5119 per register */
	regs_num = espi_regs_num << 1;
	for (i = 0; i < regs_num; ++i) {
		RESTORE_GICD_EREG(ICFGR, icfgr);
		RESTORE_GICD_EREG(NSACR, nsacr);
	}

	/* 4 interrupt IDs 32 - 1019 per GICR_IPRIORITYR register */
	regs_num = spi_regs_num << 3;
	for (i = 0; i < regs_num; ++i) {
		RESTORE_GICD_REG(IPRIORITYR, ipriorityr);
	}

	/* 4 interrupt IDs 4096 - 5119 per GICR_IPRIORITYRE register */
	regs_num = espi_regs_num << 3;
	for (i = 0; i < regs_num; ++i) {
		RESTORE_GICD_EREG(IPRIORITYR, ipriorityr);
	}

	/* RESTORE GICD_IROUTER(E) for INTIDs 32 - 5119(max) */
	for (i = MIN_SPI_ID; i < max_spi_intid; ++i) {
		gicd_write_irouter(gicd_base, i,
				dist_ctx->gicd_irouter[i - MIN_SPI_ID]);
	}

	/*
	 * Restore ISENABLER(E), ISPENDR(E) and ISACTIVER(E) after
	 * the interrupts are configured.
	 */
	/* 32 interrupt IDs per register */
	for (i = 0; i < spi_regs_num; ++i) {
		RESTORE_GICD_REG(ISENABLER, isenabler);
		RESTORE_GICD_REG(ISPENDR, ispendr);
		RESTORE_GICD_REG(ISACTIVER, isactiver);
	}

	/* 32 interrupt IDs per register */
	for (i = 0; i < espi_regs_num; ++i) {
		RESTORE_GICD_EREG(ISENABLER, isenabler);
		RESTORE_GICD_EREG(ISPENDR, ispendr);
		RESTORE_GICD_EREG(ISACTIVER, isactiver);
	}

	/* Restore the GICD_CTLR */
	gicd_write_ctlr(gicd_base, dist_ctx->gicd_ctlr);
	gicd_wait_for_pending_write(gicd_base);
}

/*******************************************************************************
 * This function gets the priority of the interrupt the processor is currently
 * servicing.
 ******************************************************************************/
unsigned int gicv3x_get_running_priority(void)
{
	return read_icc_rpr_el1();
}

/*******************************************************************************
 * This function checks if the interrupt identified by id is active (whether the
 * state is either active, or active and pending). The proc_num is used if the
 * interrupt is SGI or (E)PPI and programs the corresponding Redistributor
 * interface.
 ******************************************************************************/
unsigned int gicv3x_get_interrupt_active(unsigned int id, unsigned int proc_num)
{
	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);

	if ((id <= MAX_PPI_ID) ||
	   ((id >= MIN_EPPI_ID) && (id <= max_ppi_intid))) {
		/* For SGIs, PPIs and EPPIs: 0-31, 1056-1119 */
		return gicr_get_isactiver(
			gicv3x_driver_data->rdistif_base_addrs[proc_num], id);
	} else if ((id <= max_spi_intid) ||
		  ((id >= MIN_ESPI_ID) && (id <= max_espi_intid))) {
		/* For SPIs and ESPIs: 32-1019, 4096-5119 */
		return gicd_get_isactiver(gicv3x_driver_data->gicd_base, id);
	}

	assert(false);
	return 0;
}

/*******************************************************************************
 * This function enables the interrupt identified by id. The proc_num
 * is used if the interrupt is SGI or (E)PPI, and programs the corresponding
 * Redistributor interface.
 ******************************************************************************/
void gicv3x_enable_interrupt(unsigned int id, unsigned int proc_num)
{
	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);

	/*
	 * Ensure that any shared variable updates depending on out of band
	 * interrupt trigger are observed before enabling interrupt.
	 */
	dsbishst();
	if ((id <= MAX_PPI_ID) ||
	   ((id >= MIN_EPPI_ID) && (id <= max_ppi_intid))) {
		/* For SGIs, PPIs and EPPIs: 0-31, 1056-1119 */
		gicr_set_isenabler(
			gicv3x_driver_data->rdistif_base_addrs[proc_num], id);
	} else if ((id <= max_spi_intid) ||
		  ((id >= MIN_ESPI_ID) && (id <= max_espi_intid))) {
		/* For SPIs and ESPIs: 32-1019, 4096-5119 */
		gicd_set_isenabler(gicv3x_driver_data->gicd_base, id);
	} else {
		assert(false);
	}
}

/*******************************************************************************
 * This function disables the interrupt identified by id. The proc_num
 * is used if the interrupt is SGI or (E)PPI, and programs the corresponding
 * Redistributor interface.
 ******************************************************************************/
void gicv3x_disable_interrupt(unsigned int id, unsigned int proc_num)
{
	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);

	/*
	 * Disable interrupt, and ensure that any shared variable updates
	 * depending on out of band interrupt trigger are observed afterwards.
	 */
	if ((id <= MAX_PPI_ID) ||
	   ((id >= MIN_EPPI_ID) && (id <= max_ppi_intid))) {
		/* For SGIs, PPIs and EPPIs: 0-31, 1056-1119 */
		gicr_set_icenabler(
			gicv3x_driver_data->rdistif_base_addrs[proc_num], id);

		/* Write to clear enable requires waiting for pending writes */
		gicr_wait_for_pending_write(
			gicv3x_driver_data->rdistif_base_addrs[proc_num]);
	} else if ((id <= max_spi_intid) ||
		  ((id >= MIN_ESPI_ID) && (id <= max_espi_intid))) {
		/* For SPIs and ESPIs: 32-1019, 4096-5119 */
		gicd_set_icenabler(gicv3x_driver_data->gicd_base, id);

		/* Write to clear enable requires waiting for pending writes */
		gicd_wait_for_pending_write(gicv3x_driver_data->gicd_base);
	} else {
		assert(false);
	}

	dsbishst();
}

/*******************************************************************************
 * This function sets the interrupt priority as supplied for the given
 * interrupt id.
 ******************************************************************************/
void gicv3x_set_interrupt_priority(unsigned int id, unsigned int proc_num,
					unsigned int priority)
{
	uintptr_t gicr_base;

	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);

	if ((id <= MAX_PPI_ID) ||
	   ((id >= MIN_EPPI_ID) && (id <= max_ppi_intid))) {
		/* For SGIs, PPIs and EPPIs: 0-31, 1056-1119 */
		gicr_base = gicv3x_driver_data->rdistif_base_addrs[proc_num];
		gicr_set_ipriorityr(gicr_base, id, priority);
	} else if ((id <= max_spi_intid) ||
		  ((id >= MIN_ESPI_ID) && (id <= max_espi_intid))) {
		/* For SPIs and ESPIs: 32-1019, 4096-5119 */
		gicd_set_ipriorityr(
				gicv3x_driver_data->gicd_base, id, priority);
	} else {
		assert(false);
	}
}

/******************************************************************************
 * This function assigns group for the interrupt identified by id. The proc_num
 * is used if the interrupt is SGI or PPI, and programs the corresponding
 * Redistributor interface. The group can be any of GICV3X_INTR_GROUP*
 *****************************************************************************/
void gicv3x_set_interrupt_type(unsigned int id, unsigned int proc_num,
				unsigned int type)
{
	bool igroup, grpmod;
	uintptr_t gicr_base;

	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);

	switch (type) {
	case INTR_GROUP1S:
		igroup = false;
		grpmod = true;
		break;
	case INTR_GROUP0:
		igroup = false;
		grpmod = false;
		break;
	case INTR_GROUP1NS:
		igroup = true;
		grpmod = false;
		break;
	default:
		assert(false);
		return;
	}

	if ((id <= MAX_PPI_ID) ||
	   ((id >= MIN_EPPI_ID) && (id <= max_ppi_intid))) {
		/* For SGIs, PPIs and EPPIs: 0-31, 1056-1119 */
		gicr_base = gicv3x_driver_data->rdistif_base_addrs[proc_num];

		igroup ? gicr_set_igroupr(gicr_base, id) :
			 gicr_clr_igroupr(gicr_base, id);
		grpmod ? gicr_set_igrpmodr(gicr_base, id) :
			 gicr_clr_igrpmodr(gicr_base, id);
	} else if ((id <= max_spi_intid) ||
		  ((id >= MIN_ESPI_ID) && (id <= max_espi_intid))) {
		/* For SPIs and ESPIs: 32-1019, 4096-5119 */

		/* Serialize read-modify-write to Distributor registers */
		spin_lock(&gic_lock);

		igroup ? gicd_set_igroupr(gicv3x_driver_data->gicd_base, id) :
			 gicd_clr_igroupr(gicv3x_driver_data->gicd_base, id);
		grpmod ? gicd_set_igrpmodr(gicv3x_driver_data->gicd_base, id) :
			 gicd_clr_igrpmodr(gicv3x_driver_data->gicd_base, id);

		spin_unlock(&gic_lock);
	} else {
		assert(false);
	}
}

/******************************************************************************
 * This function raises the specified Secure Group 0 SGI.
 *
 * The target parameter must be a valid MPIDR in the system.
 *****************************************************************************/
void gicv3x_raise_secure_g0_sgi(unsigned int sgi_num, u_register_t target)
{
	unsigned int tgt, aff3, aff2, aff1, aff0;
	uint64_t sgi_val;

	/* Verify interrupt number is in the SGI range */
	assert(sgi_num < MIN_PPI_ID);

	/* Extract affinity fields from target */
	aff0 = MPIDR_AFFLVL0_VAL(target);
	aff1 = MPIDR_AFFLVL1_VAL(target);
	aff2 = MPIDR_AFFLVL2_VAL(target);
	aff3 = MPIDR_AFFLVL3_VAL(target);

	/*
	 * Make target list from affinity 0,
	 * and ensure GICv3.x SGI can target this PE.
	 */
	assert(aff0 < GICV3X_MAX_SGI_TARGETS);
	tgt = BIT_32(aff0);

	/* Raise SGI to PE specified by its affinity */
	sgi_val = GICV3X_SGIR_VALUE(aff3, aff2, aff1, sgi_num,
					SGIR_IRM_TO_AFF, tgt);

	/*
	 * Ensure that any shared variable updates depending on out of band
	 * interrupt trigger are observed before raising SGI.
	 */
	dsbishst();
	write_icc_sgi0r_el1(sgi_val);
	isb();
}

/******************************************************************************
 * This function sets the interrupt routing for the given (E)SPI interrupt id.
 * The interrupt routing is specified in routing mode and mpidr.
 *
 * The routing mode can be either of:
 *  - GICV3X_IRM_ANY
 *  - GICV3X_IRM_PE
 *
 * The mpidr is the affinity of the PE to which the interrupt will be routed,
 * and is ignored for routing mode GICV3X_IRM_ANY.
 *****************************************************************************/
void gicv3x_set_spi_routing(unsigned int id, unsigned int irm,
				u_register_t mpidr)
{
	unsigned long long aff;
	uint64_t router;

	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);

	assert((irm == GICV3X_IRM_ANY) || (irm == GICV3X_IRM_PE));
	assert(((id >= MIN_SPI_ID) && (id <= max_spi_intid)) ||
	       ((id >= MIN_ESPI_ID) && (id <= max_espi_intid)));

	aff = gicd_irouter_val_from_mpidr(mpidr, irm);
	gicd_write_irouter(gicv3x_driver_data->gicd_base, id, aff);

	/*
	 * In implementations that do not require 1 of N distribution of SPIs,
	 * IRM might be RAZ/WI. Read back and verify IRM bit.
	 */
	if (irm == GICV3X_IRM_ANY) {
		router = gicd_read_irouter(gicv3x_driver_data->gicd_base, id);
		if (((router >> IROUTER_IRM_SHIFT) & IROUTER_IRM_MASK) == 0U) {
			ERROR("GICv3.x implementation doesn't support "
							"routing ANY\n");
			panic();
		}
	}
}

/*******************************************************************************
 * This function clears the pending status of an interrupt identified by id.
 * The proc_num is used if the interrupt is SGI or (E)PPI, and programs the
 * corresponding Redistributor interface.
 ******************************************************************************/
void gicv3x_clear_interrupt_pending(unsigned int id, unsigned int proc_num)
{
	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);

	/*
	 * Clear pending interrupt, and ensure that any shared variable updates
	 * depending on out of band interrupt trigger are observed afterwards.
	 */
	if ((id <= MAX_PPI_ID) ||
	   ((id >= MIN_EPPI_ID) && (id <= max_ppi_intid))) {
		/* For SGIs and (E)PPIs */
		gicr_set_icpendr(
			gicv3x_driver_data->rdistif_base_addrs[proc_num], id);
	} else if ((id <= max_spi_intid) ||
		  ((id >= MIN_ESPI_ID) && (id <= max_espi_intid))) {
		/* For (E)SPIs */
		gicd_set_icpendr(gicv3x_driver_data->gicd_base, id);
	} else {
		assert(false);
	}
	dsbishst();
}

/*******************************************************************************
 * This function sets the pending status of an interrupt identified by id.
 * The proc_num is used if the interrupt is SGI or (E)PPI and programs the
 * corresponding Redistributor interface.
 ******************************************************************************/
void gicv3x_set_interrupt_pending(unsigned int id, unsigned int proc_num)
{
	assert(gicv3x_driver_data != NULL);
	assert(gicv3x_driver_data->gicd_base != 0U);
	assert(proc_num < gicv3x_driver_data->rdistif_num);
	assert(gicv3x_driver_data->rdistif_base_addrs != NULL);

	/*
	 * Ensure that any shared variable updates depending on out of band
	 * interrupt trigger are observed before setting interrupt pending.
	 */
	dsbishst();
	if ((id <= MAX_PPI_ID) ||
	   ((id >= MIN_EPPI_ID) && (id <= max_ppi_intid))) {
		/* For SGIs and (E)PPIs */
		gicr_set_ispendr(
			gicv3x_driver_data->rdistif_base_addrs[proc_num], id);
	} else if ((id <= max_spi_intid) ||
		  ((id >= MIN_ESPI_ID) && (id <= max_espi_intid))) {
		/* For (E)SPIs */
		gicd_set_ispendr(gicv3x_driver_data->gicd_base, id);
	} else {
		assert(false);
	}
}

/*******************************************************************************
 * This function sets the PMR register with the supplied value.
 * Returns the original PMR.
 ******************************************************************************/
unsigned int gicv3x_set_pmr(unsigned int mask)
{
	unsigned int old_mask = read_icc_pmr_el1();

	/*
	 * Order memory updates w.r.t. PMR write, and ensure they're visible
	 * before potential out of band interrupt trigger because of PMR update.
	 * PMR system register writes are self-synchronizing, so no ISB required
	 * thereafter.
	 */
	dsbishst();
	write_icc_pmr_el1(mask);

	return old_mask;
}

/*******************************************************************************
 * This function delegates the responsibility of discovering the corresponding
 * Redistributor frames to each CPU itself. It is executed by each CPU in the
 * platform and also handles the scenario in which the frames of various CPUs
 * are not contiguous in physical memory.
 ******************************************************************************/
int gicv3x_rdistif_probe(const uintptr_t gicr_frame)
{
	u_register_t mpidr;
	unsigned int proc_num, proc_self;
	uint64_t typer_val;
	uintptr_t rdistif_base;
	bool gicr_frame_found = false;

	assert(gicv3x_driver_data->gicr_base == 0U);

	/* Ensure this function is called with Data Cache enabled */
#ifdef __aarch64__
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0U);
#else
	assert((read_sctlr() & SCTLR_C_BIT) != 0U);
#endif /* __aarch64__ */

	proc_self = gicv3x_driver_data->mpidr_to_core_pos(read_mpidr_el1());
	rdistif_base = gicr_frame;
	do {
		typer_val = gicr_read_typer(rdistif_base);
		if (gicv3x_driver_data->mpidr_to_core_pos != NULL) {
			mpidr = mpidr_from_gicr_typer(typer_val);
			proc_num = gicv3x_driver_data->mpidr_to_core_pos(mpidr);
		} else {
			proc_num = (unsigned int)(typer_val >>
				TYPER_PROC_NUM_SHIFT) & TYPER_PROC_NUM_MASK;
		}

		if (proc_num == proc_self) {
			/* The base address doesn't need to be initialized on
			 * every warm boot.
			 */
			if (gicv3x_driver_data->rdistif_base_addrs[proc_num] != 0U)
				return 0;

			gicv3x_driver_data->rdistif_base_addrs[proc_num] =
								rdistif_base;
			gicr_frame_found = true;
			break;
		}
		rdistif_base += (uintptr_t)gicr_base_offset;

	} while ((typer_val & TYPER_LAST_BIT) == 0U);

	if (!gicr_frame_found)
		return -1;

	/*
	 * Flush the driver data to ensure coherency. This is
	 * not required if platform has HW_ASSISTED_COHERENCY
	 * enabled.
	 */
#if !HW_ASSISTED_COHERENCY
	/*
	 * Flush the rdistif_base_addrs[] contents linked to the GICv3 driver.
	 */
	flush_dcache_range((uintptr_t)&(
			gicv3x_driver_data->rdistif_base_addrs[proc_num]),
			sizeof(*(gicv3x_driver_data->rdistif_base_addrs)));
#endif
	return 0; /* Found matching GICR frame */
}
