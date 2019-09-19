/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/debugfs/images.h>
#include <drivers/debugfs/io.h>
#include <plat/common/platform.h>

#include "dev.h"

static int gen(chan_t *c, const dirtab_t *tab, int ntab, int n, dir_t *dir)
{
	return 1;
}

static int psciwalk(chan_t *c, const char *name)
{
	return devwalk(c, name, NULL, 0, gen);
}

static int pscistat(chan_t *c, const char *file, dir_t *dir)
{
	return devstat(c, file, dir, NULL, 0, gen);
}

static int psciread(chan_t *c, void *buf, int n)
{
	const unsigned char *topology = plat_get_power_domain_tree_desc();
	unsigned char *out = buf;
	unsigned int index;

	size_t size = 2 + topology[1];
	if (n < size) {
		return -1;
	}

	out[0] = topology[0]; /* root count */
	out[1] = topology[1]; /* cluster count */

	for (index = 2; index < size; index++) {
		out[index] = topology[index];
	}

	return index;
}

static chan_t *pscimount(chan_t *c, const char *spec)
{
	return NULL;
}

const dev_t pscidevtab = {
	.id = 'P',
	.stat = pscistat,
	.clone = devclone,
	.attach = devattach,
	.walk = psciwalk,
	.read = psciread,
	.write = deverrwrite,
	.mount = pscimount,
	.seek = devseek
};
