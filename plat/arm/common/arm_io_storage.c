/*
 * Copyright (c) 2015-2021, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_storage.h>
#include <drivers/partition/partition.h>
#include <lib/utils.h>

#include <plat/arm/common/arm_fconf_getter.h>
#include <plat/arm/common/arm_fconf_io_storage.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

/* IO devices */
static const io_dev_connector_t *fip_dev_con;
uintptr_t fip_dev_handle;
static const io_dev_connector_t *memmap_dev_con;
uintptr_t memmap_dev_handle;

#if ARM_GPT_SUPPORT
/* fip partition names */
static const char * const fip_part_names[] = {"FIP_A", "FIP_B"};
CASSERT(sizeof(fip_part_names)/sizeof(char *) == NR_OF_FW_BANKS,
	assert_fip_partition_names_missing);
#endif /* ARM_GPT_SUPPORT */

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak plat_arm_io_setup
#pragma weak plat_arm_get_alt_image_source

int open_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using FIP\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

int open_memmap(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	result = io_dev_init(memmap_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(memmap_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using Memmap\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

int arm_io_setup(void)
{
	int io_result;

	io_result = register_io_dev_fip(&fip_dev_con);
	if (io_result < 0) {
		return io_result;
	}

	io_result = register_io_dev_memmap(&memmap_dev_con);
	if (io_result < 0) {
		return io_result;
	}

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
				&fip_dev_handle);
	if (io_result < 0) {
		return io_result;
	}

	io_result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
				&memmap_dev_handle);

	return io_result;
}

void plat_arm_io_setup(void)
{
	int err;

	err = arm_io_setup();
	if (err < 0) {
		panic();
	}
}

int plat_arm_get_alt_image_source(
	unsigned int image_id __unused,
	uintptr_t *dev_handle __unused,
	uintptr_t *image_spec __unused)
{
	/* By default do not try an alternative */
	return -ENOENT;
}

/* Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result;
	const struct plat_io_policy *policy;

	policy = FCONF_GET_PROPERTY(arm, io_policies, image_id);
	result = policy->check(policy->image_spec);
	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	} else {
		VERBOSE("Trying alternative IO\n");
		result = plat_arm_get_alt_image_source(image_id, dev_handle,
						       image_spec);
	}

	return result;
}

/*
 * See if a Firmware Image Package is available,
 * by checking if TOC is valid or not.
 */
bool arm_io_is_toc_valid(void)
{
	return (io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID) == 0);
}

#if ARM_GPT_SUPPORT
/**********************************************************************
 * arm_set_image_source: Set image specification in IO policy
 *
 * @image_id: id of the image whose specification to be set
 *
 * @part_name: name of the partition that to be read for entry details
 *
 * set the entry and offset details of partition in global IO policy
 * of the image
 *********************************************************************/
int arm_set_image_source(unsigned int image_id, const char *part_name)
{
	const partition_entry_t *entry = get_partition_entry(part_name);

	if (entry == NULL) {
		ERROR("Unable to find the %s partition\n", part_name);
		return -ENOENT;
	}

	const struct plat_io_policy *policy = FCONF_GET_PROPERTY(arm,
								 io_policies,
								 image_id);

	assert(policy != NULL);
	assert(policy->image_spec != 0UL);

	/* set offset and length of the image */
	io_block_spec_t *image_spec = (io_block_spec_t *)policy->image_spec;

	image_spec->offset = PLAT_ARM_FLASH_IMAGE_BASE + entry->start;
	image_spec->length = entry->length;

	return 0;
}

/*******************************************************************************
 * arm_set_fip_addr:
 *
 * Set the base address of the FIP image in its IO policy.
 *
 * @active_fw_bank_idx: active firmware bank index gathered from FWU metadata.
 ******************************************************************************/
void arm_set_fip_addr(uint32_t active_fw_bank_idx)
{
	assert(active_fw_bank_idx < NR_OF_FW_BANKS);

	INFO("Booting with partition %s\n", fip_part_names[active_fw_bank_idx]);

	int result = arm_set_image_source(FIP_IMAGE_ID,
					  fip_part_names[active_fw_bank_idx]);
	if (result != 0) {
		panic();
	}
}
#endif /* ARM_GPT_SUPPORT */

#if PSA_FWU_SUPPORT
/*******************************************************************************
 * plat_set_fwu_images_source:
 *
 * Read the FIP partition of the flash as per the active firmware bank index to
 * get the base address of the FIP image, and then update the IO policy of the
 * FIP image with this obtained address.
 *
 * @active_fw_bank_idx: active firmware bank index gathered from FWU metadata.
 ******************************************************************************/
void plat_set_fwu_images_source(uint32_t active_fw_bank_idx)
{
	arm_set_fip_addr(active_fw_bank_idx);
}

/*******************************************************************************
 * plat_set_fwu_metadata_image_source:
 *
 * Read the requested FWU metadata partition of the flash to get the base
 * address of FWU metadata image, and then update the IO policy of that
 * requested FWU metadata image with this obtained address.
 *
 * @image_id: image id of either primary or backup FWU metadata image.
 *
 * Returns 0 on success, error otherwise
 ******************************************************************************/
int plat_set_fwu_metadata_image_source(unsigned int image_id)
{
	int result = -1;

	assert((image_id == FWU_METADATA_IMAGE_ID) ||
	       (image_id == BKUP_FWU_METADATA_IMAGE_ID));

	if (image_id == FWU_METADATA_IMAGE_ID) {
		result = arm_set_image_source(FWU_METADATA_IMAGE_ID,
					      "FWU-Metadata");
	} else if (image_id == BKUP_FWU_METADATA_IMAGE_ID) {
		result = arm_set_image_source(BKUP_FWU_METADATA_IMAGE_ID,
					      "Bkup-FWU-Metadata");
	}

	return result;
}
#endif /* PSA_FWU_SUPPORT */
