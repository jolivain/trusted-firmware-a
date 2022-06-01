/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GZ_BOOT_TAG_H
#define GZ_BOOT_TAG_H

#include <cdefs.h>
#include <stdint.h>

struct boot_tag_gz_info {
	uint32_t gz_configs;
	uint32_t bootloader;
	uint32_t build_variant;
};

#define SHA256_HASH_SIZE (32)
struct boot_tag_gz_platform {

	/* GZ platform */
	uint64_t flags;
	uint64_t exec_start_offset;
	uint32_t reserve_mem_size;

	/* GZ remap offset */
	struct {
		uint64_t offset_ddr;
		uint64_t offset_io;
		uint64_t offset_sec_io;
	} __packed remap;

	/* HW reg base */
	struct {
		uint32_t uart;
		uint32_t cpuxgpt;
		uint32_t gicd;
		uint32_t gicr;
		uint32_t pwrap;
		uint32_t rtc;
		uint32_t mcucfg;
		uint32_t spmi;
		uint32_t rgu;
		uint32_t systimer;
		uint32_t res[5];
	} __packed reg_base;

	/* Hardware version */
	struct {
		uint16_t hw_code;
		uint16_t hw_sub_code;
	} __packed plat_ver;

	/* VM info */
	uint32_t vm_mem_size;

	/* gz log info */
	uint32_t log_addr;
	uint32_t log_size;

	/* total detected DRAM size */
	uint32_t dram_size_1mb_cnt;

	/* device info area 30 */
	uint32_t dev_info_area30;

	/*sys timer irq*/
	uint32_t sys_timer_irq;

	/*pubk hash*/
	uint8_t pubk_hash[SHA256_HASH_SIZE];

	/*ram console buffer*/
	uint32_t ramconsole_pa;
	uint32_t ramconsole_sz;

	/*software irq*/
	uint32_t gz_sw_irq;

	/* RAZ */
	uint32_t reserved[20];

} __packed;
#endif /* GZ_BOOT_TAG_H */
