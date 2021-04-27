/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FWU metadata information as per the specification:
 * https://developer.arm.com/-/media/Files/pdf/FWU-PSA-A_DEN0118_1.0ALP3.pdf
 */

#ifndef FWU_METADATA_H
#define FWU_METADATA_H

#include <stdint.h>
#include <tools_share/uuid.h>

/* Image details in a bank */
typedef struct image_bank_info {

	/* UUID of the image in this bank */
	uuid_t img_uuid;

	/* [0]: bit describing the image acceptance status –
	 *      1 means the image is accepted
	 * [31:1]: MBZ
	 */
	uint32_t accepted;

	/* reserved (MBZ) */
	uint32_t reserved;

} __packed image_bank_info_t;

/* Image entry information */
typedef struct image_entry {

	/* UUID identifying the image type */
	uuid_t img_type_uuid;

	/* UUID of the storage volume where the image is located */
	uuid_t location_uuid;

	/* Properties of images with img_type_uuid in the different FW banks */
	image_bank_info_t img_bank_info[NR_OF_FW_BANKS];

} __packed image_entry_t;

/*
 * FWU metadata filled by the updater and consumed by TF-A for
 * various purposes as below:
 * 1. Get active FW bank.
 * 2. Rollback to previous working FW bank.
 * 3. Get details about all images present in all banks.
 */
typedef struct fwu_metadata {

	/* Metadata CRC value */
	uint32_t crc_32;

	/* Metadata version */
	uint32_t version;

	/* Bank index with which device boot */
	uint32_t active_index;

	/* Previous bank index with which device boot successfully */
	uint32_t previous_active_index;

	/* Image entry information */
	image_entry_t img_entry[NR_OF_IMAGES_IN_FW_BANK];

} __packed fwu_metadata_t;

#endif /* FWU_METADATA_H */
