/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TPM2_CHIP_H
#define TPM2_CHIP_H

#define BYTE 0x1
#define WORD 0x4

#define TPM_READ 0x1
#define TPM_WRITE 0x2

typedef struct spi_chip_data {
	uint8_t cs_gpio, sclk_gpio, mosi_gpio, miso_gpio, reset_gpio;
	unsigned long spi_delay_us;
	int flags;
	uint32_t mode;
} spi_tpm_data;

typedef struct tpm_chip_data {
	int locality;
	unsigned long timeout_msec_a, timeout_msec_b;
	unsigned long timeout_msec_c, timeout_msec_d;
	uintptr_t address;
#if (TPM_INTERFACE == FIFO_SPI)
	spi_tpm_data tpm_spi_data;
#endif
} chip_data_t;

#endif /* TPM2_CHIP_H */
