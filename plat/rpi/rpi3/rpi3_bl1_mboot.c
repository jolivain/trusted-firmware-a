/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2022, Linaro.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <platform_def.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <drivers/tpm/tpm2.h>
#include <drivers/tpm/tpm2_chip.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/event_log/event_log.h>
#include <drivers/measured_boot/metadata.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <common/desc_image_load.h>
#include <common/ep_info.h>


/* Event Log data */
uint8_t event_log[PLAT_ARM_EVENT_LOG_MAX_SIZE];
void rpi_delay(int us);
struct spi_plat *spidev;

/* RPI3 table with platform specific image IDs, names and PCRs */
const event_log_metadata_t rpi3_event_log_metadata[] = {
	{ FW_CONFIG_ID, MBOOT_FW_CONFIG_STRING, PCR_0 },
	{ TB_FW_CONFIG_ID, MBOOT_TB_FW_CONFIG_STRING, PCR_0 },
	{ BL2_IMAGE_ID, MBOOT_BL2_IMAGE_STRING, PCR_0 },

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


void bl1_plat_mboot_init(void)
{
	int rc;

	tpm_early_interface_setup();
	rc = tpm_interface_init(&chip_data, 0);
	if (rc != 0) {
		ERROR("BL1: TPM interface init failed\n");
	}
	rc = tpm_startup(&chip_data, TPM_SU_CLEAR);
	if (rc != 0) {
		ERROR("BL1: TPM Startup failed\n");
	}

	event_log_init(event_log, event_log + sizeof(event_log));
	event_log_write_header();
}

void bl1_plat_mboot_finish(void)
{
	size_t event_log_cur_size;
	image_desc_t *image_desc;
	entry_point_info_t *ep_info;

	event_log_cur_size = event_log_get_cur_size(event_log);
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc != NULL);

	/* Get the entry point info */
	ep_info = &image_desc->ep_info;
	ep_info->args.arg2 = (uint64_t ) event_log;
	ep_info->args.arg3 = (uint32_t) event_log_cur_size;

	// relinquish control of locality 0 and close interface
	tpm_interface_close(&chip_data, 0);
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int rc = 0;
	unsigned char hash_data[CRYPTO_MD_MAX_SIZE];
	const event_log_metadata_t *metadata_ptr = rpi3_event_log_metadata;

	rc = event_log_measure(image_data->image_base, image_data->image_size, hash_data);
	if (rc != 0) {
		return rc;
	}

	rc = tpm_pcr_extend(&chip_data, 0, TPM_ALG_ID, hash_data, TCG_DIGEST_SIZE);
	if (rc != 0) {
		ERROR("BL1: TPM PCR-0 extend failed\n");
		return rc;
	}

	while ((metadata_ptr->id != EVLOG_INVALID_ID) &&
		(metadata_ptr->id != image_id)) {
		metadata_ptr++;
	}
	assert(metadata_ptr->id != EVLOG_INVALID_ID);

	event_log_record(hash_data, EV_POST_CODE, metadata_ptr);

	// Dump Event Log for user view
	dump_event_log((uint8_t *)event_log, event_log_get_cur_size(event_log));

	return rc;
}
