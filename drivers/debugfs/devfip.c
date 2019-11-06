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
#include <plat/arm/common/plat_arm.h>
#include <tools_share/firmware_image_package.h>

#include "dev.h"

#define NR_FIPS		1
#define STOC_HEADER	16              /* sizeof(fip_toc_header_t); */
#define STOC_ENTRY	40              /* sizeof(fip_toc_entry_t); */

struct fipfile {
	chan_t	*c;
	long	offset[NR_FILES];
	long	size[NR_FILES];
};

struct fipntry {
	uuid_t		uuid;
	long long	offset_address;
	long long	size;
	long long	flags;
};

/*******************************************************************************
 * This array links the FIP file names to their UUID.
 * The elements are ordered according to the image number stored in
 * tbbr_img_def.h, starting at index 1.
 ******************************************************************************/

static const struct uuidnames uuidnames[] = {
	{"",			{ {0}, {0}, {0}, 0, 0, {0} } },
	{"bl2.bin",		UUID_TRUSTED_BOOT_FIRMWARE_BL2},
	{"scp-bl2.bin",		UUID_SCP_FIRMWARE_SCP_BL2},
	{"bl31.bin",		UUID_EL3_RUNTIME_FIRMWARE_BL31},
	{"bl32.bin",		UUID_SECURE_PAYLOAD_BL32},
	{"bl33.bin",		UUID_NON_TRUSTED_FIRMWARE_BL33},
	{"tb-fw.crt",		UUID_TRUSTED_BOOT_FW_CERT},
	{"trstd-k.crt",		UUID_TRUSTED_KEY_CERT},
	{"scp-fw-k.crt",	UUID_SCP_FW_KEY_CERT},
	{"soc-fw-k.crt",	UUID_SOC_FW_KEY_CERT},
	{"tos-fw-k.crt",	UUID_TRUSTED_OS_FW_KEY_CERT},
	{"nt-fw-k.crt",		UUID_NON_TRUSTED_FW_KEY_CERT},
	{"scp-fw-c.crt",	UUID_SCP_FW_CONTENT_CERT},
	{"soc-fw-c.crt",	UUID_SOC_FW_CONTENT_CERT},
	{"tos-fw-c.crt",	UUID_TRUSTED_OS_FW_CONTENT_CERT},
	{"nt-fw-c.crt",		UUID_NON_TRUSTED_FW_CONTENT_CERT},
	{ },
	{"fwu.crt",		UUID_TRUSTED_FWU_CERT},
	{"scp-bl2u.bin",	UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U},
	{"bl2u.bin",		UUID_TRUSTED_UPDATE_FIRMWARE_BL2U},
	{"ns-bl2u.bin",		UUID_TRUSTED_UPDATE_FIRMWARE_NS_BL2U},
	{"bl32-xtr1.bin",	UUID_SECURE_PAYLOAD_BL32_EXTRA1},
	{"bl32-xtr2.bin",	UUID_SECURE_PAYLOAD_BL32_EXTRA2},
	{"hw.cfg",		UUID_HW_CONFIG},
	{"tb-fw.cfg",		UUID_TB_FW_CONFIG},
	{"soc-fw.cfg",		UUID_SOC_FW_CONFIG},
	{"tos-fw.cfg",		UUID_TOS_FW_CONFIG},
	{"nt-fw.cfg",		UUID_NT_FW_CONFIG},
	{"rot-k.crt",		UUID_ROT_KEY_CERT},
	{"nt-k.crt",		UUID_NON_TRUSTED_WORLD_KEY_CERT}
};

/*******************************************************************************
 * This array contains all the available FIP files.
 ******************************************************************************/
static struct fipfile archives[NR_FIPS];

/*******************************************************************************
 * This variable stores the current number of registered FIP files.
 ******************************************************************************/
static int nfips;

/*******************************************************************************
 * This function parses the ToC of the FIP.
 ******************************************************************************/
static int getntry(chan_t *c, struct fipntry *ntry)
{
	int n;

	n = devtab[c->type]->read(c, ntry, sizeof(struct fipntry));
	if (n <= 0) {
		return n;
	}

	if (n != sizeof(struct fipntry)) {
		return -1;
	}

	if ((ntry->size > LONG_MAX) || (ntry->offset_address > LONG_MAX)) {
		return -1;
	}

	if (ntry->size == 0) {
		return 0;
	}

	return 1;
}

/*******************************************************************************
 * This function exposes the FIP images as files.
 ******************************************************************************/
static int fipgen(chan_t *c, const dirtab_t *tab, int ntab, int n, dir_t *dir)
{
	int i, r;
	long off;
	chan_t nc;
	struct fipntry ntry;
	struct fipfile *fip;
	static char unk[NAMELEN];

	if (c->dev >= nfips) {
		panic();
	}

	clone(archives[c->dev].c, &nc);
	fip = &archives[nc.dev];

	off = STOC_HEADER;
	for (i = 0; i <= n; i++) {
		if (fip->offset[i] == -1) {
			return 0;
		}

		if (devtab[nc.type]->seek(&nc, off, KSEEK_SET) < 0) {
			return -1;
		}

		r = getntry(&nc, &ntry);
		if (r <= 0) {
			return r;
		}

		off += sizeof(ntry);
	}

	for (i = 1; i < NELEM(uuidnames); i++) {
		if (memcmp(&uuidnames[i].uuid,
			   &ntry.uuid, sizeof(uuid_t)) == 0) {
			break;
		}
	}

	if (i < NELEM(uuidnames)) {
		mkentry(c, dir, uuidnames[i].name, ntry.size, n, O_READ);
	} else {
		// TODO: set name depending on uuid node value
		// FIXME: unk not initialized?....
		mkentry(c, dir, unk, ntry.size, n, O_READ);
	}

	return 1;
}

static int fipwalk(chan_t *c, const char *name)
{
	return devwalk(c, name, NULL, 0, fipgen);
}

static int fipstat(chan_t *c, const char *file, dir_t *dir)
{
	return devstat(c, file, dir, NULL, 0, fipgen);
}

/*******************************************************************************
 * This function copies at most n bytes of the FIP image referred by c into
 * buf.
 ******************************************************************************/
static int fipread(chan_t *c, void *buf, int n)
{
	long off;
	chan_t cs;
	struct fipfile *fip;
	long size;

	/* Only makes sense when using debug language */
	assert(c->qid != CHDIR);

	if (c->dev >= nfips || (c->qid & CHDIR)) {
		panic();
	}

	fip = &archives[c->dev];

	if ((c->qid >= NR_FILES) || (fip->offset[c->qid] < 0)) {
		panic();
	}

	clone(fip->c, &cs);

	size = fip->size[c->qid];
	if (c->offset >= size) {
		return 0;
	}

	if (n < 0) {
		return -1;
	}

	if (n > (size - c->offset)) {
		n = size - c->offset;
	}

	off = fip->offset[c->qid] + c->offset;
	if (devtab[cs.type]->seek(&cs, off, KSEEK_SET) < 0) {
		return -1;
	}

	n = devtab[cs.type]->read(&cs, buf, n);
	if (n > 0) {
		c->offset += n;
	}

	return n;
}

/*******************************************************************************
 * This function parses the FIP spec and registers its images in order to
 * expose them as files in the driver namespace.
 * It acts as an initialization function for the FIP driver.
 * It returns a pointer to the newly created channel.
 ******************************************************************************/
static chan_t *fipmount(chan_t *c, const char *spec)
{
	int r, n, t;
	chan_t *cspec;
	uint32_t hname;
	struct fipntry ntry;
	struct fipfile *fip;
	dir_t dir;

	if (nfips == NR_FIPS) {
		return NULL;
	}

	fip = &archives[nfips];

	for (n = 0; n < NR_FILES; n++) {
		fip->offset[n] = -1;
	}

	cspec = namec(spec, O_READ);
	if (cspec == NULL) {
		return NULL;
	}

	fip->c = cspec;

	r = devtab[cspec->type]->read(cspec, &hname, sizeof(hname));
	if (r < 0) {
		goto err;
	}

	if ((r != sizeof(hname)) || (hname != TOC_HEADER_NAME)) {
		goto err;
	}

	if (stat(spec, &dir) < 0) {
		goto err;
	}

	t = cspec->type;
	if (devtab[t]->seek(cspec, STOC_HEADER, KSEEK_SET) < 0) {
		goto err;
	}

	for (n = 0; n < NR_FILES; n++) {
		switch (getntry(cspec, &ntry)) {
		case 0:
			return attach('F', nfips++);
		case -1:
			goto err;
		default:
			if ((ntry.offset_address + ntry.size) > dir.length) {
				goto err;
			}

			fip->offset[n] = ntry.offset_address;
			fip->size[n] = ntry.size;
			break;
		}
	}

err:
	chanclose(cspec);
	return NULL;
}

const dev_t fipdevtab = {
	.id = 'F',
	.stat = fipstat,
	.clone = devclone,
	.attach = devattach,
	.walk = fipwalk,
	.read = fipread,
	.write = deverrwrite,
	.mount = fipmount,
	.seek = devseek
};

