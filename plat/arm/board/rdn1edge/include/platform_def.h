/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>

#include <sgi_base_platform_def.h>

#define CSS_SGI_MAX_CHIP_COUNT		U(2)
#define PLAT_ARM_CLUSTER_COUNT		U(2)
#define CSS_SGI_MAX_CPUS_PER_CLUSTER	U(4)
#define CSS_SGI_MAX_PE_PER_CPU		U(1)

#define PLAT_CSS_MHU_BASE		UL(0x45400000)
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

/* Base address of DMC-620 instances */
#define RDN1EDGE_DMC620_BASE0		UL(0x4e000000)
#define RDN1EDGE_DMC620_BASE1		UL(0x4e100000)

/* System power domain level */
#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2

#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL1

/* Virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xc0000000)

/*
 * Physical and virtual address space limits for MMU in AARCH64 & AARCH32 modes
 */
#ifdef __aarch64__
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 43)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 43)
#else
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#endif

#endif /* PLATFORM_DEF_H */
