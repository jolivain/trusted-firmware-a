/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GIC600_MULTICHIP_PRIVATE_H
#define GIC600_MULTICHIP_PRIVATE_H

#include <drivers/arm/gic600_multichip.h>

#include "gicv3_private.h"

/* GIC600 GICD multichip related offsets */
#define GICD_CHIPR			U(0xC008)
#define GICD_CHIPSR			U(0xC000)
#define GICD_DCHIPR			U(0xC004)

/* GIC600 GICD multichip related masks */
#define GICD_CHIPRx_PUP_BIT		BIT_64(1)
#define GICD_CHIPRx_SOCKET_STATE	BIT_64(0)
#define GICD_DCHIPR_PUP_BIT		BIT_32(0)
#define GICD_CHIPSR_RTS_MASK		(BIT_32(4) | BIT_32(5))

/* GIC600 GICD multichip related shifts */
#define GICD_CHIPRx_ADDR_SHIFT		16
#define GICD_CHIPRx_SPI_BLOCK_MIN_SHIFT	10
#define GICD_CHIPRx_SPI_BLOCKS_SHIFT	5
#define GICD_CHIPSR_RTS_SHIFT		4

#define GICD_CHIPSR_RTS_STATE_UPDATING		0x1
#define GICD_CHIPSR_RTS_STATE_CONSISTENT	0x1

/* SPI interrupt id minimum and maximum range */
#define GICv3_SPI_ID_MIN		32
#define GICv3_SPI_ID_MAX		1019

/* Number of retries for PUP update */
#define GICD_PUP_UPDATE_RETRIES		10000

#define SPI_BLOCK_MIN_VALUE(spi_id_min) \
			((spi_id_min - GICv3_SPI_ID_MIN)/ \
			GICv3_SPI_ID_MIN)
#define SPI_BLOCKS_VALUE(spi_id_min, spi_id_max) \
			((spi_id_max - spi_id_min + 1)/ \
			GICv3_SPI_ID_MIN)
#define GICD_CHIPR_VALUE(chip_addr, spi_block_min, spi_blocks) \
			((chip_addr << GICD_CHIPRx_ADDR_SHIFT) | \
			(spi_block_min << GICD_CHIPRx_SPI_BLOCK_MIN_SHIFT) | \
			(spi_blocks << GICD_CHIPRx_SPI_BLOCKS_SHIFT))

/*******************************************************************************
 * GIC-600 multichip operation related helper functions
 ******************************************************************************/
static inline uint32_t gicd_read_dchipr(uintptr_t base)
{
	return mmio_read_32(base + GICD_DCHIPR);
}

static inline uint64_t gicd_read_chipr_n(uintptr_t base, uint8_t n)
{
	return mmio_read_64(base + (GICD_CHIPR + (8U * n)));
}

static inline uint32_t gicd_read_chipsr(uintptr_t base)
{
	return mmio_read_32(base + GICD_CHIPSR);
}

static inline void gicd_write_dchipr(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GICD_DCHIPR, val);
}

static inline void gicd_write_chipr_n(uintptr_t base, uint8_t n, uint64_t val)
{
	mmio_write_64(base + (GICD_CHIPR + (8U * n)), val);
}

#endif /* GIC600_MULTICHIP_PRIVATE_H */
