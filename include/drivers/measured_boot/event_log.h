/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <stdint.h>
#include <common/debug.h>

#include <drivers/measured_boot/tcg.h>

/* Set Event Log debug level */
/* #define	EVENT_LOG_LEVEL	LOG_LEVEL_ERROR */
/* #define	EVENT_LOG_LEVEL	LOG_LEVEL_INFO */
/* #define	EVENT_LOG_LEVEL	LOG_LEVEL_WARNING */
#define	EVENT_LOG_LEVEL	LOG_LEVEL_VERBOSE

#if EVENT_LOG_LEVEL   == LOG_LEVEL_ERROR
#define	LOG_EVENT	ERROR
#elif EVENT_LOG_LEVEL == LOG_LEVEL_NOTICE
#define	LOG_EVENT	NOTICE
#elif EVENT_LOG_LEVEL == LOG_LEVEL_WARNING
#define	LOG_EVENT	WARN
#elif EVENT_LOG_LEVEL == LOG_LEVEL_INFO
#define	LOG_EVENT	INFO
#elif EVENT_LOG_LEVEL == LOG_LEVEL_VERBOSE
#define	LOG_EVENT	VERBOSE
#else
#error "Not supported EVENT_LOG_LEVEL"
#endif

/* Number of hashing algorithms supported */
#define HASH_ALG_COUNT		1

/* Event Log length in bytes */
#define	EVENT_LOG_SIZE		1024

#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

#define BL2_STRING			"BL_2"
#define BL31_STRING			"BL_31"
#define BL32_STRING			"BL_32"
#define	BL32_EXTRA1_IMAGE_STRING	"BL32_EXTRA1_IMAGE"
#define	BL32_EXTRA2_IMAGE_STRING	"BL32_EXTRA2_IMAGE"
#define BL33_STRING			"BL_33"
#define HW_CONFIG_STRING		"HW_CONFIG"
#define NT_FW_CONFIG_STRING		"NT_FW_CONFIG"
#define SCP_BL2_IMAGE_STRING		"SCP_BL2_IMAGE"
#define SOC_FW_CONFIG_STRING		"SOC_FW_CONFIG"
#define	TOS_FW_CONFIG_STRING		"TOS_FW_CONFIG"

#pragma pack(push, 1)

typedef struct {
	unsigned int id;
	const char *name;
} image_data_t;

#pragma pack(pop)

#define	ID_EVENT_SIZE	sizeof(id_event_headers_t) + \
			sizeof(id_event_algorithm_size_t) * HASH_ALG_COUNT + \
			sizeof(id_event_struct_data_t)

#define	LOC_EVENT_SIZE	sizeof(event2_header_t) + \
			sizeof(tpmt_ha) + TCG_DIGEST_SIZE + \
			sizeof(event2_data_t) + \
			sizeof(startup_locality_event_t)

#define	LOG_MIN_SIZE	ID_EVENT_SIZE + LOC_EVENT_SIZE

#define EVENT2_HDR_SIZE	sizeof(event2_header_t) + \
			sizeof(tpmt_ha) + TCG_DIGEST_SIZE + \
			sizeof(event2_data_t)

#endif /* EVENT_LOG_H */
