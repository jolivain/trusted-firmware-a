/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HW_CRC32_H
#define HW_CRC32_H

#include <stddef.h>
#include <stdint.h>

/* compute CRC using Arm intrinsic function */
unsigned long hw_crc32(unsigned long crc, unsigned char *buf, size_t size);

#endif /* HW_CRC32_H */
