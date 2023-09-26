/*
 * Copyright (c) 2016-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <common/tf_crc32.h>
#include <drivers/io/io_storage.h>
#include <drivers/partition/efi.h>
#include <drivers/partition/partition.h>
#include <drivers/partition/gpt.h>
#include <drivers/partition/mbr.h>
#include <plat/common/platform.h>

static uint8_t mbr_sector[PLAT_PARTITION_BLOCK_SIZE];
static partition_entry_list_t list;

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
static void dump_entries(int num)
{
	char name[EFI_NAMELEN];
	int i, j, len;

	VERBOSE("Partition table with %d entries:\n", num);
	for (i = 0; i < num; i++) {
		len = snprintf(name, EFI_NAMELEN, "%s", list.list[i].name);
		for (j = 0; j < EFI_NAMELEN - len - 1; j++) {
			name[len + j] = ' ';
		}
		name[EFI_NAMELEN - 1] = '\0';
		VERBOSE("%d: %s %" PRIx64 "-%" PRIx64 "\n", i + 1, name, list.list[i].start,
			list.list[i].start + list.list[i].length - 4);
	}
}
#else
#define dump_entries(num)	((void)num)
#endif

/*
 * Load the first sector that carries MBR header.
 * The MBR boot signature should be always valid whether it's MBR or GPT.
 */
static int load_mbr_header(uintptr_t image_handle, mbr_entry_t *mbr_entry)
{
	size_t bytes_read;
	uintptr_t offset;
	int result;

	assert(mbr_entry != NULL);
	/* MBR partition table is in LBA0. */
	result = io_seek(image_handle, IO_SEEK_SET, MBR_OFFSET);
	if (result != 0) {
		WARN("Failed to seek (%i)\n", result);
		return result;
	}
	result = io_read(image_handle, (uintptr_t)&mbr_sector,
			 PLAT_PARTITION_BLOCK_SIZE, &bytes_read);
	if (result != 0) {
		WARN("Failed to read data (%i)\n", result);
		return result;
	}

	/* Check MBR boot signature. */
	if ((mbr_sector[LEGACY_PARTITION_BLOCK_SIZE - 2] != MBR_SIGNATURE_FIRST) ||
	    (mbr_sector[LEGACY_PARTITION_BLOCK_SIZE - 1] != MBR_SIGNATURE_SECOND)) {
		WARN("MBR boot signature failure\n");
		return -ENOENT;
	}
	offset = (uintptr_t)&mbr_sector + MBR_PRIMARY_ENTRY_OFFSET;
	memcpy(mbr_entry, (void *)offset, sizeof(mbr_entry_t));
	return 0;
}

/*
 * Load GPT header and check the GPT signature and header CRC.
 * If partition numbers could be found, check & update it.
 */
static int load_gpt_header(uintptr_t image_handle, size_t header_offset,
			size_t *part_lba)
{
	gpt_header_t header;
	size_t bytes_read;
	int result;
	uint32_t header_crc, calc_crc;

	result = io_seek(image_handle, IO_SEEK_SET, header_offset);
	if (result != 0) {
		WARN("Failed to read data (%i)\n", result);
		return result;
	}
	result = io_read(image_handle, (uintptr_t)&header,
			 sizeof(gpt_header_t), &bytes_read);
	if ((result != 0) || (sizeof(gpt_header_t) != bytes_read)) {
		WARN("GPT header read error(%i) or read mismatch occurred expected(%zu) and actual(%zu)\n",
			result, sizeof(gpt_header_t), bytes_read);
		return result;
	}
	if (memcmp(header.signature, GPT_SIGNATURE,
			   sizeof(header.signature)) != 0) {
		WARN("GPT header signature failure\n");
		return -EINVAL;
	}

	/*
	 * UEFI Spec 2.8 March 2019 Page 119: HeaderCRC32 value is
	 * computed by setting this field to 0, and computing the
	 * 32-bit CRC for HeaderSize bytes.
	 */
	header_crc = header.header_crc;
	header.header_crc = 0U;

	calc_crc = tf_crc32(0U, (uint8_t *)&header, header.size);
	if (header_crc != calc_crc) {
		ERROR("Invalid GPT Header CRC: Expected 0x%x but got 0x%x.\n",
		      header_crc, calc_crc);
		return -EINVAL;
	}

	header.header_crc = header_crc;

	/* partition numbers can't exceed PLAT_PARTITION_MAX_ENTRIES */
	list.entry_count = header.list_num;
	if (list.entry_count > PLAT_PARTITION_MAX_ENTRIES) {
		list.entry_count = PLAT_PARTITION_MAX_ENTRIES;
	}

	*part_lba = header.part_lba;
	return 0;
}

/*
 * Load a single MBR entry based on details from MBR header.
 */
static int load_mbr_entry(uintptr_t image_handle, mbr_entry_t *mbr_entry,
			int part_number)
{
	size_t bytes_read;
	uintptr_t offset;
	int result;

	assert(mbr_entry != NULL);
	/* MBR partition table is in LBA0. */
	result = io_seek(image_handle, IO_SEEK_SET, MBR_OFFSET);
	if (result != 0) {
		WARN("Failed to seek (%i)\n", result);
		return result;
	}
	result = io_read(image_handle, (uintptr_t)&mbr_sector,
			 PLAT_PARTITION_BLOCK_SIZE, &bytes_read);
	if (result != 0) {
		WARN("Failed to read data (%i)\n", result);
		return result;
	}

	/* Check MBR boot signature. */
	if ((mbr_sector[LEGACY_PARTITION_BLOCK_SIZE - 2] != MBR_SIGNATURE_FIRST) ||
	    (mbr_sector[LEGACY_PARTITION_BLOCK_SIZE - 1] != MBR_SIGNATURE_SECOND)) {
		WARN("MBR Entry boot signature failure\n");
		return -ENOENT;
	}
	offset = (uintptr_t)&mbr_sector +
		MBR_PRIMARY_ENTRY_OFFSET +
		MBR_PRIMARY_ENTRY_SIZE * part_number;
	memcpy(mbr_entry, (void *)offset, sizeof(mbr_entry_t));

	return 0;
}

/*
 * Load MBR entries based on max number of partition entries.
 */
static int load_mbr_entries(uintptr_t image_handle)
{
	mbr_entry_t mbr_entry;
	int i;

	list.entry_count = MBR_PRIMARY_ENTRY_NUMBER;

	for (i = 0; i < list.entry_count; i++) {
		load_mbr_entry(image_handle, &mbr_entry, i);
		list.list[i].start = mbr_entry.first_lba * 512;
		list.list[i].length = mbr_entry.sector_nums * 512;
		list.list[i].name[0] = mbr_entry.type;
	}

	return 0;
}

/*
 * Try to read and load a single GPT entry.
 */
static int load_gpt_entry(uintptr_t image_handle, gpt_entry_t *entry)
{
	size_t bytes_read;
	int result;

	assert(entry != NULL);
	result = io_read(image_handle, (uintptr_t)entry, sizeof(gpt_entry_t),
				&bytes_read);
	if (result != 0 || sizeof(gpt_entry_t) != bytes_read) {
		WARN("GPT Entry read error(%i) or read mismatch occurred expected(%zu) and actual(%zu)\n",
			result, sizeof(gpt_entry_t), bytes_read);
		return -EINVAL;
	}

	return result;
}

/*
 * Retrieve each entry in the partition table, parse the data from each
 * entry and store them in the list of partition table entries.
 */
static int verify_partition_gpt(uintptr_t image_handle)
{
	gpt_entry_t entry;
	int result, i;

	for (i = 0; i < list.entry_count; i++) {
		result = load_gpt_entry(image_handle, &entry);
		if (result != 0) {
			WARN("Failed to load gpt entry data(%i) error is (%i)\n",
						i, result);
			return result;
		}

		result = parse_gpt_entry(&entry, &list.list[i]);
		if (result != 0) {
			break;
		}
	}
	if (i == 0) {
		WARN("No Valid GPT Entries found\n");
		return -EINVAL;
	}
	/*
	 * Only records the valid partition number that is loaded from
	 * partition table.
	 */
	list.entry_count = i;
	dump_entries(list.entry_count);

	return 0;
}

/*
 * Load a GPT partition, Try retrieving and parsing the primary GPT header,
 * if its corrupted try loading backup GPT header and then retrieve list
 * of partition table entries found from the GPT.
 */
static int load_gpt(uintptr_t image_handle, unsigned int first_lba,
			unsigned int sector_nums)
{
	int result;
	ssize_t gpt_entry_offset, gpt_header_offset;
	size_t part_lba = 0;

	/* Try to load Primary GPT header from LBA1 */
	gpt_header_offset = first_lba * PLAT_PARTITION_BLOCK_SIZE;
	result = load_gpt_header(image_handle, gpt_header_offset, &part_lba);
	if (result != 0 || part_lba == 0) {
		WARN("Failed to retrieve Primary GPT header, trying to retrieve back-up GPT header\n");
		/*
		 * Primary GPT header maybe be corrupted, so now we try with
		 * back-up GPT header, Back-up header is located in the last LBAn.
		 */
		gpt_header_offset = sector_nums * PLAT_PARTITION_BLOCK_SIZE;
		result = load_gpt_header(image_handle, gpt_header_offset,
						&part_lba);
		if (result != 0 || part_lba == 0) {
			WARN("Failed to retrieve Backup GPT header, Partition maybe corrupted\n");
			return result;
		}
	}

	gpt_entry_offset = part_lba * PLAT_PARTITION_BLOCK_SIZE;
	result = io_seek(image_handle, IO_SEEK_SET, gpt_entry_offset);
	if (result != 0) {
		WARN("Failed to seek (%i), Failed loading GPT partition table entries\n", result);
		return result;
	}

	result = verify_partition_gpt(image_handle);

	return result;
}

/*
 * Load the partition table info based on the image id provided.
 */
int load_partition_table(unsigned int image_id)
{
	uintptr_t dev_handle, image_handle, image_spec = 0;
	mbr_entry_t mbr_entry;
	int result;

	result = plat_get_image_source(image_id, &dev_handle, &image_spec);
	if (result != 0) {
		WARN("Failed to obtain reference to image id=%u (%i)\n",
			image_id, result);
		return result;
	}

	result = io_open(dev_handle, image_spec, &image_handle);
	if (result != 0) {
		WARN("Failed to access image id=%u (%i)\n", image_id, result);
		return result;
	}

	result = load_mbr_header(image_handle, &mbr_entry);
	if (result != 0) {
		WARN("Failed to access image id=%u (%i)\n", image_id, result);
		return result;
	}
	if (mbr_entry.type == PARTITION_TYPE_GPT) {
		result = load_gpt(image_handle, mbr_entry.first_lba,
					mbr_entry.sector_nums);
	} else {
		result = load_mbr_entries(image_handle);
	}

	io_close(image_handle);
	return result;
}

/*
 * Try retrieving a partition table entry based on the name of the partition.
 */
const partition_entry_t *get_partition_entry(const char *name)
{
	int i;

	for (i = 0; i < list.entry_count; i++) {
		if (strcmp(name, list.list[i].name) == 0) {
			return &list.list[i];
		}
	}
	return NULL;
}

/*
 * Try retrieving a partition table entry based on the GUID.
 */
const partition_entry_t *get_partition_entry_by_type(const uuid_t *type_uuid)
{
	int i;

	for (i = 0; i < list.entry_count; i++) {
		if (guidcmp(type_uuid, &list.list[i].type_guid) == 0) {
			return &list.list[i];
		}
	}

	return NULL;
}

/*
 * Try retrieving a partition table entry based on the UUID.
 */
const partition_entry_t *get_partition_entry_by_uuid(const uuid_t *part_uuid)
{
	int i;

	for (i = 0; i < list.entry_count; i++) {
		if (guidcmp(part_uuid, &list.list[i].part_guid) == 0) {
			return &list.list[i];
		}
	}

	return NULL;
}

/*
 * Return entry to the list of partition table entries.
 */
const partition_entry_list_t *get_partition_entry_list(void)
{
	return &list;
}

/*
 * Try loading partition table info for the given image ID.
 */
void partition_init(unsigned int image_id)
{
	load_partition_table(image_id);
}
