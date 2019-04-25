/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <stddef.h>

#include <io.h>

#include "convfr9p.h"

#define USED(x) ((void) x)
#define USERLEN 20

int
p9todir(Dir *dp, unsigned char *buf, int size)
{
	int n, len;
	long l;
	unsigned long long type, path;

	n = 0;
	SHORT(len, buf, n, size);			/* len */
	SHORT(dp->type, buf, n, size);			/* type */
	LONG(dp->dev, buf, n, size);			/* dev */
	CHAR(type, buf, n, size);			/* qid.type */
	LONG(l, buf, n, size);				/* qid.vers */
	LLONG(path, buf, n, size);			/* qid.path */
	LONG(l, buf, n, size);				/* mode */
	LONG(l, buf, n, size);				/* atime */
	LONG(l, buf, n, size);				/* mtime */
	LLONG(dp->length, buf, n, size);		/* length */
	STRING(NULL, USERLEN, buf, n, size);		/* uid */
	STRING(NULL, USERLEN, buf, n, size);		/* gid */
	STRING(NULL, USERLEN, buf, n, size);		/* muid */
	STRING(dp->name, NAMELEN, buf, n, size);	/* name */
	USED(l);

	if (len != n)
		return -1;
	dp->qid = type << 8 | path;

	return n;
}
