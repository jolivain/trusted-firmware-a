/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if __aarch64__
/* AARCH64 */
#define ALIGN 16
extern void memcpy16(void *dest, const void *src, unsigned long length);

#else
/* AARCH32 */
#define ALIGN 8
extern void memcpy8(void *dest, const void *src, unsigned int length);
#endif

#define is_aligned(pointer) ( ((uintptr_t)pointer & (ALIGN - 1)) == 0 )

void *memcpy(void *dst, const void *src, size_t len)
{
	if (len) {
		if (is_aligned(dst) && is_aligned(src)) {
#if __aarch64__
			memcpy16(dst, src, len);
#else
			memcpy8(dst, src, len);
#endif
		}
		else {
			const char *s = src;
			char *d = dst;

			while (len--)
				*d++ = *s++;
		}
	}
	return dst;
}
