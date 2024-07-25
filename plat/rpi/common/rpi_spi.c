/*
 * Copyright (c) 2023-2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stdint.h>
#include <platform_def.h>
#include <drivers/gpio.h>
#include <common/debug.h>
#include <drivers/spi.h>
#include "include/rpi_shared.h"
#include "drivers/rpi3/gpio/rpi3_gpio.h"

static struct spi_plat rpi_spidev;

extern void rpi_delay(int us);

int rpi_spi_miso()
{
	return gpio_get_value(rpi_spidev.miso);
}

static void rpi_spi_scl(int bit)
{
	gpio_set_value(rpi_spidev.sclk, bit);
}

static void rpi_spi_sda(int bit)
{
	gpio_set_value(rpi_spidev.mosi, bit);
}

static void rpi_spi_start()
{
	gpio_set_value(rpi_spidev.cs,1);
	gpio_set_value(rpi_spidev.sclk,0);
	gpio_set_value(rpi_spidev.cs,0);
}

static void rpi_spi_stop()
{
	gpio_set_value(rpi_spidev.cs,1);
}

static int rpi_spi_get_access()
{
	rpi_spi_scl(0);
	return 0;
}

static int rpi_spi_xfer(unsigned int bytes, const void *out, void *in)
{
	unsigned char	in_byte  = 0;
	unsigned char	out_byte = 0;
	unsigned int	j;

	for (j = 0U; j < bytes; j++) {
		int i;
		i=7;
		if (out)
			out_byte = *(const uint8_t*)out++;

		while(i >= 0) {
			rpi_spi_sda(!!(out_byte & (1 << i)));
			rpi_spi_scl(1);
			in_byte	|= rpi_spi_miso() << i;
			rpi_spi_scl(0);
			i--;
		}

		if (in)
			*(uint8_t *)in++ = in_byte;

		out_byte = 0;
		in_byte  = 0;
	}
	return 0;
}

static const struct spi_ops rpi_spidev_ops = {
	.get_access	= rpi_spi_get_access,
	.start      = rpi_spi_start,
	.stop       = rpi_spi_stop,
	.xfer		= rpi_spi_xfer,
};

struct spi_plat* spi_init(chip_data_t *chip_data)
{
    rpi_spidev.mode = chip_data->tpm_spi_data.mode;
    rpi_spidev.cs = chip_data->tpm_spi_data.cs_gpio;
    rpi_spidev.sclk = chip_data->tpm_spi_data.sclk_gpio;
    rpi_spidev.mosi = chip_data->tpm_spi_data.mosi_gpio;
    rpi_spidev.miso = chip_data->tpm_spi_data.miso_gpio;
    rpi_spidev.reset_gpio = chip_data->tpm_spi_data.reset_gpio;

	rpi_spidev.ops =  &rpi_spidev_ops;

	return &rpi_spidev;
}
