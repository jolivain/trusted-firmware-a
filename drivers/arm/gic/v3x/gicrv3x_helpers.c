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
#include "gicv3x_private.h"

/*******************************************************************************
 * GIC Redistributor functions
 * Note: The raw register values correspond to multiple interrupt IDs and
 * the number of interrupt IDs involved depends on the register accessed.
 ******************************************************************************/
/*
 * Accessor to get the bit corresponding to interrupt ID
 * from GIC Redistributor IGROUPR0 and IGROUPRE
 */
unsigned int gicr_get_igroupr(uintptr_t base, unsigned int id)
{
	GICR_GET_BIT(IGROUPR, base, id);
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Redistributor IGROUPR0 and IGROUPRE
 */
void gicr_set_igroupr(uintptr_t base, unsigned int id)
{
	GICR_SET_BIT(IGROUPR, base, id);
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Redistributor IGROUPR0 and IGROUPRE
 */
void gicr_clr_igroupr(uintptr_t base, unsigned int id)
{
	GICR_CLR_BIT(IGROUPR, base, id);
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * from GIC Redistributor IGRPMODR0 and IGRPMODRE
 */
unsigned int gicr_get_igrpmodr(uintptr_t base, unsigned int id)
{
	GICR_GET_BIT(IGRPMODR, base, id);
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Redistributor IGRPMODR and IGRPMODRE
 */
void gicr_set_igrpmodr(uintptr_t base, unsigned int id)
{
	GICR_SET_BIT(IGRPMODR, base, id);
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Redistributor IGRPMODR and IGRPMODRE
 */
void gicr_clr_igrpmodr(uintptr_t base, unsigned int id)
{
	GICR_CLR_BIT(IGRPMODR, base, id);
}

/*
 * Accessor to write the bit corresponding to interrupt ID
 * in GIC Redistributor ISENABLER and ISENABLERE
 */
void gicr_set_isenabler(uintptr_t base, unsigned int id)
{
	GICR_WRITE_BIT(ISENABLER, base, id);
}

/*
 * Accessor to write the bit corresponding to interrupt ID
 * in GIC Redistributor ICENABLER0 and ICENABLERE
 */
void gicr_set_icenabler(uintptr_t base, unsigned int id)
{
	GICR_WRITE_BIT(ICENABLER, base, id);
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * in GIC Redistributor ISACTIVER0 and ISACTIVERE
 */
unsigned int gicr_get_isactiver(uintptr_t base, unsigned int id)
{
	GICR_GET_BIT(ISACTIVER, base, id);
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Redistributor ICPENDR0 and ICPENDRE
 */
void gicr_set_icpendr(uintptr_t base, unsigned int id)
{
	GICR_WRITE_BIT(ICPENDR, base, id);
}

/*
 * Accessor to write the bit corresponding to interrupt ID
 * in GIC Redistributor ISPENDR0 and ISPENDRE
 */
void gicr_set_ispendr(uintptr_t base, unsigned int id)
{
	GICR_WRITE_BIT(ISPENDR, base, id);
}

/*
 * Accessor to set the byte corresponding to interrupt ID
 * in GIC Redistributor IPRIORITYR and IPRIORITYRE
 */
void gicr_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri)
{
	uint8_t val = pri & GIC_PRI_MASK;

	GICR_WRITE_8(IPRIORITYR, base, id, val);
}

/*
 * Accessor to set the bit fields corresponding to interrupt ID
 * in GIC Redistributor ICFGR0, ICFGR1 and ICFGRE
 */
void gicr_set_icfgr(uintptr_t base, unsigned int id, unsigned int cfg)
{
	/*
	 * Interrupt configuration is a 2-bit field
	 * Clear the field, and insert required configuration
	 */
	GICR_CLR_SET_BIT(ICFGR, base, id, GIC_CFG_MASK, cfg);
}

