/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMAGES_H
#define IMAGES_H

#include <drivers/debugfs/io.h>
#include <tools_share/firmware_image_package.h>

struct uuidnames {
	const char   name[NAMELEN];
	const uuid_t uuid;
};

#endif /* IMAGES_H */
