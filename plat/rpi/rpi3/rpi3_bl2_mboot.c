/*
 * Copyright (c) 2022 - 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <stdarg.h>
#include <platform_def.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <drivers/tpm/tpm2_chip.h>
#include <drivers/auth/crypto_mod.h>

#include "./include/rpi3_measured_boot.h"

#include <drivers/measured_boot/event_log/event_log.h>
#include <drivers/measured_boot/metadata.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <tools_share/tbbr_oid.h>
#include <drivers/tpm/tpm2.h>

#include <assert.h>

struct spi_plat *spidev;
void rpi_delay(int us);

/* FVP table with platform specific image IDs, names and PCRs */
const event_log_metadata_t rpi3_event_log_metadata[] = {
	{ BL31_IMAGE_ID, MBOOT_BL31_IMAGE_STRING, PCR_0 },
	{ BL33_IMAGE_ID, MBOOT_BL33_IMAGE_STRING, PCR_0 },
	{ NT_FW_CONFIG_ID, MBOOT_NT_FW_CONFIG_STRING, PCR_0 },

	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

/*gpio numbers taken from https://pinout.xyz/pinout/spi*/
/* https://github.com/joholl/rpi4-uboot-tpm/blob/master/tpm-soft-spi.dts*/

chip_data_t chip_data = {
	.locality = -1,
	.address = 0,
	.timeout_msec_a = 750,
	.timeout_msec_b = 2000,
	.timeout_msec_c = 200,
	.timeout_msec_d = 30,
	.tpm_spi_data.cs_gpio = 7,
	.tpm_spi_data.sclk_gpio = 11,
	.tpm_spi_data.mosi_gpio = 10,
	.tpm_spi_data.miso_gpio = 9,
	.tpm_spi_data.reset_gpio = 24,
	.tpm_spi_data.spi_delay_us = 0,
	.tpm_spi_data.flags = 0,
	.tpm_spi_data.mode = 0,
};

void tpm_early_interface_setup()
{
	gpio_set_value(chip_data.tpm_spi_data.cs_gpio, 1);
	gpio_set_direction(chip_data.tpm_spi_data.cs_gpio, GPIO_DIR_OUT);

	gpio_set_value(chip_data.tpm_spi_data.sclk_gpio, 0);
	gpio_set_direction(chip_data.tpm_spi_data.sclk_gpio, GPIO_DIR_OUT);

	gpio_set_value(chip_data.tpm_spi_data.mosi_gpio, 1);
	gpio_set_direction(chip_data.tpm_spi_data.mosi_gpio, GPIO_DIR_OUT);

	gpio_set_direction(chip_data.tpm_spi_data.miso_gpio, GPIO_DIR_IN);

	gpio_set_value(chip_data.tpm_spi_data.reset_gpio,1);
	gpio_set_direction(chip_data.tpm_spi_data.reset_gpio, GPIO_DIR_OUT);

	gpio_set_value(chip_data.tpm_spi_data.reset_gpio,0);
	rpi_delay(1000);
	gpio_set_value(chip_data.tpm_spi_data.reset_gpio,1);
	rpi_delay(40*1000);

	spidev = spi_init(&chip_data);
}

extern uint8_t* event_log_start;
extern size_t event_log_size;

void bl2_plat_mboot_init(void)
{
	int rc;
	uint8_t *bl2_event_log_start;
	uint8_t *bl2_event_log_finish;

	tpm_early_interface_setup();
	rc = tpm_interface_init(&chip_data, 0);
	if (rc != 0) {
		ERROR("BL2: TPM interface init failed\n");
	}

	bl2_event_log_start = event_log_start + event_log_size;
	bl2_event_log_finish = event_log_start + PLAT_ARM_EVENT_LOG_MAX_SIZE;
	event_log_init(bl2_event_log_start, bl2_event_log_finish);
}

void bl2_plat_mboot_finish(void)
{
	int rc;

	// Event Log address in Non-Secure memory
	uintptr_t ns_log_addr;

	// Event Log filled size
	size_t event_log_cur_size;

	event_log_cur_size = event_log_get_cur_size((uint8_t *)event_log_start);
	rc = rpi3_set_nt_fw_info(event_log_cur_size, &ns_log_addr);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n",
		      __func__, "NT");
		//
		// It is a fatal error because on RPi U-boot assumes that
    	// a valid event log exists and will use it to record the
		// measurements into the fTPM.
		//
		panic();
	}

	// Copy Event Log to Non-secure memory
	(void)memcpy((void *)ns_log_addr, (const void *)event_log_start,
		     event_log_cur_size);

	// Ensure that the Event Log is visible in Non-secure memory
	flush_dcache_range(ns_log_addr, event_log_cur_size);

	// Dump Event Log for user view
	dump_event_log((uint8_t *)event_log_start, event_log_cur_size);

	// relinquish control of TPM locality and close interface
	tpm_interface_close(&chip_data, 0);
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int rc = 0;

	unsigned char hash_data[CRYPTO_MD_MAX_SIZE];
	const event_log_metadata_t *metadata_ptr = rpi3_event_log_metadata;

	// Measure the payload with algorithm selected by EventLog driver
	rc = event_log_measure(image_data->image_base, image_data->image_size, hash_data);
	if (rc != 0) {
		return rc;
	}

	rc = tpm_pcr_extend(&chip_data, 0, TPM_ALG_ID, hash_data, TCG_DIGEST_SIZE);
	if (rc != 0) {
		ERROR("BL2: TPM PCR-0 extend failed\n");
		return rc;
	}

	while ((metadata_ptr->id != EVLOG_INVALID_ID) &&
		(metadata_ptr->id != image_id)) {
		metadata_ptr++;
	}
	assert(metadata_ptr->id != EVLOG_INVALID_ID);

	event_log_record(hash_data, EV_POST_CODE, metadata_ptr);

	return rc;
}
