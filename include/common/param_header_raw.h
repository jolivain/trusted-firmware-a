/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_TRUSTED_FIRMWARE_COMMON_PARAM_HEADER_RAW_H
#define ARM_TRUSTED_FIRMWARE_COMMON_PARAM_HEADER_RAW_H

/**************** NOTE ************** NOTE ************** NOTE ****************
 * This file is intended to be included by third-party projects. It should    *
 * not contain any #include directives or function prototypes. It should only *
 * contain constant and type definitions, only rely on ISO C fixed-width      *
 * integer types and not depend on the definition of any macros other than    *
 * U(), AARCH32/64 and __ASSEMBLY__. It should never be included directly,    *
 * but only through the corresponding wrapper header, which must include      *
 * relevant headers (like <stdint.h>) before this file.                       *
 **************** NOTE ************** NOTE ************** NOTE ****************/

/* Param header types */
#define PARAM_EP			U(0x01)
#define PARAM_IMAGE_BINARY		U(0x02)
#define PARAM_BL31			U(0x03)
#define PARAM_BL_LOAD_INFO		U(0x04)
#define PARAM_BL_PARAMS			U(0x05)
#define PARAM_PSCI_LIB_ARGS		U(0x06)
#define PARAM_SP_IMAGE_BOOT_INFO	U(0x07)

/* Param header version */
#define PARAM_VERSION_1			U(0x01)
#define PARAM_VERSION_2			U(0x02)

#ifndef __ASSEMBLY__

/***************************************************************************
 * This structure provides version information and the size of the
 * structure, attributes for the structure it represents
 ***************************************************************************/
typedef struct param_header {
	uint8_t type;		/* type of the structure */
	uint8_t version;	/* version of this structure */
	uint16_t size;		/* size of this structure in bytes */
	uint32_t attr;		/* attributes: unused bits SBZ */
} param_header_t;

#endif /*__ASSEMBLY__*/

#endif /* ARM_TRUSTED_FIRMWARE_COMMON_PARAM_HEADER_RAW_H */
