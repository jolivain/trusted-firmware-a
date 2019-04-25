/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>
#include <string.h>

#include <io.h>

int
closedir(DIR *dir)
{
	int fd = dir->fd;

	dir->fd = 0;
	return close(fd);
}
