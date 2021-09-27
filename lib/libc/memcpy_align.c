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
#define ALIGN 16
#define ZERO 0UL
extern void memcpy_align16(void *dst, const void *src, unsigned long length);
#define MEMCPY_ALIGN(dst, src, len) (memcpy_align16)(dst, src, len)

#else

/* __aarch32__ */
#define ALIGN 8
#define ZERO 0U
extern void memcpy_align8(void *dst, const void *src, unsigned int length);
#define MEMCPY_ALIGN(dst, src, len) (memcpy_align8)(dst, src, len)

#endif

#define are_aligned(dst,src) ((((uintptr_t)src | (uintptr_t)dst) & (ALIGN - 1)) == 0)

void *memcpy(void *dst, const void *src, size_t len)
{
	if (len != ZERO) {
		if (are_aligned(dst,src)) {
			MEMCPY_ALIGN(dst, src, len);
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
