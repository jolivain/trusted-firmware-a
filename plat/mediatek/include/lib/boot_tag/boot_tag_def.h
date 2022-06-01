/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Define boot tag index and callback here */
#define BOOT_TAG_TABLE(_func) \
	_func(BOOT_TAG_BOOT_REASON, 0x88610001, init_boot_reason) \
	_func(BOOT_TAG_COM_INFO, 0x88610004, init_debug_port) \
	_func(BOOT_TAG_MEM, 0x88610005, boot_tag_mem_init) \
	_func(BOOT_TAG_BOOT_TIME, 0x88610007, boot_time_init) \
	_func(BOOT_TAG_ABNORMAL_BOOT, 0x8861001b, abnormal_boot_status_init) \
	_func(BOOT_TAG_RAM_CONSOLE_INFO, 0x8861001c, init_ram_console) \
	_func(BOOT_TAG_EMI_INFO, 0x88610020, init_dramc_info) \
	_func(BOOT_TAG_GZ_INFO, 0x88610023, init_gz_info) \
	_func(BOOT_TAG_GZ_PLAT, 0x88610025, init_gz_plat)
