/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <assert.h>

#include <arm_acle.h>
#include <common/debug.h>

/* hw_crc32 - compute CRC using Arm intrinsic function
 *
 * This function is useful for the platforms with the CPU ARMv8.0
 * (with CRC instructions supported), and onwards.
 * Make sure to add compile switch '-march=armv8-a+crc" for
 * successful compilation of this file.
 *
 * @crc: previous accumulated CRC
 * @buf: buffer base address
 * @size: the size of the buffer
 *
 * Return calculated CRC value
 */
unsigned long hw_crc32(unsigned long crc, unsigned char *buf, size_t size)
{
	assert(size != 0UL);
	assert(buf != NULL);

	unsigned long calc_crc = ~crc;

	/*
	 * calculate CRC over byte data
	 */
	while (size != 0UL) {
		calc_crc = __crc32b(calc_crc, *buf++);
		size--;
	}

	return ~calc_crc;
}
