/*
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GIC600_MULTICHIP_H
#define GIC600_MULTICHIP_H

#include <stdint.h>

/*
 * GIC-600 microarchitecture supports coherent multichip environments containing
 * up to 16 chips of up to 64 cores each.
 */
#define GIC600_MAX_MULTICHIP	16

/*******************************************************************************
 * GIC-600 multichip data structure describes platform specific attributes
 * related to GIC-600 multichip. Platform port is expected to define these
 * attributes to initialize the multichip related registers and create
 * successful connections between the GIC-600s in a multichip system.
 *
 * The 'gicd_base' field contains the base address of the Distributor interface
 * programmer's view.
 *
 * The 'rt_owner' field contains the chip number which owns the routing table.
 * Chip number or chip_id starts from 0.
 *
 * The 'chip_count' field contains the total number of chips in a multichip
 * system. This should match the number of entries in 'chip_addrs' and 'spi_ids'
 * fields.
 *
 * The 'chip_addrs' field contains array of chip addresses. These addresses are
 * implementation specific values.
 *
 * The 'spi_ids' field contains array of minimum and maximum SPI interrupt ids
 * that each chip owns. Note that SPI interrupt ids can range for 32 to 1019.
 * If a chip doesn't own any SPI interrupts a value of {32, 32} should be
 * passed.
 ******************************************************************************/
struct gic600_multichip_data {
	uintptr_t gicd_base;
	unsigned int rt_owner;
	unsigned int chip_count;
	uint64_t chip_addrs[GIC600_MAX_MULTICHIP];
	unsigned int spi_ids[GIC600_MAX_MULTICHIP][2];
};

void gic600_multichip_init(struct gic600_multichip_data *multichip_data);
#endif /* GIC600_MULTICHIP_H */
