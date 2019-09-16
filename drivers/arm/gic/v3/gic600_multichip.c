/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * GIC-600 driver extension for multichip setup
 */

#include <assert.h>
#include <common/debug.h>

#include <drivers/arm/gicv3.h>
#include <drivers/arm/gic600_multichip.h>

#include "gic600_multichip_private.h"
#include "../common/gic_common_private.h"

/*******************************************************************************
 * GIC-600 multichip operation related helper functions
 ******************************************************************************/
static void gicd_dchipr_wait_for_power_update_progress(uintptr_t gicd_base)
{
	unsigned int retry = GICD_PUP_UPDATE_RETRIES;

	while ((gicd_read_dchipr(gicd_base) & GICD_DCHIPR_PUP_BIT) != 0U) {
		if (retry-- == 0) {
			ERROR("GIC-600 connection to Routing Table Owner timed "
					 "out\n");
			panic();
		}
	}
}

static void gicd_chipr_n_wait_for_power_update_progress(
					uintptr_t gicd_base,
					unsigned int chip_id)
{
	unsigned int retry = GICD_PUP_UPDATE_RETRIES;

	while ((gicd_read_chipr_n(gicd_base, chip_id) &
				GICD_CHIPRx_PUP_BIT) != 0U) {
		if (retry-- == 0) {
			ERROR("GIC-600 Routing Table register CHIPR%u failed "
					"to update\n", chip_id);
			panic();
		}
	}
}

static void gicd_chipsr_wait_for_rts_update(uintptr_t base)
{
	unsigned int retry = GICD_PUP_UPDATE_RETRIES;

	while (((gicd_read_chipsr(base) & GICD_CHIPSR_RTS_MASK) >>
				GICD_CHIPSR_RTS_SHIFT) ==
				GICD_CHIPSR_RTS_STATE_UPDATING) {
		if (retry-- == 0) {
			ERROR("GIC-600 failed to come to Consistent State\n");
			panic();
		}
	}
}

/*******************************************************************************
 * Sets up the routing table owner.
 ******************************************************************************/
static void gicd_set_dchipr_rt_owner(uintptr_t base, unsigned int rt_owner)
{
	/*
	 * Ensure that Group enables in GICD_CTLR are disabled and no pending
	 * register writes to GICD_CTLR.
	 */
	if ((gicd_read_ctlr(base) &
			(CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1S_BIT |
			 CTLR_ENABLE_G1NS_BIT | GICD_CTLR_RWP_BIT)) != 0) {
		ERROR("GICD_CTLR group enables is either enabled or have "
				"pending writes. Cannot set RT owner.\n");
		panic();
	}

	gicd_write_dchipr(base, gicd_read_dchipr(base) |
			(rt_owner << GICD_DCHIPR_RT_OWNER_SHIFT));

	/* Poll till PUP is zero */
	gicd_dchipr_wait_for_power_update_progress(base);
}

/*******************************************************************************
 * Configures the Chip Register to make connections to GICDs on
 * a multichip platform.
 ******************************************************************************/
static void gicd_set_chipr_n(uintptr_t base,
				unsigned int chip_id,
				uint64_t chip_addr,
				unsigned int spi_id_min,
				unsigned int spi_id_max)
{
	unsigned int spi_block_min, spi_blocks;
	uint64_t chipr_n_val;

	/*
	 * Ensure that group enables in GICD_CTLR are disabled and no pending
	 * register writes to GICD_CTLR.
	 */
	if ((gicd_read_ctlr(base) &
			(CTLR_ENABLE_G0_BIT | CTLR_ENABLE_G1S_BIT |
			 CTLR_ENABLE_G1NS_BIT | GICD_CTLR_RWP_BIT)) != 0) {
		ERROR("GICD_CTLR group enables is either enabled or have "
				"pending writes. Cannot set CHIPR register.\n");
		panic();
	}

	/* SPI interrupt id ranges from 32 to 1019 */
	assert((spi_id_min >= GICv3_SPI_ID_MIN) &&
			(spi_id_min <= GICv3_SPI_ID_MAX));
	assert((spi_id_max >= GICv3_SPI_ID_MIN) &&
			(spi_id_max <= GICv3_SPI_ID_MAX));
	assert(spi_id_min <= spi_id_max);

	spi_block_min = SPI_BLOCK_MIN_VALUE(spi_id_min);
	spi_blocks    = SPI_BLOCKS_VALUE(spi_id_min, spi_id_max);

	chipr_n_val = GICD_CHIPR_VALUE(chip_addr, spi_block_min, spi_blocks);

	/*
	 * Wait for DCHIPR.PUP to be zero before commencing writes to
	 * GICD_CHIPRx
	 */
	gicd_dchipr_wait_for_power_update_progress(base);

	/*
	 * Assign chip addr, spi min block, number of spi blocks and bring chip
	 * online by setting SocketState
	 */
	gicd_write_chipr_n(base, chip_id,
			(chipr_n_val | GICD_CHIPRx_SOCKET_STATE));

	/* Wait for CHIPR Power Update Progress */
	gicd_chipr_n_wait_for_power_update_progress(base, chip_id);

	/*
	 * Ensure that write to GICD_CHIPRx is successful and the chip_n came
	 * online.
	 */
	if (gicd_read_chipr_n(base, chip_id) !=
			(chipr_n_val | GICD_CHIPRx_SOCKET_STATE)) {
		ERROR("GICD_CHIPR%u write failed\n", chip_id);
		panic();
	}

	/*
	 * Poll until DCHIP.PUP is zero to verify connection to rt_owner chip
	 * is complete
	 */
	gicd_dchipr_wait_for_power_update_progress(base);

	/* Wait till the chip is in consistent state */
	gicd_chipsr_wait_for_rts_update(base);
}

void gic600_multichip_init(struct gic600_multichip_data *multichip_data)
{
	unsigned int i;

	/*
	 * Clear the "enable" bits for G0/G1S/G1NS interrupts before configuring
	 * the routing table registers.
	 */
	gicd_clr_ctlr(multichip_data->gicd_base,
		      CTLR_ENABLE_G0_BIT |
		      CTLR_ENABLE_G1S_BIT |
		      CTLR_ENABLE_G1NS_BIT,
		      RWP_TRUE);

	/* Initialize the GICD which is marked as routing table owner first */
	gicd_set_dchipr_rt_owner(multichip_data->gicd_base,
			multichip_data->rt_owner);
	gicd_set_chipr_n(multichip_data->gicd_base, multichip_data->rt_owner,
			multichip_data->chip_addrs[multichip_data->rt_owner],
			multichip_data->spi_ids[multichip_data->rt_owner][0],
			multichip_data->spi_ids[multichip_data->rt_owner][1]);

	for (i = 0; i < multichip_data->chip_count; i++) {
		if (i == multichip_data->rt_owner)
			continue;

		gicd_set_chipr_n(multichip_data->gicd_base, i,
				multichip_data->chip_addrs[i],
				multichip_data->spi_ids[i][0],
				multichip_data->spi_ids[i][1]);
	}
}
