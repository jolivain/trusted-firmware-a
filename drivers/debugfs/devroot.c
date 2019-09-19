/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <common/debug.h>
#include <drivers/debugfs/io.h>

#include "dev.h"

enum {
	DEV_ROOT_QROOT,
	DEV_ROOT_QDEV,
	DEV_ROOT_QFIP,
	DEV_ROOT_QBLOBS,
	DEV_ROOT_QBLOBCTL,
	DEV_ROOT_QPSCI
};

#include "blobs.h"

/*******************************************************************************
 * This array contains the directories available from the root directory.
 ******************************************************************************/
static const dirtab_t dirtab[] = {
	{"dev",   CHDIR | DEV_ROOT_QDEV,   0, O_READ},
	{"blobs", CHDIR | DEV_ROOT_QBLOBS, 0, O_READ},
	{"fip",   CHDIR | DEV_ROOT_QFIP,   0, O_READ}
};

static const dirtab_t devfstab[] = {
	{"psci",  CHDIR | DEV_ROOT_QPSCI, 0, O_READ}
};

/*******************************************************************************
 * This function exposes the elements of the root directory.
 * It also exposes the content of the dev and blobs directories.
 ******************************************************************************/
static int rootgen(chan_t *c, const dirtab_t *tab, int ntab, int n, dir_t *dir)
{
	switch (c->qid & ~CHDIR) {
	case DEV_ROOT_QROOT:
		tab = dirtab;
		ntab = NELEM(dirtab);
		break;
	case DEV_ROOT_QDEV:
		tab = devfstab;
		ntab = NELEM(devfstab);
		break;
	case DEV_ROOT_QBLOBS:
		tab = blobtab;
		ntab = NELEM(blobtab);
		break;
	default:
		return 0;
	}

	return devgen(c, tab, ntab, n, dir);
}

static int rootwalk(chan_t *c, const char *name)
{
	return devwalk(c, name, NULL, 0, rootgen);
}

/*******************************************************************************
 * This function copies at most n bytes from the element referred by c into buf.
 ******************************************************************************/
static int rootread(chan_t *c, void *buf, int n)
{
	const dirtab_t *dp;
	dir_t *dir;

	/* TODO: compile only for BL31 */
	if ((c->qid & CHDIR) != 0) {
		if (n < sizeof(dir_t)) {
			return -1;
		}

		dir = buf;
		return dirread(c, dir, NULL, 0, rootgen);
	}

	/* Only makes sense when using debug language */
	assert(c->qid != DEV_ROOT_QBLOBCTL);

	dp = &blobtab[c->qid - DEV_ROOT_QBLOBCTL];
	return buf2chan(c, buf, dp->data, n, dp->length);
}

static int rootstat(chan_t *c, const char *file, dir_t *dir)
{
	return devstat(c, file, dir, NULL, 0, rootgen);
}

const dev_t rootdevtab = {
	.id = '/',
	.stat = rootstat,
	.clone = devclone,
	.attach = devattach,
	.walk = rootwalk,
	.read = rootread,
	.write = deverrwrite,
	.mount = deverrmount,
	.seek = devseek
};
