/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/measured_boot/event_log.h>

#include <plat/arm/common/plat_arm.h>

#if LOG_LEVEL >= EVENT_LOG_LEVEL

#define	LOG_ERROR	__LINE__

/*
 * Print TCG_EfiSpecIDEventStruct
 *
 * @param[in] data	A pointer to Event Log
 */
static unsigned int id_event_print(void **log_addr, size_t *log_size)
{
	unsigned int i;
	uint8_t info_size, *info_size_ptr;
	const id_event_headers_t *event = *log_addr;
	uint8_t *end_ptr = *log_addr + *log_size;
	const id_event_algorithm_size_t *alg_ptr;
	uint32_t number_of_algorithms;
	size_t digest_len;
	bool valid = true;

	if (*log_size < sizeof(id_event_headers_t)) {
		return LOG_ERROR;
	}

	/* The fields of the event log header are defined to be PCRIndex of 0,
	 * EventType of EV_NO_ACTION, Digest of 20 bytes of 0, and
	 * Event content defined as TCG_EfiSpecIDEventStruct.
	 */
	LOG_EVENT("TCG_EfiSpecIDEvent:\n");
	LOG_EVENT("  PCRIndex           : %u\n", event->header.pcr_index);
	if (event->header.pcr_index != PCR_0) {
		return LOG_ERROR;
	}

	LOG_EVENT("  EventType          : %u\n", event->header.event_type);
	if (event->header.event_type != EV_NO_ACTION) {
		return LOG_ERROR;
	}

	LOG_EVENT("  Digest             :");
	for (i = 0U; i < sizeof(event->header.digest);) {
		uint8_t val = event->header.digest[i++];

		printf(" %02x", val);
		if ((i & 0xF) == 0) {
			printf("\n");
			LOG_EVENT("\t\t      :");
		}
		if (val != 0U) {
			valid = false;
		}
	}
	if ((i & 0xF) != 0) {
		printf("\n");
	}

	if (!valid) {
		return LOG_ERROR;
	}

	LOG_EVENT("  EventSize          : %u\n",
			event->header.event_size);
	LOG_EVENT("  Signature          : %s\n",
			event->struct_header.signature);
	LOG_EVENT("  PlatformClass      : %u\n",
			event->struct_header.platform_class);
	LOG_EVENT("  SpecVersion        : %u.%u.%u\n",
			event->struct_header.spec_version_major,
			event->struct_header.spec_version_minor,
			event->struct_header.spec_errata);
	LOG_EVENT("  UintnSize          : %u\n",
			event->struct_header.uintn_size);

	number_of_algorithms = event->struct_header.number_of_algorithms;
	LOG_EVENT("  NumberOfAlgorithms : %u\n", number_of_algorithms);

	/* Address of DigestSizes[] */
	alg_ptr = event->struct_header.digest_size;

	/* Size of DigestSizes[] */
	digest_len = number_of_algorithms * sizeof(id_event_algorithm_size_t);
	if ((uint8_t *)alg_ptr + digest_len > end_ptr) {
		return LOG_ERROR;
	}

	LOG_EVENT("  DigestSizes        :\n");
	for (i = 0U; i < number_of_algorithms; ++i) {
		LOG_EVENT("    #%u AlgorithmId   : SHA", i);
		uint16_t algorithm_id = alg_ptr[i].algorithm_id;

		switch (algorithm_id) {
		case TPM_ALG_SHA256:
			printf("256\n");
			break;
		case TPM_ALG_SHA384:
			printf("384\n");
			break;
		case TPM_ALG_SHA512:
			printf("512\n");
			break;
		default:
			printf("?\n");
			ERROR("Algorithm 0x%x not found\n", algorithm_id);
			return LOG_ERROR;
		}

		LOG_EVENT("       DigestSize    : %u\n",
					alg_ptr[i].digest_size);
	}

	/* Address of VendorInfoSize */
	info_size_ptr = (uint8_t *)alg_ptr + digest_len;
	if (info_size_ptr > end_ptr) {
		return LOG_ERROR;
	}

	info_size = *info_size_ptr++;
	LOG_EVENT("  VendorInfoSize     : %u\n", info_size);

	/* Check VendorInfo end address */
	if (info_size_ptr + info_size > end_ptr) {
		return LOG_ERROR;
	}

	if (info_size != 0) {
		LOG_EVENT("  VendorInfo         :");
		for (i = 0U; i < info_size; ++i) {
			printf(" %02x", *info_size_ptr++);
		}
		printf("\n");
	}

	*log_size -= info_size_ptr - (uint8_t *)*log_addr;
	*log_addr = info_size_ptr;

	return 0;
}

/*
 * Print TCG_PCR_EVENT2
 *
 * @param[in] data	A pointer to Event Log
 */
unsigned int event2_print(uint8_t **log_addr, size_t *log_size)
{
	uint32_t event_size, count;
	size_t sha_size, digests_size = 0U;
	uint8_t *ptr = *log_addr, *end_ptr = *log_addr + *log_size;

	if (*log_size < sizeof(event2_header_t)) {
		return LOG_ERROR;
	}

	LOG_EVENT("PCR_Event2:\n");
	LOG_EVENT("  PCRIndex           : %u\n",
			((event2_header_t *)ptr)->pcr_index);
	LOG_EVENT("  EventType          : %u\n",
			((event2_header_t *)ptr)->event_type);

	count = ((event2_header_t *)ptr)->digests.count;
	LOG_EVENT("  Digests Count      : %u\n", count);

	/* Address of TCG_PCR_EVENT2.Digests[] */
	ptr += sizeof(event2_header_t);
	if ((ptr > end_ptr) || (count == 0U)) {
		return LOG_ERROR;
	}

	for (unsigned int i = 0U; i < count; ++i) {
		/* Check AlgorithmId address */
		if (ptr + offsetof(tpmt_ha, digest) > end_ptr) {
			return LOG_ERROR;
		}

		LOG_EVENT("    #%u AlgorithmId   : SHA", i);
		switch (((tpmt_ha *)ptr)->algorithm_id) {
		case TPM_ALG_SHA256:
			sha_size = SHA256_DIGEST_SIZE;
			printf("256\n");
			break;
		case TPM_ALG_SHA384:
			sha_size = SHA384_DIGEST_SIZE;
			printf("384\n");
			break;
		case TPM_ALG_SHA512:
			sha_size = SHA512_DIGEST_SIZE;
			printf("512\n");
			break;
		default:
			printf("?\n");
			ERROR("Algorithm 0x%x not found\n",
				((tpmt_ha *)ptr)->algorithm_id);
			panic();
		}

		/* End of Digest[] */
		ptr += offsetof(tpmt_ha, digest);
		if (ptr + sha_size > end_ptr) {
			return LOG_ERROR;
		}

		/* Total size of all digests */
		digests_size += sha_size;

		LOG_EVENT("       Digest        :");
		for (unsigned int j = 0U; j < sha_size;) {
			printf(" %02x", *ptr++);
			if ((++j & 0xF) == 0) {
				printf("\n");
				if (j < sha_size) {
					LOG_EVENT("\t\t      :");
				}
			}
		}
	}

	/* TCG_PCR_EVENT2.EventSize */
	if (ptr + offsetof(event2_data_t, event) > end_ptr) {
		return LOG_ERROR;
	}

	event_size = ((event2_data_t *)ptr)->event_size;
	LOG_EVENT("  EventSize          : %u\n", event_size);

	/* Address of TCG_PCR_EVENT2.Event[EventSize] */
	ptr += offsetof(event2_data_t, event);

	/* End of TCG_PCR_EVENT2.Event[EventSize] */
	if (ptr + event_size > end_ptr) {
		return LOG_ERROR;
	}

	if ((event_size == sizeof(startup_locality_event_t)) &&
	     strcmp((const char *)ptr, TCG_STARTUP_LOCALITY_SIGNATURE) == 0) {
		LOG_EVENT("  Signature          : %s\n",
			((startup_locality_event_t *)ptr)->signature);
		LOG_EVENT("  StartupLocality    : %u\n",
			((startup_locality_event_t *)ptr)->startup_locality);
	} else {
		LOG_EVENT("  Event              : %s\n", ptr);
	}

	*log_size -= ptr + event_size - *log_addr;
	*log_addr = ptr + event_size;

	return 0;
}
#endif	/* LOG_LEVEL >= EVENT_LOG_LEVEL */

int dump_event_log(uint8_t *log_addr, size_t log_size)
{
#if LOG_LEVEL >= EVENT_LOG_LEVEL
	unsigned int rc;

	/* Print TCG_EfiSpecIDEvent */
	rc = id_event_print((void *)&log_addr, &log_size);
	if (rc != 0U) {
		ERROR("Corrupted TCG_EfiSpecIDEvent, error @#%u\n", rc);
		return -1;
	}

	while (log_size != 0U) {
		rc = event2_print(&log_addr, &log_size);
		if (rc != 0U) {
			ERROR("Corrupted TCG_PCR_EVENT2 entry @#%u\n", rc);
			return -1;
		}
	}
#endif
	return 0;
}
