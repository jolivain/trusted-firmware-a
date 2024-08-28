/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <drivers/tpm/tpm2_chip.h>

struct spi_plat {
	int cs;
	int sclk;
	int mosi;
	int miso;
	int reset_gpio;
	int spi_delay_us;
	int flags;
    unsigned int mode;
    const struct spi_ops *ops;
};

struct spi_ops {
	int (*get_access)	(void);
	void (*start)		(void);
	void (*stop)		(void);
	int (*xfer)			(unsigned int bitlen, const void *dout, void *din);
};

struct spi_plat* spi_init(chip_data_t *chip_data);

#endif /* SPI_H */
