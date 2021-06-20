/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch_features.h>
#include <assert.h>

#include <common/debug.h>
#include <common/hw_crc32.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/fwu/fwu.h>
#include <drivers/io/io_storage.h>
#include "fwu_private.h"

#include <plat/common/platform.h>

CASSERT((offsetof(struct fwu_metadata, crc_32) == 0),
	crc_32_must_be_first_member_of_structure);

struct fwu_metadata metadata;

/*******************************************************************************
 * fwu_metadata_crc_check: Compute CRC32 of the FWU metadata, and check it
 *			   against CRC32 value present in the FWU metadata.
 *
 * return -1 on error, otherwise 0
 ******************************************************************************/
static int fwu_metadata_crc_check(void)
{
	unsigned char *data = (unsigned char *)&metadata;

	uint32_t calc_crc = hw_crc32(0U, data + sizeof(metadata.crc_32),
				     (sizeof(metadata) -
				      sizeof(metadata.crc_32)));

	if (metadata.crc_32 != (uint32_t)calc_crc) {
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * fwu_metadata_sanity_check: Check the sanity of FWU metadata
 *
 * return -1 on error, otherwise 0
 ******************************************************************************/
static int fwu_metadata_sanity_check(void)
{
	/* ToDo: add more conditions for sanity check */
	if ((metadata.active_index >= NR_OF_FW_BANKS) ||
	    (metadata.previous_active_index >= NR_OF_FW_BANKS)) {
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * fwu_metadata_load: Verify and load specified FWU metadata image
 *		      to local FWU metadata structure.
 *
 * @image_id: FWU metadata image id (either primary or backup)
 *
 * return -1 on error, otherwise 0
 ******************************************************************************/
int fwu_metadata_load(unsigned int image_id)
{
	int result;
	uintptr_t dev_handle, image_handle, image_spec;
	size_t bytes_read;

	result = plat_set_fwu_metadata_image_source(image_id);
	if (result != 0) {
		WARN("Failed to set reference to image id=%u (%i)\n",
		     image_id, result);
		return -1;
	}

	result = plat_get_image_source(image_id,
				       &dev_handle,
				       &image_spec);
	if (result != 0) {
		WARN("Failed to obtain reference to image id=%u (%i)\n",
		     image_id, result);
		return result;
	}

	result = io_open(dev_handle, image_spec, &image_handle);
	if (result != 0) {
		WARN("Failed to load image id id=%u (%i)\n",
		     image_id, result);
		return result;
	}

	result = io_read(image_handle, (uintptr_t)&metadata,
			 sizeof(struct fwu_metadata), &bytes_read);
	if ((result != 0) || (sizeof(struct fwu_metadata) != bytes_read)) {
		if (result == 0) {
			/* return -1 in case of partial/no read */
			result = -1;
			WARN("Read bytes (%lu) instead of expected (%lu) bytes\n",
			     bytes_read, sizeof(struct fwu_metadata));
		} else {
			WARN("Failed to read image id=%u (%i)\n",
			     FWU_METADATA_IMAGE_ID, result);
		}
		goto exit;
	}

	/* sanity check on loaded parameters */
	result = fwu_metadata_sanity_check();
	if (result != 0) {
		WARN("Sanity %s\n", "check failed on FWU metadata");
		goto exit;
	}

	/* CRC check on loaded parameters */
	result = fwu_metadata_crc_check();
	if (result != 0) {
		WARN("CRC %s\n", "check failed on FWU metadata");
		goto exit;
	}

exit:
	(void)io_close(image_handle);
	(void)io_dev_close(dev_handle);

	return result;
}
