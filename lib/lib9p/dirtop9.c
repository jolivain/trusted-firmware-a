/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <io.h>

#include "convto9p.h"

int
dirtop9(Dir *dp, unsigned char *buf, int n)
{
	int len;
	unsigned char *p;

	if (n < DIRLEN)
		return 0;

	p = buf + 2;
	SHORT(dp->type, p);	/* type */
	LONG(dp->dev, p);	/* dev */
	CHAR(dp->qid >> 8, p);	/* qid.type */
	LONG(0, p);		/* qid.vers */
	LLONG(dp->qid, p);	/* qid.path */
	LONG(0, p);		/* mode */
	LONG(0, p);		/* atime */
	LONG(0, p);		/* mtime */
	LLONG(dp->length, p);	/* length */
	STRING("root", p);	/* uid */
	STRING("root", p);	/* gid */
	STRING("root", p);	/* muid */
	STRING(dp->name, p);	/* name */
	len = p - buf;
	SHORT(len, buf);	/* size */

	assert(len < DIRLEN);

	return len;
}
