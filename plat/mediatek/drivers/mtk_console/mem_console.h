/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEM_CONSOLE_H
#define MEM_CONSOLE_H

#include <lib/utils_def.h>
#if CONFIG_MTK_DEBUG_BUF
#include <lib/debug_buf/debug_buf_export.h>
#endif

#define DEBUG_BUF_CRASH_MEM_RECORD_BASE         U(0)
#define DEBUG_BUF_CRASH_MEM_RECORD_W_OFFSET     U(8)
#define DEBUG_BUF_CRASH_MEM_RECORD_SIZE         U(12)
#define SIZEOF_DEBUG_BUF_CRASH_MEM_RECORD       U(16)
#define DEBUG_BUF_CRASH_BUF_PER_CPU             (CONSOLE_T_DRVDATA + U(8))
#define TFA_CRASH_MAGIC_NO                      U(0xdead1abf)

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <drivers/console.h>

typedef struct mem_console {
	console_t generic_console;
	void *debug_hdr_p;
} mem_console_t;

struct crash_mem_record {
	uintptr_t crash_buf_addr;
	uint32_t w_offset;
	uint32_t crash_buf_size;
};

typedef struct crash_mem_console {
	console_t generic_console;
	uintptr_t debug_hdr_addr;
	struct crash_mem_record crash_buf_per_cpu[PLATFORM_CORE_COUNT];
} crash_mem_console_t;

int console_mem_register(uintptr_t debug_buf_instance,
	mem_console_t *console);
int console_crash_mem_register(uintptr_t debug_buf_instance,
	crash_mem_console_t *console);
#endif /* __ASSEMBLER__ */
#endif /* MEM_CONSOLE_H */
