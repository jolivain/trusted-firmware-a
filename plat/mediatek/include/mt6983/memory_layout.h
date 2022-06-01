/*
 * Copyright (c) 2021, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

/* Constant memory address, size is defined here */
#define MTK_BL31_BASE 0x48200000
#define MTK_BL31_MAX_SIZE 0x200000

/*
 * TF-A BL31 memory layout
 * |----------------| <--BL31_BASE(TZRAM_BASE)
 * |     TEXT       |
 * |     RODATA     |
 * |     RWDATA     |
 * |     BSS        |
 * |----------------| <--__RW_END__
 * |    Available   |
 * |   memory space |
 * |----------------| <--BL31_LIMIT(a.k.a RESERVED_MEM_BASE)
 * | RESERVED_MEM   |
 * |----------------|
 *
 * RESERVED_MEM
 * |----------------| <--RESERVED_MEM_BASE(a.k.a TFA_BOOT_ARG_BASE)
 * | tf-a boot arg  |
 * | (size:0x100)   |
 * |----------------| <--TEE_PARAM_BASE
 * | TEE para       |
 * | (size:0x1000-  |
 * | 0x100          |
 * |----------------|
 */
#define TEE_PARAM_SIZE     U(0x1000)
/* Sum up reserved memory in TF-A memory space */
#define BL31_RESERVED_MEM_SIZE (TEE_PARAM_SIZE)
#define RESERVED_MEM_BASE (TZRAM_BASE + TZRAM_SIZE - BL31_RESERVED_MEM_SIZE)
#define TFA_BOOT_ARG_BASE RESERVED_MEM_BASE
#define TFA_BOOT_ARG_SIZE U(0x100)
#define TEE_PARAM_BASE   (TFA_BOOT_ARG_BASE + TFA_BOOT_ARG_SIZE)

/* RPMB HMAC calculate */
#define LK_HMAC_BASE                    (0x48400000)
#define LK_HMAC_SIZE                    (0x1000)

/* To hash data from LK for secure boot */
#define LK_SECURE_BOOT_BASE             (0x48401000)
#define LK_SECURE_BOOT_MAX_SIZE         (0x1000)

#endif /* MEMORY_LAYOUT_H */
