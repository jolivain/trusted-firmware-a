/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch_helpers.h>

#include <common/bl_common.h>
#include <common/debug.h>

#include <drivers/auth/crypto_mod.h>
#include <drivers/measured_boot/event_log.h>

/* Mbed TLS headers */
#include <mbedtls/md.h>

#include <plat/arm/common/plat_arm.h>

/* Event Log data */
static uint8_t event_log[EVENT_LOG_SIZE];

/* Pointer in event_log[] */
static uint8_t *log_ptr = event_log;

static uintptr_t tos_fw_config_base;
static uintptr_t nt_fw_config_base;

int dump_event_log(uint8_t *log_addr, size_t log_size);

void *event2_print(void *data);

static const image_data_t images_data[] = {
	{ BL31_IMAGE_ID, BL31_STRING },
	{ BL32_IMAGE_ID, BL32_STRING },
	{ BL32_EXTRA1_IMAGE_ID, BL32_EXTRA1_IMAGE_STRING },
	{ BL32_EXTRA2_IMAGE_ID, BL32_EXTRA2_IMAGE_STRING },
	{ BL33_IMAGE_ID, BL33_STRING },
	{ HW_CONFIG_ID, HW_CONFIG_STRING },
	{ NT_FW_CONFIG_ID, NT_FW_CONFIG_STRING },
	{ SCP_BL2_IMAGE_ID, SCP_BL2_IMAGE_STRING },
	{ SOC_FW_CONFIG_ID, SOC_FW_CONFIG_STRING },
	{ TOS_FW_CONFIG_ID, TOS_FW_CONFIG_STRING }
};

/* TCG_EfiSpecIdEvent */
static const id_event_headers_t id_event_headers = {
	.header = {
		.pcr_index = PCR_0,
		.event_type = EV_NO_ACTION,
		.digest = {0},
		.event_size = sizeof(id_event_struct_t) +
				sizeof(id_event_algorithm_size_t) *
				HASH_ALG_COUNT
	},

	.struct_header = {
		.signature = TCG_ID_EVENT_SIGNATURE_03,
		.platform_class = PLATFORM_CLASS_CLIENT,
		.spec_version_minor = TCG_SPEC_VERSION_MINOR_TPM2,
		.spec_version_major = TCG_SPEC_VERSION_MAJOR_TPM2,
		.spec_errata = TCG_SPEC_ERRATA_TPM2,
		.uintn_size = sizeof(unsigned int) / sizeof(uint32_t),
		.number_of_algorithms = HASH_ALG_COUNT
	}
};

static const event2_header_t locality_event_header = {
		/*
		 * All EV_NO_ACTION events SHALL set
		 * TCG_PCR_EVENT2.pcrIndex = 0, unless otherwise specified
		 */
		.pcr_index = PCR_0,

		/*
		 * All EV_NO_ACTION events SHALL set
		 * TCG_PCR_EVENT2.eventType = 03h
		 */
		.event_type = EV_NO_ACTION,

		/*
		 * All EV_NO_ACTION events SHALL set
		 * TCG_PCR_EVENT2.digests to all
		 * 0x00's for each allocated Hash algorithm
		 */
		.digests = {
			.count = HASH_ALG_COUNT
		}
};

static int add_event2(uint8_t *hash, const char *name, uint32_t image_id)
{
	uint8_t *ptr = log_ptr;
	unsigned int name_len = strlen(name) + 1;
	size_t event_size = (EVENT2_HDR_SIZE + name_len);

	/* Check for space in Event Log buffer */
	if (ptr - event_log + event_size > sizeof(event_log)) {
		ERROR("%s: Event Log is out of memory. "
			"%lu bytes requested, %lu left\n",
			__func__, event_size,
			sizeof(event_log) - (ptr - event_log));
		return -ENOMEM;
	}

	/*
	 * As per TCG specifications, firmware components that are measured
	 * into PCR[0] must be logged in the event log using the event type
	 * EV_POST_CODE.
	 */
	/* TCG_PCR_EVENT2.PCRIndex */
	((event2_header_t *)ptr)->pcr_index = PCR_0;

	/* TCG_PCR_EVENT2.EventType */
	((event2_header_t *)ptr)->event_type = EV_POST_CODE;

	/* TCG_PCR_EVENT2.Digests.Count */
	ptr += offsetof(event2_header_t, digests);
	((tpml_digest_values *)ptr)->count = HASH_ALG_COUNT;

	/* TCG_PCR_EVENT2.Digests[] */
	ptr += offsetof(tpml_digest_values, digests);

	/* TCG_PCR_EVENT2.Digests[].AlgorithmId */
	((tpmt_ha *)ptr)->algorithm_id = TPM_ALG_ID;

	/* TCG_PCR_EVENT2.Digests[].Digest[] */
	ptr += offsetof(tpmt_ha, digest);

	if (hash == NULL) {
		/* Get BL2 hash from DTB */
		arm_bl2_get_hash(ptr);
	} else {
		/* Copy digest */
		(void)memcpy(ptr, hash, TCG_DIGEST_SIZE);
	}

	/* TCG_PCR_EVENT2.EventSize */
	ptr += TCG_DIGEST_SIZE;
	((event2_data_t *)ptr)->event_size = name_len;

	/* Copy event data to TCG_PCR_EVENT2.Event */
	(void)memcpy(&((event2_data_t *)ptr)->event, name, name_len);

	/* End of event data */
	ptr += offsetof(event2_data_t, event) + name_len;

	log_ptr = ptr;

	return 0;
}

void event_log_init(void)
{
	uint8_t *ptr = event_log;
	uint8_t *start_ptr = ptr;
	int rc;

	if (sizeof(event_log) < LOG_MIN_SIZE) {
		assert(false);
		panic();
	}

	/*
	 * Add Specification ID Event first
	 *
	 * Copy TCG_EfiSpecIDEventStruct structure header
	 */
	(void)memcpy(ptr, &id_event_headers, sizeof(id_event_headers));
	ptr += sizeof(id_event_headers);

	/* TCG_EfiSpecIdEventAlgorithmSize structure */
	((id_event_algorithm_size_t *)ptr)->algorithm_id = TPM_ALG_ID;
	((id_event_algorithm_size_t *)ptr)->digest_size = TCG_DIGEST_SIZE;
	ptr += sizeof(id_event_algorithm_size_t);

	/*
	 * TCG_EfiSpecIDEventStruct.vendorInfoSize
	 * No vendor data
	 */
	((id_event_struct_data_t *)ptr)->vendor_info_size = 0;
	ptr += offsetof(id_event_struct_data_t, vendor_info);
	if ((ptr - start_ptr) != ID_EVENT_SIZE) {
		assert(false);
		panic();
	}

	start_ptr = ptr;

	/*
	 * The Startup Locality event should be placed in the log before
	 * any event which extends PCR[0].
	 *
	 * Ref. TCG PC Client Platform Firmware Profile 9.4.5.3
	 */

	/* Copy Startup Locality Event Header */
	(void)memcpy(ptr, &locality_event_header,
			sizeof(locality_event_header));
	ptr += sizeof(locality_event_header);

	/* TCG_PCR_EVENT2.Digests[].AlgorithmId */
	((tpmt_ha *)ptr)->algorithm_id = TPM_ALG_ID;

	/* TCG_PCR_EVENT2.Digests[].Digest[] */
	memset(&((tpmt_ha *)ptr)->digest, 0, TCG_DIGEST_SIZE);
	ptr += offsetof(tpmt_ha, digest) + TCG_DIGEST_SIZE;

	/* TCG_PCR_EVENT2.EventSize */
	((event2_data_t *)ptr)->event_size = sizeof(startup_locality_event_t);
	ptr += offsetof(event2_data_t, event);

	/* TCG_EfiStartupLocalityEvent.Signature */
	(void)memcpy(ptr, TCG_STARTUP_LOCALITY_SIGNATURE,
			sizeof(TCG_STARTUP_LOCALITY_SIGNATURE));
	/*
	 * TCG_EfiStartupLocalityEvent.StartupLocality = 0:
	 * the platform's boot firmware
	 */
	((startup_locality_event_t *)ptr)->startup_locality = 0;
	ptr += sizeof(startup_locality_event_t);
	if ((ptr - start_ptr) != LOC_EVENT_SIZE) {
		assert(false);
		panic();
	}

	log_ptr = ptr;

	/* Add BL2 event */
	rc = add_event2(NULL, BL2_STRING, BL2_IMAGE_ID);
	if (rc != 0) {
		panic();
	}
}

int record_measurement(uintptr_t image_base, uint32_t image_size,
			uint32_t image_id)
{
	unsigned int i;
	const char *image_name;

	unsigned char hash_data[MBEDTLS_MD_MAX_SIZE];
	int rc;

	/* Check if image_id is supported */
	for (i = 0; i < sizeof(images_data) / sizeof(*images_data); i++) {
		if (image_id == images_data[i].id) {
			image_name = images_data[i].name;
			break;
		}
	}

	if (i == sizeof(images_data) / sizeof(*images_data)) {
		ERROR("%s(): image_id %u not supported\n", __func__, image_id);
		return -EINVAL;
	}

	if (image_id == TOS_FW_CONFIG_ID) {
		tos_fw_config_base = image_base;
	} else if (image_id == NT_FW_CONFIG_ID) {
		nt_fw_config_base = image_base;
	}

	/* Calculate hash */
	rc = crypto_mod_calc_hash(MBEDTLS_MD_ID, (void *)image_base,
					image_size, hash_data);
	if (rc != 0) {
		return rc;
	}

	return add_event2(hash_data, image_name, image_id);
}

int event_log_finalise(uint8_t **log_addr, size_t *log_size)
{
	/* Event Log size */
	size_t num_bytes = log_ptr - event_log;
	void *ns_log_addr;
	int rc;

	*log_addr = NULL;
	*log_size = 0U;

	if (tos_fw_config_base != 0UL) {
		/* Set Event Log data in TOS_FW_CONFIG */
		rc = arm_set_tos_fw_info((void *)tos_fw_config_base,
					event_log, num_bytes);
		if (rc != 0) {
			ERROR("Unable to update TOS_HW_CONFIG data\n");
			return rc;
		}
	} else {
		WARN("TOS_FW_CONFIG not loaded\n");
	}

	if (nt_fw_config_base == 0UL) {
		ERROR("NT_FW_CONFIG not loaded\n");
		return -ENOENT;
	}

	/*
	 * Set Event Log data in NT_FW_CONFIG and
	 * get Event Log address in Non-Secure memory
	 */
	rc = arm_set_nt_fw_info((void *)nt_fw_config_base,
#ifdef SPD_opteed
				event_log,
#endif
				num_bytes, &ns_log_addr);
	if (rc != 0) {
		ERROR("Unable to update NT_HW_CONFIG data\n");
		return rc;
	}

	/* Copy Event Log to Non-secure memory */
	(void)memcpy(ns_log_addr, event_log, num_bytes);

	/* Ensure that the Event Log is visible in Secure memory */
	flush_dcache_range((uintptr_t)event_log, num_bytes);

	/* Ensure that the Event Log is visible in Non-secure memory */
	flush_dcache_range((uintptr_t)ns_log_addr, num_bytes);

	/* Return Event Log address in Non-Secure memory */
	*log_addr = ns_log_addr;
	*log_size = num_bytes;

	return 0;
}
