/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_TRUSTED_FIRMWARE_LIB_BL_AUX_PARAMS_RAW_H
#define ARM_TRUSTED_FIRMWARE_LIB_BL_AUX_PARAMS_RAW_H

/**************** NOTE ************** NOTE ************** NOTE ****************
 * This file is intended to be included by third-party projects. It should    *
 * not contain any #include directives or function prototypes. It should only *
 * contain constant and type definitions, only rely on ISO C fixed-width      *
 * integer types and not depend on the definition of any macros other than    *
 * U(), AARCH32/64 and __ASSEMBLY__. It should never be included directly,    *
 * but only through the corresponding wrapper header, which must include      *
 * relevant headers (like <stdint.h>) before this file.                       *
 **************** NOTE ************** NOTE ************** NOTE ****************/

/*
 * This API implements a lightweight parameter passing mechanism that can be
 * used to pass SoC Firmware configuration data from BL2 to BL31 by platforms or
 * configurations that do not want to depend on libfdt. It is structured as a
 * singly-linked list of parameter structures that all share the same common
 * header but may have different (and differently-sized) structure bodies after
 * that. The header contains a type field to indicate the parameter type (which
 * is used to infer the structure length and how to interpret its contents) and
 * a next pointer which contains the absolute physical address of the next
 * parameter structure. The next pointer in the last structure block is set to
 * NULL. The picture below shows how the parameters are kept in memory.
 *
 * head of list  ---> +----------------+ --+
 *                    |      type      |   |
 *                    +----------------+   |--> struct bl_aux_param
 *               +----|      next      |   |
 *               |    +----------------+ --+
 *               |    | parameter data |
 *               |    +----------------+
 *               |
 *               +--> +----------------+ --+
 *                    |      type      |   |
 *                    +----------------+   |--> struct bl_aux_param
 *           NULL <---|      next      |   |
 *                    +----------------+ --+
 *                    | parameter data |
 *                    +----------------+
 *
 * Note: The SCTLR_EL3.A bit (Alignment fault check enable) is set in TF-A, so
 * BL2 must ensure that each parameter struct starts on a 64-bit aligned address
 * to avoid alignment faults. Parameters may be allocated in any address range
 * accessible at the time of BL31 handoff (e.g. SRAM, DRAM, SoC-internal scratch
 * registers, etc.), in particular address ranges that may not be mapped in
 * BL31's page tables, so the parameter list must be parsed before the MMU is
 * enabled and any information that is required at a later point should be
 * deep-copied out into BL31-internal data structures.
 */

enum bl_aux_param_type {
	BL_AUX_PARAM_NONE = 0,
	BL_AUX_PARAM_VENDOR_SPECIFIC_FIRST = 0x1,
	/* 0x1 - 0x7fffffff can be used by vendor-specific handlers. */
	BL_AUX_PARAM_VENDOR_SPECIFIC_LAST = 0x7fffffff,
	BL_AUX_PARAM_GENERIC_FIRST = 0x80000001,
	BL_AUX_PARAM_COREBOOT_TABLE = BL_AUX_PARAM_GENERIC_FIRST,
	/* 0x80000001 - 0xffffffff are reserved for the generic handler. */
	BL_AUX_PARAM_GENERIC_LAST = 0xffffffff,
	/* Top 32 bits of the type field are reserved for future use. */
};

/* common header for all BL aux parameters */
typedef struct {
	uint64_t type;
	uint64_t next;
} bl_aux_param_header_t;

/* commonly useful parameter structures that can be shared by multiple types */
typedef struct {
	bl_aux_param_header_t h;
	uint64_t value;
} bl_aux_param_uint64_t;

typedef struct {
	uint8_t polarity;
	uint8_t direction;
	uint8_t pull_mode;
	uint8_t reserved;
	uint32_t index;
} bl_aux_gpio_info_t;

typedef struct {
	bl_aux_param_header_t h;
	bl_aux_gpio_info_t gpio;
} bl_aux_param_gpio_t;

#endif /* ARM_TRUSTED_FIRMWARE_LIB_BL_AUX_PARAMS_RAW_H */
