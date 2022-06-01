/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <cold_boot.h>
#include <common/debug.h>
#include <lib/boot_tag/boot_tag.h>
#include <lib/mtk_init/mtk_init.h>
#include <mtk_common.h>

#define BOOT_TAG_INIT_DEBUG_MSG "tag id:0x%x, tag size:%u\n"
#define BOOT_TAG_INIT_DEBUG(...) INFO(__VA_ARGS__)
#define EXPAND_AS_BOOT_TAG_OPERATION(_boot_tag_idx_enum, _boot_tag_idx, _callback) \
	case _boot_tag_idx: \
		boot_tag_callback_hit_num++; \
		BOOT_TAG_INIT_DEBUG(BOOT_TAG_INIT_DEBUG_MSG, _boot_tag_idx,\
			pboot_tag_entry->hdr.size); \
		ret = _callback((void *)&pboot_tag_entry->u); \
		if (ret) \
			NOTICE("%s fail, errno:%d\n", #_callback, ret); \
		break;

static uint32_t boot_tag_callback_hit_num;

/*
 * Create registered boot tag callback as weak function
 * to prevent from build break if a certain module having
 * a boot tag callback is not builtin.
 */
#define EXPAND_AS_WEAK_FUNC(_boot_tag_idx_enum, _boot_tag_idx, _callback) \
int __attribute__((weak)) _callback(void *tag_entry) \
{ \
	INFO("[boot tag id:0x%x] %s is not implemented\n", \
		_boot_tag_idx, #_callback); \
	return -1; \
}

BOOT_TAG_TABLE(EXPAND_AS_WEAK_FUNC);

static struct boot_tag *get_boot_tag(void)
{
	struct mtk_bl_param_t *p_mtk_bl_param;

	p_mtk_bl_param = (struct mtk_bl_param_t *) get_mtk_bl31_fw_config(BOOT_ARG_FROM_BL2);
	if (!p_mtk_bl_param) {
		assert(p_mtk_bl_param != NULL);
		ERROR("p_mtk_bl_param is NULL!\n");
		return NULL;
	}
	return (struct boot_tag *)(p_mtk_bl_param->bootarg_loc);
}

static void print_all_boot_tag(void)
{
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	struct boot_tag *pboot_tag_table = get_boot_tag();
	struct boot_tag *pboot_tag_entry;

	for (pboot_tag_entry = pboot_tag_table;
		pboot_tag_entry != 0 && pboot_tag_entry->hdr.size;
		pboot_tag_entry = boot_tag_next(pboot_tag_entry)) {
		BOOT_TAG_INIT_DEBUG(BOOT_TAG_INIT_DEBUG_MSG, pboot_tag_entry->hdr.tag,
			pboot_tag_entry->hdr.size);
	}
#endif
}

int boot_tag_parse(void)
{
	struct boot_tag *pboot_tag_table = get_boot_tag();
	struct boot_tag *pboot_tag_entry;
	int ret = 0;

	print_all_boot_tag();
	INFO("BOOT_TAG_CNT_MAX:%u\n", BOOT_TAG_CNT_MAX);
	for (pboot_tag_entry = pboot_tag_table;
			pboot_tag_entry != 0 && pboot_tag_entry->hdr.size;
			pboot_tag_entry = boot_tag_next(pboot_tag_entry)) {
		if (boot_tag_callback_hit_num >= BOOT_TAG_CNT_MAX)
			break;
		switch (pboot_tag_entry->hdr.tag) {
		BOOT_TAG_TABLE(EXPAND_AS_BOOT_TAG_OPERATION);
		default:
			INFO("boot tag idx is not found:0x%x\n", pboot_tag_entry->hdr.tag);
		}
	}
	return ret;
}

