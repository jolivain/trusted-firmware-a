/*
 * Copyright (c) 2018-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPTOOL_H
#define SPTOOL_H

#include <stdint.h>

#define SECURE_PARTITION_MAGIC		0x5A5A5A5A

/* Header for a secure partition package. */
struct sp_pkg_header {
	uint32_t magic;
	uint32_t version;
	uint32_t img_offset;
	uint32_t img_size;
	uint32_t pm_offset;
	uint32_t pm_size;
};

#endif /* SPTOOL_H */
