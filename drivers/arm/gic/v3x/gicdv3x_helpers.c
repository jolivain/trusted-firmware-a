/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/arm/gicv3x.h>
#include <lib/mmio.h>

/*******************************************************************************
 * GIC Distributor interface accessors for bit operations
 ******************************************************************************/
/*
 * Accessor to get the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR and IGRPMODRE.
 */
unsigned int gicd_get_igrpmodr(uintptr_t base, unsigned int id)
{
	GICD_GET_BIT(IGRPMODR, base, id);
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR and IGRPMODRE.
 */
void gicd_set_igrpmodr(uintptr_t base, unsigned int id)
{
	GICD_SET_BIT(IGRPMODR, base, id);
}

/*******************************************************************************
 * GIC Distributor interface accessors for reading entire registers
 ******************************************************************************/
/*
 * Accessor to read the GIC Distributor IGRPMODR and IGRPMODRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_igrpmodr(uintptr_t base, unsigned int id)
{
	GICD_READ(IGRPMODR, base, id);
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR and IGRPMODRE..
 */
void gicd_clr_igrpmodr(uintptr_t base, unsigned int id)
{
	GICD_CLR_BIT(IGRPMODR, base, id);
}

/*
 * Accessor to read the GIC Distributor IGROUPR and IGROUPRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_igroupr(uintptr_t base, unsigned int id)
{
	GICD_READ(IGROUPR, base, id);
}

/*
 * Accessor to read the GIC Distributor ISENABLER and ISENABLERE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_isenabler(uintptr_t base, unsigned int id)
{
	GICD_READ(ISENABLER, base, id);
}

/*
 * Accessor to read the GIC Distributor ICENABLER and ICENABLERE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_icenabler(uintptr_t base, unsigned int id)
{
	GICD_READ(ICENABLER, base, id);
}

/*
 * Accessor to read the GIC Distributor ISPENDR and ISPENDRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_ispendr(uintptr_t base, unsigned int id)
{
	GICD_READ(ISPENDR, base, id);
}

/*
 * Accessor to read the GIC Distributor ICPENDR and ICPENDRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_icpendr(uintptr_t base, unsigned int id)
{
	GICD_READ(ICPENDR, base, id);
}

/*
 * Accessor to read the GIC Distributor ISACTIVER and ISACTIVERE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_isactiver(uintptr_t base, unsigned int id)
{
	GICD_READ(ISACTIVER, base, id);
}

/*
 * Accessor to read the GIC Distributor ICACTIVER and ICACTIVERE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_icactiver(uintptr_t base, unsigned int id)
{
	GICD_READ(ICACTIVER, base, id);
}

/*
 * Accessor to read the GIC Distributor IPRIORITYR and IPRIORITYRE
 * corresponding to the interrupt ID, 4 interrupt IDs at a time.
 */
unsigned int gicd_read_ipriorityr(uintptr_t base, unsigned int id)
{
	GICD_READ(IPRIORITYR, base, id);
}

/*
 * Accessor to read the GIC Distributor ICGFR and ICGFRE
 * corresponding to the interrupt ID, 16 interrupt IDs at a time.
 */
unsigned int gicd_read_icfgr(uintptr_t base, unsigned int id)
{
	GICD_READ(ICFGR, base, id);
}

/*
 * Accessor to read the GIC Distributor NSACR and NSACRE
 * corresponding to the interrupt ID, 16 interrupt IDs at a time.
 */
unsigned int gicd_read_nsacr(uintptr_t base, unsigned int id)
{
	GICD_READ(NSACR, base, id);
}

/*******************************************************************************
 * GIC Distributor interface accessors for writing entire registers
 ******************************************************************************/
/*
 * Accessor to write the GIC Distributor IGRPMODR and IGRPMODRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
void gicd_write_igrpmodr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(IGRPMODR, base, id, val);
}

/*
 * Accessor to write the GIC Distributor IGROUPR and IGROUPRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
void gicd_write_igroupr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(IGROUPR, base, id, val);
}

/*
 * Accessor to write the GIC Distributor ISENABLER and ISENABLERE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
void gicd_write_isenabler(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ISENABLER, base, id, val);
}

/*
 * Accessor to write the GIC Distributor ICENABLER and ICENABLERE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
void gicd_write_icenabler(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ICENABLER, base, id, val);
}

/*
 * Accessor to write the GIC Distributor ISPENDR and ISPENDRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
void gicd_write_ispendr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ISPENDR, base, id, val);
}

/*
 * Accessor to write the GIC Distributor ICPENDR and ICPENDRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
void gicd_write_icpendr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ICPENDR, base, id, val);
}

/*
 * Accessor to write the GIC Distributor ISACTIVER and ISACTIVERE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
void gicd_write_isactiver(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ISACTIVER, base, id, val);
}

/*
 * Accessor to write the GIC Distributor ICACTIVER and ICACTIVERE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
void gicd_write_icactiver(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ICACTIVER, base, id, val);
}

/*
 * Accessor to write the GIC Distributor IPRIORITYR and IPRIORITYRE
 * corresponding to the interrupt ID, 4 interrupt IDs at a time.
 */
void gicd_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(IPRIORITYR, base, id, val);
}

/*
 * Accessor to write the GIC Distributor ICFGR and ICFGRE
 * corresponding to the interrupt ID, 16 interrupt IDs at a time.
 */
void gicd_write_icfgr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ICFGR, base, id, val);
}

/*
 * Accessor to write the GIC Distributor NSACR and NSACRE
 * corresponding to the interrupt ID, 16 interrupt IDs at a time.
 */
void gicd_write_nsacr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(NSACR, base, id, val);
}

/*******************************************************************************
 * GIC Distributor functions for accessing the GIC registers
 * corresponding to a single interrupt ID. These functions use bitwise
 * operations or appropriate register accesses to modify or return
 * the bit-field corresponding the single interrupt ID.
 ******************************************************************************/
unsigned int gicd_get_igroupr(uintptr_t base, unsigned int id)
{
	GICD_GET_BIT(IGROUPR, base, id);
}

void gicd_set_igroupr(uintptr_t base, unsigned int id)
{
	GICD_SET_BIT(IGROUPR, base, id);
}

void gicd_clr_igroupr(uintptr_t base, unsigned int id)
{
	GICD_CLR_BIT(IGROUPR, base, id);
}

void gicd_set_isenabler(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ISENABLER, base, id);
}

void gicd_set_icenabler(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ICENABLER, base, id);
}

void gicd_set_ispendr(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ISPENDR, base, id);
}

void gicd_set_icpendr(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ICPENDR, base, id);
}

unsigned int gicd_get_isactiver(uintptr_t base, unsigned int id)
{
	GICD_GET_BIT(ISACTIVER, base, id);
}

void gicd_set_isactiver(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ISACTIVER, base, id);
}

void gicd_set_icactiver(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ICACTIVER, base, id);
}

void gicd_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri)
{
	uint8_t val = pri & GIC_PRI_MASK;

	GICD_WRITE_8(IPRIORITYR, base, id, val);
}

void gicd_set_icfgr(uintptr_t base, unsigned int id, unsigned int cfg)
{
	/*
	 * Interrupt configuration is a 2-bit field
	 * Clear the field, and insert required configuration
	 */
	GICD_CLR_SET_BIT(ICFGR, base, id, GIC_CFG_MASK, cfg);
}
