/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_TRUSTED_FIRMWARE_DRIVERS_GPIO_RAW_H
#define ARM_TRUSTED_FIRMWARE_DRIVERS_GPIO_RAW_H

/**************** NOTE ************** NOTE ************** NOTE ****************
 * This file is intended to be included by third-party projects. It should    *
 * not contain any #include directives or function prototypes. It should only *
 * contain constant and type definitions, only rely on ISO C fixed-width      *
 * integer types and not depend on the definition of any macros other than    *
 * U(), AARCH32/64 and __ASSEMBLY__. It should never be included directly,    *
 * but only through the corresponding wrapper header, which must include      *
 * relevant headers (like <stdint.h>) before this file.                       *
 **************** NOTE ************** NOTE ************** NOTE ****************/

#define ARM_TF_GPIO_DIR_OUT		0
#define ARM_TF_GPIO_DIR_IN		1

#define ARM_TF_GPIO_LEVEL_LOW		0
#define ARM_TF_GPIO_LEVEL_HIGH		1

#define ARM_TF_GPIO_PULL_NONE		0
#define ARM_TF_GPIO_PULL_UP		1
#define ARM_TF_GPIO_PULL_DOWN		2

#endif /* ARM_TRUSTED_FIRMWARE_DRIVERS_GPIO_RAW_H */
