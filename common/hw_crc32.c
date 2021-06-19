/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <arm_acle.h>
#include <assert.h>
#include <stdarg.h>

#include <common/debug.h>
#include <lib/cassert.h>

CASSERT(ENABLE_HW_CRC32 == 1, assert_hw_crc32_not_supported);

/* hw_crc32 - compute CRC using Arm intrinsic function
 *
 * This function is useful for the platforms with the CPU ARMv8.0
 * (with CRC instructions supported), and onwards.
 * Platforms with CPU ARMv8.0 should make sure to add a compile switch
 * '-march=armv8-a+crc" for successful compilation of this file.
 *
 * @crc: previous accumulated CRC
 * @buf: buffer base address
 * @size: the size of the buffer
 *
 * Return calculated CRC value
 */
uint32_t hw_crc32(uint32_t crc, const unsigned char *buf, size_t size)
{
	assert(buf != NULL);

#if (ARM_ARCH_MAJOR == 8) && (ARM_ARCH_MINOR == 0)
	/*
	 * Assert that the ARMv8.0-CRC32 instructions are implemented
	 * otherwise, a fault gets triggered on accessing CRC32
	 * instructions.
	 * CRC32 instructions are implemented by default in ARMv8.1+
	 * architectures.
	 */
	assert(is_armv8_0_crc32_present());
#endif /* (ARM_ARCH_MAJOR == 8) && (ARM_ARCH_MINOR == 0) */

	uint32_t calc_crc = ~crc;
	const unsigned char *local_buf = buf;
	size_t local_size = size;

	/*
	 * calculate CRC over byte data
	 */
	while (local_size != 0UL) {
		calc_crc = __crc32b(calc_crc, *local_buf);
		local_buf++;
		local_size--;
	}

	return ~calc_crc;
}
