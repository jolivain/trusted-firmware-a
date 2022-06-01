/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <drivers/uart.h>
#if CONFIG_BOOT_TAG
#include <lib/boot_tag/boot_tag.h>
#endif
#if CONFIG_MTK_DEBUG_BUF
#include <lib/debug_buf/debug_buf_export.h>
#endif
#include <lib/mtk_init/mtk_init.h>
#include "mem_console.h"

/* Declare COM port boot tag struct */
struct boot_tag_log_com {
	unsigned int log_port;
	unsigned int log_baudrate;
	unsigned int log_enable;
	unsigned int log_dynamic_switch;
};

static console_t mtk_boot_console;

#pragma weak plat_uart_init

/* UART platform interface */
int plat_uart_init(uintptr_t uart_base, uint32_t baudrate, console_t *console)
{
	WARN("UART platform driver is not ready\n");
	return -1;
}
/******************************************************************************
 * Functions that parse log port information from boot tag.
 ******************************************************************************/
static unsigned int port_base = UART0_BASE;
static unsigned int port_baud = UART_BAUDRATE;
static unsigned int log_enable = 1;

int init_debug_port(void *tag_entry)
{
#if CONFIG_BOOT_TAG
	struct boot_tag_log_com *com_tag_info = (struct boot_tag_log_com *)tag_entry;

	if (com_tag_info == NULL) {
		return -1;
	}
	port_base = com_tag_info->log_port;
	port_baud = com_tag_info->log_baudrate;
	log_enable = com_tag_info->log_dynamic_switch;
#endif
	return 0;
}

/*******************************************************************************
 * Functions that set up the console
 ******************************************************************************/
/* Initialize the console to provide early debug support */
void mtk_console_core_init(void)
{
	int rc = 0;

	if (log_enable) {
		rc = plat_uart_init(port_base,
			port_baud,
			&mtk_boot_console);
		if (rc == 0) {
			/*
			 * The crash console doesn't use the multi console API, it uses
			 * the core console functions directly. It is safe to call panic
			 * and let it print debug information.
			 */
			panic();
		}

	console_set_scope(&mtk_boot_console, CONSOLE_FLAG_BOOT
					| CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}
}

void mtk_console_core_end(void)
{
	(void)console_flush();

	(void)console_unregister(&mtk_boot_console);
}

#if CONFIG_MTK_DEBUG_BUF && CONFIG_MEM_CONSOLE
static void mtk_mem_console_init(void *debug_buf_instance)
{
	int rc;
	static mem_console_t mtk_mem_console;
	uintptr_t mem_console_body;

	if (!debug_buf_instance) {
		ERROR("debug buffer instance is NULL\n");
		assert(debug_buf_instance != NULL);
		return;
	}
	mem_console_body = debug_buf_get_body_addr(debug_buf_instance);
	if (!mem_console_body) {
		ERROR("memory console is not available\n");
		return;
	}
	rc = console_mem_register((uintptr_t)debug_buf_instance,
		&mtk_mem_console);
	if (rc == 0) {
		ERROR("Fail to register memory console\n");
		assert(rc != 0);
		return;
	}
	console_set_scope((console_t *)&mtk_mem_console,
			CONSOLE_FLAG_BOOT
			| CONSOLE_FLAG_RUNTIME);
}

static void mtk_mem_crash_console_init(void *debug_buf_instance)
{
	static crash_mem_console_t mtk_mem_crash_console;

	if (!debug_buf_instance) {
		ERROR("debug buffer instance is NULL\n");
		assert(debug_buf_instance != NULL);
		return;
	}
	/*
	 * Initialize crash memory meta information for
	 * per cpu record.
	 */
	uintptr_t crash_mem_body_current;
	struct crash_mem_record *crash_buf_record_ptr =
			mtk_mem_crash_console.crash_buf_per_cpu;

	/* Sanity check */
	crash_mem_body_current = debug_buf_get_body_addr(debug_buf_instance);
	if (!crash_mem_body_current) {
		ERROR("Crash memory is not available\n");
		return;
	}
	uint32_t debug_buf_size = debug_buf_get_size(debug_buf_instance);

	if (debug_buf_size <
		(DEBUG_BUF_CRASH_REPORT_ONE_CORE_SIZE * PLATFORM_CORE_COUNT)) {
		ERROR("The crash buffer is not enough for every cpu\n");
		return;
	}
	/* Populate crash memory per cpu record */
	uint32_t cpuid;

	/* Init cash buffer magic number */
	mtk_mem_crash_console.debug_hdr_addr =
			debug_buf_get_header(debug_buf_instance);
	/*
	 * Kind of tricky, magic number is the first unsigned int
	 * in debug_buf_hdr.
	 */
	*(uint32_t *)mtk_mem_crash_console.debug_hdr_addr = 0x0;
	for (cpuid = 0 ; cpuid < PLATFORM_CORE_COUNT ; cpuid++) {
		crash_buf_record_ptr->crash_buf_addr = crash_mem_body_current;
		crash_buf_record_ptr->w_offset = 0;
		crash_buf_record_ptr->crash_buf_size =
			DEBUG_BUF_CRASH_REPORT_ONE_CORE_SIZE;
		crash_buf_record_ptr++;
		crash_mem_body_current += DEBUG_BUF_CRASH_REPORT_ONE_CORE_SIZE;
	}
	int rc;

	rc = console_crash_mem_register((uintptr_t)debug_buf_instance,
		&mtk_mem_crash_console);
	if (rc == 0) {
		ERROR("Fail to register crash memory console\n");
		assert(rc != 0);
		return;
	}
	console_set_scope((console_t *)&mtk_mem_crash_console,
		CONSOLE_FLAG_CRASH);
}

static int memory_console_init(void)
{
	void *runtime_handle = debug_buf_get_handle(RUNTIME_LOG);
	void *crash_handle = debug_buf_get_handle(CRASH_REPORT);

	if (runtime_handle != NULL)
		mtk_mem_console_init(runtime_handle);
	if (crash_handle != NULL)
		mtk_mem_crash_console_init(crash_handle);
	return 0;
}
/* In order to enable memory console as early as possible */
MTK_ARCH_INIT(memory_console_init);
#endif /* End of CONFIG_MTK_DEBUG_BUF && CONFIG_MEM_CONSOLE */
