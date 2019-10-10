/*
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GICV3X_PRIVATE_H
#define GICV3X_PRIVATE_H

#include <assert.h>
#include <stdint.h>

#include <drivers/arm/gic_common.h>

#include <drivers/arm/gicv3x.h>
#include <lib/mmio.h>

#include "../common/gic_common_private.h"

/* Constants to indicate the status of the RWP bit */
#define RWP_TRUE	U(1)
#define RWP_FALSE	U(0)

/*
 * Macro to convert an mpidr to a value suitable for programming into a
 * GICD_IROUTER. Bits[31:24] in the MPIDR are cleared as they are not relevant
 * to GICv3x.
 */
static inline u_register_t gicd_irouter_val_from_mpidr(
						u_register_t mpidr,
						unsigned int irm)
{
	return (mpidr & ~(U(0xff) << 24)) |
		((irm & IROUTER_IRM_MASK) << IROUTER_IRM_SHIFT);
}

/*
 * Macro to convert a GICR_TYPER affinity value into a MPIDR value. Bits[31:24]
 * are zeroes.
 */
#ifdef __aarch64__
static inline u_register_t mpidr_from_gicr_typer(uint64_t typer_val)
{
	return (((typer_val >> 56) & MPIDR_AFFLVL_MASK) << MPIDR_AFF3_SHIFT) |
		((typer_val >> 32) & U(0xffffff));
}
#else
static inline u_register_t mpidr_from_gicr_typer(uint64_t typer_val)
{
	return (((typer_val) >> 32) & U(0xffffff));
}
#endif

/******************************************************************************
 * GICv3.x private global variables declarations
 *****************************************************************************/
extern const gicv3x_driver_data_t *gicv3x_driver_data;

/******************************************************************************
 * Private GICv3.x function prototypes for accessing entire registers.
 * Note: The raw register values correspond to multiple interrupt IDs and
 * the number of interrupt IDs involved depends on the register accessed.
 *****************************************************************************/
unsigned int gicd_read_igrpmodr(uintptr_t base, unsigned int id);
void gicd_write_igrpmodr(uintptr_t base, unsigned int id, unsigned int val);

/******************************************************************************
 * Private GICv3.x function prototypes for accessing the GIC registers
 * corresponding to a single interrupt ID. These functions use bitwise
 * operations or appropriate register accesses to modify or return
 * the bit-field corresponding the single interrupt ID.
 *****************************************************************************/
unsigned int gicd_get_igrpmodr(uintptr_t base, unsigned int id);
unsigned int gicr_get_igrpmodr(uintptr_t base, unsigned int id);
unsigned int gicr_get_igroupr(uintptr_t base, unsigned int id);
unsigned int gicr_get_isactiver(uintptr_t base, unsigned int id);
void gicd_set_igrpmodr(uintptr_t base, unsigned int id);
void gicr_set_igrpmodr(uintptr_t base, unsigned int id);
void gicr_set_isenabler(uintptr_t base, unsigned int id);
void gicr_set_icenabler(uintptr_t base, unsigned int id);
void gicr_set_ispendr(uintptr_t base, unsigned int id);
void gicr_set_icpendr(uintptr_t base, unsigned int id);
void gicr_set_igroupr(uintptr_t base, unsigned int id);
void gicd_clr_igrpmodr(uintptr_t base, unsigned int id);
void gicr_clr_igrpmodr(uintptr_t base, unsigned int id);
void gicr_clr_igroupr(uintptr_t base, unsigned int id);
void gicr_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri);
void gicr_set_icfgr(uintptr_t base, unsigned int id, unsigned int cfg);

/******************************************************************************
 * Private GICv3.x helper function prototypes
 *****************************************************************************/
void gicv3x_spis_config_defaults(uintptr_t gicd_base);
void gicv3x_ppi_sgi_config_defaults(uintptr_t gicr_base);
unsigned int gicv3x_secure_ppi_sgi_config_props(uintptr_t gicr_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num);
unsigned int gicv3x_secure_spis_config_props(uintptr_t gicd_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num);
void gicv3x_rdistif_base_addrs_probe(uintptr_t *rdistif_base_addrs,
					unsigned int rdistif_num,
					uintptr_t gicr_base,
					mpidr_hash_fn mpidr_to_core_pos);
void gicv3x_rdistif_mark_core_awake(uintptr_t gicr_base);
void gicv3x_rdistif_mark_core_asleep(uintptr_t gicr_base);

/******************************************************************************
 * GIC Distributor interface accessors
 *****************************************************************************/
/*
 * Wait for updates to:
 * GICD_CTLR[2:0] - the Group Enables
 * GICD_CTLR[7:4] - the ARE bits, E1NWF bit and DS bit
 * GICD_ICENABLER<n> - the clearing of enable state for SPIs
 */
static inline void gicd_wait_for_pending_write(uintptr_t gicd_base)
{
	while ((gicd_read_ctlr(gicd_base) & GICD_CTLR_RWP_BIT) != 0U)
		;
}

static inline uint32_t gicd_read_pidr2(uintptr_t base)
{
	return mmio_read_32(base + GICD_PIDR2_GICV3);
}

static inline uint64_t gicd_read_irouter(uintptr_t base, unsigned int id)
{
	GICD_READ_64(IROUTER, base, id);
}

static inline void gicd_write_irouter(uintptr_t base,
				      unsigned int id,
				      uint64_t affinity)
{
	GICD_WRITE_64(IROUTER, base, id, affinity);
}

static inline void gicd_clr_ctlr(uintptr_t base, unsigned int bitmap,
					unsigned int rwp)
{
	gicd_write_ctlr(base, gicd_read_ctlr(base) & ~bitmap);
	if (rwp != 0U) {
		gicd_wait_for_pending_write(base);
	}
}

static inline void gicd_set_ctlr(uintptr_t base, unsigned int bitmap,
					unsigned int rwp)
{
	gicd_write_ctlr(base, gicd_read_ctlr(base) | bitmap);
	if (rwp != 0U) {
		gicd_wait_for_pending_write(base);
	}
}

/******************************************************************************
 * GIC Redistributor interface accessors
 *****************************************************************************/
static inline uint32_t gicr_read_ctlr(uintptr_t base)
{
	return mmio_read_32(base + GICR_CTLR);
}

static inline void gicr_write_ctlr(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GICR_CTLR, val);
}

static inline uint64_t gicr_read_typer(uintptr_t base)
{
	return mmio_read_64(base + GICR_TYPER);
}

static inline uint32_t gicr_read_waker(uintptr_t base)
{
	return mmio_read_32(base + GICR_WAKER);
}

static inline void gicr_write_waker(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GICR_WAKER, val);
}

/*
 * Wait for updates to:
 * GICR_ICENABLER0
 * GICR_CTLR.DPG1S
 * GICR_CTLR.DPG1NS
 * GICR_CTLR.DPG0
 * GICR_CTLR, which clears EnableLPIs from 1 to 0
 */
static inline void gicr_wait_for_pending_write(uintptr_t gicr_base)
{
	while ((gicr_read_ctlr(gicr_base) & GICR_CTLR_RWP_BIT) != 0U)
		;
}

static inline void gicr_wait_for_upstream_pending_write(uintptr_t gicr_base)
{
	while ((gicr_read_ctlr(gicr_base) & GICR_CTLR_UWP_BIT) != 0U)
		;
}

/* Private implementation of Distributor power control hooks */
void arm_gicv3_distif_pre_save(unsigned int rdist_proc_num);
void arm_gicv3_distif_post_restore(unsigned int rdist_proc_num);

/******************************************************************************
 * GIC Redistributor functions for accessing entire registers.
 * Note: The raw register values correspond to multiple interrupt IDs and
 * the number of interrupt IDs involved depends on the register accessed.
 *****************************************************************************/
/*
 * Accessor to read the GIC Redistributor IPRIORITYR(E) register
 * corresponding to its number, 4 interrupts IDs at a time.
 */
static inline unsigned int gicr_read_ipriorityr(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_IPRIORITYR + (reg_num << 2));
}

/*
 * Accessor to write the GIC Redistributor IPRIORITYR(E) register
 * corresponding to its number, 4 interrupts IDs at a time.
 */
static inline void gicr_write_ipriorityr(uintptr_t base, unsigned int reg_num,
						unsigned int val)
{
	mmio_write_32(base + GICR_IPRIORITYR + (reg_num << 2), val);
}

/*
 * Accessor to read GIC Redistributor ICENABLER0 and ICENABLERE
 * corresponding to its number
 */
static inline unsigned int gicr_read_icenabler(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_ICENABLER + (reg_num << 2));
}

/*
 * Accessor to write to GIC Redistributor ICENABLER0 and ICENABLERE
 * corresponding to its number
 */
static inline void gicr_write_icenabler(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ICENABLER + (reg_num << 2), val);
}

/*
 * Accessor to read GIC Redistributor ISENABLER0 and ISENABLERE
 * corresponding to its number
 */
static inline unsigned int gicr_read_isenabler(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_ISENABLER + (reg_num << 2));
}

/*
 * Accessor to write to GIC Redistributor ISENABLER0 and ISENABLERE
 * corresponding to its number
 */
static inline void gicr_write_isenabler(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ISENABLER + (reg_num << 2), val);
}

/*
 * Accessor to read GIC Redistributor IGROUPR0 and IGROUPRE register
 * corresponding to its number
 */
static inline unsigned int gicr_read_igroupr(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_IGROUPR + (reg_num << 2));
}

/*
 * Accessor to write to GIC Redistributor IGROUPR0 and IGROUPRE register
 * corresponding to its number
 */
static inline void gicr_write_igroupr(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_IGROUPR + (reg_num << 2), val);
}

/*
 * Accessor to read GIC Redistributor ISPENDR0 and ISPENDRE register
 * corresponding to its number
 */
static inline unsigned int gicr_read_ispendr(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_ISPENDR + (reg_num << 2));
}

/*
 * Accessor to write to GIC Redistributor ISPENDR0 and ISPENDRE register
 * corresponding to its number
 */
static inline void gicr_write_ispendr(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ISPENDR + (reg_num << 2), val);
}

/*
 * Accessor to read GIC Redistributor IGRPMODR0 and IGRPMODRE register
 * corresponding to its number
 */
static inline unsigned int gicr_read_igrpmodr(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_IGRPMODR + (reg_num << 2));
}

/*
 * Accessor to write to GIC Redistributor IGRPMODR0 and IGRPMODRE register
 * corresponding to its number
 */
static inline void gicr_write_igrpmodr(uintptr_t base, unsigned int reg_num,
				       unsigned int val)
{
	mmio_write_32(base + GICR_IGRPMODR + (reg_num << 2), val);
}

/*
 * Accessor to write to GIC Redistributor ICPENDR0 and ICPENDRE register
 * corresponding to its number
 */
static inline void gicr_write_icpendr(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ICPENDR + (reg_num << 2), val);
}

/*
 * Accessor to read GIC Redistributor ISACTIVER0 and ISACTIVERE register
 * corresponding to its number
 */
static inline unsigned int gicr_read_isactiver(uintptr_t base,
						unsigned int reg_num)
{
	return mmio_read_32(base + GICR_ISACTIVER + (reg_num << 2));
}

/*
 * Accessor to write to GIC Redistributor ISACTIVER0 and ISACTIVERE register
 * corresponding to its number
 */
static inline void gicr_write_isactiver(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ISACTIVER + (reg_num << 2), val);
}

static inline uint32_t gicr_read_nsacr(uintptr_t base)
{
	return mmio_read_32(base + GICR_NSACR);
}

static inline void gicr_write_nsacr(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GICR_NSACR, val);
}

/*
 * Accessor to read GIC Redistributor ICFGR0, ICFGR1 and ICFGRE register
 * corresponding to its number
 */
static inline unsigned int gicr_read_icfgr(uintptr_t base, unsigned int reg_num)
{
	return mmio_read_32(base + GICR_ICFGR + (reg_num << 2));
}

/*
 * Accessor to write to GIC Redistributor ICFGR0, ICFGR1 and ICFGRE register
 * corresponding to interrupt ID
 */
static inline void gicr_write_icfgr(uintptr_t base, unsigned int reg_num,
					unsigned int val)
{
	mmio_write_32(base + GICR_ICFGR + (reg_num << 2), val);
}

static inline uint64_t gicr_read_propbaser(uintptr_t base)
{
	return mmio_read_64(base + GICR_PROPBASER);
}

static inline void gicr_write_propbaser(uintptr_t base, uint64_t val)
{
	mmio_write_64(base + GICR_PROPBASER, val);
}

static inline uint64_t gicr_read_pendbaser(uintptr_t base)
{
	return mmio_read_64(base + GICR_PENDBASER);
}

static inline void gicr_write_pendbaser(uintptr_t base, uint64_t val)
{
	mmio_write_64(base + GICR_PENDBASER, val);
}

/******************************************************************************
 * GIC ITS functions to read and write entire ITS registers
 *****************************************************************************/
static inline uint32_t gits_read_ctlr(uintptr_t base)
{
	return mmio_read_32(base + GITS_CTLR);
}

static inline void gits_write_ctlr(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GITS_CTLR, val);
}

static inline uint64_t gits_read_cbaser(uintptr_t base)
{
	return mmio_read_64(base + GITS_CBASER);
}

static inline void gits_write_cbaser(uintptr_t base, uint64_t val)
{
	mmio_write_64(base + GITS_CBASER, val);
}

static inline uint64_t gits_read_cwriter(uintptr_t base)
{
	return mmio_read_64(base + GITS_CWRITER);
}

static inline void gits_write_cwriter(uintptr_t base, uint64_t val)
{
	mmio_write_64(base + GITS_CWRITER, val);
}

static inline uint64_t gits_read_baser(uintptr_t base,
					unsigned int its_table_id)
{
	assert(its_table_id < 8U);
	return mmio_read_64(base + GITS_BASER + (8U * its_table_id));
}

static inline void gits_write_baser(uintptr_t base, unsigned int its_table_id,
					uint64_t val)
{
	assert(its_table_id < 8U);
	mmio_write_64(base + GITS_BASER + (8U * its_table_id), val);
}

/*
 * Wait for Quiescent bit when GIC ITS is disabled
 */
static inline void gits_wait_for_quiescent_bit(uintptr_t gits_base)
{
	assert((gits_read_ctlr(gits_base) & GITS_CTLR_ENABLED_BIT) == 0U);
	while ((gits_read_ctlr(gits_base) & GITS_CTLR_QUIESCENT_BIT) == 0U)
		;
}

#endif /* GICV3X_PRIVATE_H */
