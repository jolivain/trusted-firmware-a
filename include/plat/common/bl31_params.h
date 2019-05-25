/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef COMMON_BL31_PLAT_PARAMS_H
#define COMMON_BL31_PLAT_PARAMS_H

#include <stdbool.h>
#include <stdint.h>

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
 *                    +----------------+   |--> struct bl31_plat_param
 *               +----|      next      |   |
 *               |    +----------------+ --+
 *               |    | parameter data |
 *               |    +----------------+
 *               |
 *               +--> +----------------+ --+
 *                    |      type      |   |
 *                    +----------------+   |--> struct bl31_plat_param
 *           NULL <---|      next      |   |
 *                    +----------------+ --+
 *                    | parameter data |
 *                    +----------------+
 *
 * Note: The SCTLR_EL3.A bit (Alignment fault check enable) is set in TF, so BL2
 * must ensure that each parameter struct starts on a 64-bit aligned address to
 * avoid alignment faults. Parameters may be allocated in any address range
 * accessible at the time of BL31 handoff (e.g. SRAM, DRAM, SoC-internal scratch
 * registers, etc.), in particular address ranges that may not be mapped in
 * BL31's page tables, so the parameter list must be parsed before the MMU is
 * enabled and any information that is required at a later point should be
 * deep-copied out into BL31-internal data structures.
 */

enum bl31_plat_param_type {
	BL31_PLAT_PARAM_NONE = 0,
	BL31_PLAT_PARAM_VENDOR_SPECIFIC_FIRST = 0x1,
	/* 0x1 - 0x7fffffff can be used by vendor-specific handlers. */
	BL31_PLAT_PARAM_VENDOR_SPECIFIC_LAST = 0x7fffffff,
	BL31_PLAT_PARAM_GENERIC_FIRST = 0x80000001,
	BL31_PLAT_PARAM_COREBOOT_TABLE = BL31_PLAT_PARAM_GENERIC_FIRST,
	/* 0x80000001 - 0xffffffff are reserved for the generic handler. */
	BL31_PLAT_PARAM_GENERIC_LAST = 0xffffffff,
	/* Top 32 bits of the type field are reserved for future use. */
};

/* common header for all BL31 platform parameters */
typedef struct {
	uint64_t type;
	void *next;
} bl31_plat_param_header_t;

/* commonly useful parameter structures that can be shared by multiple types */
typedef struct {
	bl31_plat_param_header_t h;
	uint64_t value;
} bl31_plat_param_uint64_t;

#define BL31_GPIO_DIR_OUT		0
#define BL31_GPIO_DIR_IN		1

#define BL31_GPIO_LEVEL_LOW		0
#define BL31_GPIO_LEVEL_HIGH		1

#define BL31_GPIO_PULL_NONE		0
#define BL31_GPIO_PULL_UP		1
#define BL31_GPIO_PULL_DOWN		2

typedef struct {
	uint8_t polarity;
	uint8_t direction;
	uint8_t pull_mode;
	uint8_t reserved;
	uint32_t index;
} bl31_plat_gpio_info_t;

typedef struct {
	bl31_plat_param_header_t h;
	bl31_gpio_info_t gpio;
} bl31_plat_param_gpio_t;

/* Handler function that handles an individual platform parameter. Returns true
 * if the parameter was recognized and false if not. */
typedef bool (*bl31_plat_param_handler_t)(bl31_plat_param_header_t *param);

/* Interprets plat_param as the start of a platform parameter list, and passes
 * the parameters individually to handler(). Handles generic parameters directly
 * if handler() hasn't already done so. If only generic parameters are expected,
 * handler() can be NULL. */
void bl31_plat_params_parse(u_register_t plat_param,
			    bl31_plat_param_handler_t handler);

#endif /* COMMON_BL31_PLAT_PARAMS_H */
