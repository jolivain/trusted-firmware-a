/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>

/* Generic platform constants */
#define PLAT_PRIMARY_CPU	       0x0

#define IO_PHYS			       0x10000000

#define DRAM_BASE		       0x40000000
#define PLATFORM_STACK_SIZE	       0x1000

#define PLATFORM_SYSTEM_COUNT	       U(1)
#define PLATFORM_CLUSTER_COUNT	       U(1)
#define PLATFORM_MAX_CPUS_PER_CLUSTER  U(4)
#define PLATFORM_CLUSTER0_CORE_COUNT   PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CORE_COUNT	       PLATFORM_CLUSTER0_CORE_COUNT
#define SOC_CHIP_ID		       U(0x8365)

#define SYS_COUNTER_FREQ_IN_TICKS      13000000

/* BL31 specific defines */
#define BL31_BASE		       (DRAM_BASE + 0x3000000 + 0x1000)
#define BL31_SIZE		       (0x30000 - 0x1000)
#define BL31_LIMIT		       (BL31_BASE + BL31_SIZE)

/* BL32 specific defines */
#define BL32_BASE		       (DRAM_BASE + 0x3200000)
#define BL32_SIZE		       0xa00000

/* Platform binary types for linking */
#define PLATFORM_LINKER_FORMAT	       "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH	       aarch64

/* Power */
#define PLAT_MAX_PWR_LVL	       U(2)
#define PLAT_MAX_RET_STATE	       U(1)
#define PLAT_MAX_OFF_STATE	       U(2)

/* Generic Interrupt Controller */
#define BASE_GICD_BASE		       0x0c000000
#define MT_GIC_BASE		       0x0c000000
#define MT_GIC_RDIST_BASE	       (MT_GIC_BASE + 0x80000)

/* Platform memory map related constants */
#define MTK_DEV_RNG0_BASE	       IO_PHYS
#define MTK_DEV_RNG0_SIZE	       0x490000
#define MTK_DEV_RNG1_BASE	       (IO_PHYS + 0x1000000)
#define MTK_DEV_RNG1_SIZE	       0x4000000
#define MTK_DEV_RNG2_BASE	       0x0c000000
#define MTK_DEV_RNG2_SIZE	       0x600000

/* System Power Manager */
#define SPM_BASE		       (IO_PHYS + 0x6000)

/* MCUCFG */
#define MCUCFG_BASE		       (IO_PHYS + 0x200000)
#define SEC_POL_CTL_EN0		       (MCUCFG_BASE + 0x0a00)
#define MCU_ALL_PWR_ON		       (MCUCFG_BASE + 0x0b58)

/* MCUCPM */
#define MCUPM_BASE		       (IO_PHYS + 0x216000)

/* UART related constants */
#define UART0_BASE		       (IO_PHYS + 0x1002000)
#define UART1_BASE		       (IO_PHYS + 0x1003000)

#define MTK_UART_CLOCK		       26000000
#define MTK_UART_BAUDRATE	       921600
#define MTK_UART_DEFAULT_BASE	       UART0_BASE

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_SHIFT	       6
#define CACHE_WRITEBACK_GRANULE	       BIT(CACHE_WRITEBACK_SHIFT)

/* Platform specific page table and MMU setup constants */
#define PLAT_PHY_ADDR_SPACE_SIZE       BIT(32)
#define PLAT_VIRT_ADDR_SPACE_SIZE      BIT(32)
#define MAX_XLAT_TABLES		       16
#define MAX_MMAP_REGIONS	       16

#endif /* PLATFORM_DEF_H */
