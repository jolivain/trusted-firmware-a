/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>
#include <string.h>

#include <io.h>

#define NR_DIRS 2


/*
 * This code assumes that open is not going to
 * return 0 because it is the console fd.
 */
DIR *
opendir(const char *name)
{
	int fd;
	DIR *dir;
	static DIR dirs[NR_DIRS];

	for (dir = dirs; dir < &dirs[NR_DIRS] && dir->fd; ++dir)
		;

	if (dir == &dirs[NR_DIRS]) {
		errno = ENOMEM;
		return NULL;
	}

	fd = open(name, O_READ);
	if (fd < 0)
		return NULL;
	dir->fd = fd;

	/* TODO: check that it is actually a directory */

	return dir;
}
