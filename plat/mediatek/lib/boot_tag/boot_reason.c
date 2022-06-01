/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/boot_tag/boot_tag.h>

static uint32_t g_boot_reason;

int init_boot_reason(void *tag_entry)
{
	struct boot_tag_boot_reason *boot_reason = (struct boot_tag_boot_reason *)tag_entry;

	g_boot_reason = boot_reason->boot_reason;
	return 0;
}

uint32_t get_boot_reason(void)
{
	return g_boot_reason;
}
