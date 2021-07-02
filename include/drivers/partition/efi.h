/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021, Linaro Limited
 */

#ifndef DRIVERS_PARTITION_EFI_H
#define DRIVERS_PARTITION_EFI_H

#include <cdefs.h>
#include <stdint.h>

#define GUID_LEN	16
#define EFI_NAMELEN	36

typedef struct {
	uint8_t b[GUID_LEN];
} efi_guid_t __aligned(8);

static inline int guidcmp(const void *g1, const void *g2)
{
	return memcmp(g1, g2, sizeof(efi_guid_t));
}

static inline void *guidcpy(void *dst, const void *src)
{
	return memcpy(dst, src, sizeof(efi_guid_t));
}

#define EFI_GUID(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7) \
	{{ (a) & 0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, \
		((a) >> 24) & 0xff, \
		(b) & 0xff, ((b) >> 8) & 0xff, \
		(c) & 0xff, ((c) >> 8) & 0xff, \
		(d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) } }

#define NULL_GUID \
	EFI_GUID(0x00000000, 0x0000, 0x0000, 0x00, 0x00, \
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

#endif /* DRIVERS_PARTITION_EFI_H */
