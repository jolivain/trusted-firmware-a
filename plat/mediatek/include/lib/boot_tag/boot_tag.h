/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOOT_TAG_H
#define BOOT_TAG_H

#include <stdint.h>
#include <lib/boot_tag/boot_tag_def.h>

struct boot_tag_header {
	uint32_t size;
	uint32_t tag;
};

#define ALIGN(x, a)			(((x) + (a) - 1) & ~((a) - 1))
#define boot_tag_next(t)	((void *)(t) + (t)->hdr.size)
#define boot_tag_size(type)	\
	ALIGN((sizeof(struct boot_tag_header) + sizeof(struct type)), sizeof(void *))

/* Expand boot tag enumeration */
#define EXPAND_AS_ENUMERATION(_boot_tag_idx_enum, _boot_tag_idx, _callbacks) \
	_boot_tag_idx_enum = _boot_tag_idx,
/* Expand boot tag function declaration */
#define EXPAND_AS_DECLARATION(_boot_tag_idx_enum, _boot_tag_idx, _callbacks) \
	extern int _callbacks(void *);
/* Calculate max boot tag callback counts */
#define EXPAND_AS_INDEX_COUNT(_boot_tag_idx_enum, _boot_tag_idx, _callbacks) \
	_boot_tag_idx_enum##_idx,

enum {
	BOOT_TAG_TABLE(EXPAND_AS_INDEX_COUNT)
	BOOT_TAG_CNT_MAX
};

enum {
	BOOT_TAG_TABLE(EXPAND_AS_ENUMERATION)
};
/* Declare boot tag function prototype */
BOOT_TAG_TABLE(EXPAND_AS_DECLARATION);

/* Declare individual boot tag struct */
struct boot_tag_boot_reason {
	uint32_t boot_reason;
};

struct mr_info_t {
	unsigned short mr_index;
	unsigned short mr_value;
};

struct isu_info_t {
	uint32_t buf_size;
	uint64_t buf_addr;
	uint64_t ver_addr;
	uint64_t con_addr;
};

/* Declare union for each boot tag struct */
struct boot_tag {
	struct boot_tag_header hdr;
	union {
		struct boot_tag_boot_reason boot_reason;
	} u;
};

/* Declare funciton of boot_tag_parse */
int boot_tag_parse(void);

#endif /* BOOT_TAG_H_INCLUDED */
