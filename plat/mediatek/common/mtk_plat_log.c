/*
 * Copyright (c) 2020, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <plat/common/platform.h>
#include <drivers/el3_uptime.h>

static const char * const plat_prefix_str[] = {
	"ERROR:   ", "NOTICE:  ", "WARNING: ", "INFO:    ", "VERBOSE: "};
#define MTK_PREFIX "(%u)"
#define DECIMAL_64BIT_CHAR_LEN_PLUS_DOT	21
/* loglevel + [TFA] + (x) + DECIMAL_64BIT_CHAR_LEN_PLUS_DOT + BUFFER + NULL char */
#define MTK_PREFIX_STR_LEN (9 + 5 + 3 + DECIMAL_64BIT_CHAR_LEN_PLUS_DOT + 10 + 1)

static char mtk_prefix_str[MTK_PREFIX_STR_LEN];
/* Reference unsigned_num_print of printf.c */
static int unsigned_num_print(uint64_t unum, unsigned int radix,
			      unsigned int insert_pos,  char insert_pattern,
			      char padc, int padn, char *output)
{
	/*
	 * Just need enough space to store 64 bit decimal integer
	 * plus '.' character.
	 */
	char num_buf[21];
	int i = 0, count = 0;
	unsigned int rem;
	char *output_ptr  = output;

	do {
		rem = unum % radix;
		if (rem < 0xa)
			num_buf[i] = '0' + rem;
		else
			num_buf[i] = 'a' + (rem - 0xa);
		i++;
		if (i == insert_pos)
			num_buf[i++] = insert_pattern;
		unum /= radix;
	} while (unum > 0U);

	if (padn > 0) {
		/* For six digit pad first zero, '0'.xxxxxx */
		if (i == padn) {
			*output_ptr++ = padc;
			count++;
		}
		while (i < padn) {
			*output_ptr++ = padc;
			count++;
			if (count == 1) {
				*output_ptr++ = insert_pattern;
				count++;
			}
			padn--;
		}
	}

	while (--i >= 0) {
		*output_ptr++ = num_buf[i];
		count++;
	}
	return count;
}

const char *plat_log_get_prefix(unsigned int log_level)
{
	unsigned int level;
	int print_cnt;
	char *prefix_str_ptr = mtk_prefix_str;

	if (log_level < LOG_LEVEL_ERROR) {
		level = LOG_LEVEL_ERROR;
	} else if (log_level > LOG_LEVEL_VERBOSE) {
		level = LOG_LEVEL_VERBOSE;
	} else {
		level = log_level;
	}
	print_cnt = snprintf(mtk_prefix_str, MTK_PREFIX_STR_LEN,
		"%s"MTK_PREFIX, plat_prefix_str[(level / 10U) - 1U], plat_my_core_pos());
	prefix_str_ptr += print_cnt;
	if (MT_LOG_KTIME) {
		*prefix_str_ptr++ = 'K';
		*prefix_str_ptr++ = ':';
	}
	*prefix_str_ptr++ = '[';
	print_cnt = unsigned_num_print(el3_uptime(), 10, 6, '.', '0', 7, prefix_str_ptr);
	prefix_str_ptr += print_cnt;
	*prefix_str_ptr++ = ']';
	*prefix_str_ptr++ = '\0';
	return mtk_prefix_str;
}
