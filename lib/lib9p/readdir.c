/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>
#include <string.h>

#include <io.h>

int
readdir(DIR *dir, struct dirent *ent)
{
	int n;
	Dir dentry;

	n = read(dir->fd, dir->buf, sizeof(dir->buf));

	if (n <= 0)
		return n;

	if (p9todir(&dentry, dir->buf, n) < 0) {
		errno = EINVAL;
		return -1;
	}

	strcpy(ent->d_name, dentry.name);

	return 1;
}
