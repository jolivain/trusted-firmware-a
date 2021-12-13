/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if __aarch64__
/* __aarch64__ */
#define ZERO 0UL
#else
/* __aarch32__ */
#define ZERO 0U
#endif

extern void memcpy_align8(void *dst, const void *src, unsigned long length);

/* stp/ldp only require 8 byte alignment not 16 */
#define ALIGN 8

#define are_aligned(dst,src) ((((uintptr_t)src | (uintptr_t)dst) & (ALIGN - 1)) == 0)

void *memcpy(void *dst, const void *src, size_t len)
{
	if (len != ZERO) {
		if (are_aligned(dst,src)) {
			memcpy_align8(dst, src, len);
		}
		else {
			const char *s = src;
			char *d = dst;

			while (len != ZERO) {
				*d++ = *s++;
				len--;
			}
		}
	}

	return dst;
}
