/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/debugfs/io.h>

#include "dev.h"

#define NR_MPOINTS 4

struct mpoint {
	chan_t	*new;
	chan_t	*old;
};

/* This array contains all the available channels of the filesystem.
 * A file descriptor is the index of a specific channel in this array.
 */
static chan_t fdset[NR_CHANS];

/* This array contains all the available mount points of the filesystem. */
static struct mpoint mpoints[NR_MPOINTS];

/* This variable stores the channel associated to the root directory. */
static chan_t slash;

/* This function creates a channel from a type and registers it to fdset. */
static chan_t *newchan(unsigned char type)
{
	chan_t *c = NULL;
	int i;

	for (i = 0; i < NR_CHANS; i++) {
		if (fdset[i].type == NODEV) {
			c = &fdset[i];
			c->type = type;
			break;
		}
	}

	return c;
}

/*******************************************************************************
 * This function returns a pointer to an existing channel in fdset from a file
 * descriptor.
 ******************************************************************************/
static chan_t *fd2chan(int fd)
{
	if ((fd < 0) || (fd >= NR_CHANS) || (fdset[fd].type == NODEV)) {
		return NULL;
	}

	return &fdset[fd];
}

/*******************************************************************************
 * This function returns a file descriptor from a channel.
 * The caller must be sure that the channel is registered in fdset.
 ******************************************************************************/
static int chan2fd(chan_t *c)
{
	return (c == NULL) ? -1 : (c - fdset);
}

/*******************************************************************************
 * This function checks the validity of a mode.
 ******************************************************************************/
static bool validmode(int mode)
{
	if ((mode & O_READ) && (mode & (O_WRITE | O_RDWR))) {
		return false;
	}
	if ((mode & O_WRITE) && (mode & (O_READ | O_RDWR))) {
		return false;
	}
	if ((mode & O_RDWR) && (mode & (O_READ | O_WRITE))) {
		return false;
	}

	return true;
}

/*******************************************************************************
 * This function extracts the next part of the path contained in s and puts it
 * in elem. It returns a pointer to the remainder of the path.
 ******************************************************************************/
static const char *next(const char *s, char *elem)
{
	int n;
	const char *t;

	while (*s == '/') {
		++s;
	}

	n = 0;
	t = s;
	if (*s != '\0') {
		while (*t != '/' && *t != '\0') {
			if (n == NAMELEN) {
				return NULL;
			}
			elem[n++] = *t++;
		}
	}
	elem[n] = '\0';

	return t;
}

/*******************************************************************************
 * This function returns the driver type of the driver identifier c.
 *
 * NOTE: the returned value is devtab index for the driver identified by c.
 ******************************************************************************/
static int devtype(int c)
{
	int i;
	dev_t * const *dp;

	for (i = 0, dp = devtab; *dp && (*dp)->id != c; ++dp) {
		i++;
	}

	if (*dp == NULL) {
		return -1;
	}

	return i;
}

/*******************************************************************************
 * This function clears a given channel fields
 ******************************************************************************/
static void chan_clear(chan_t *chan)
{
	chan->offset = 0;
	chan->qid    = 0;
	chan->type   = NODEV;
	chan->dev    = 0;
	chan->mode   = 0;
}

/*******************************************************************************
 * This function closes the channel pointed to by c.
 ******************************************************************************/
void chanclose(chan_t *c)
{
	if (c != NULL) {
		chan_clear(c);
	}
}

/*******************************************************************************
 * This function copies data from src to dst after applying the offset of the
 * channel c. nbytes bytes are expected to be copied unless the data goes over
 * dst + len.
 * It returns the actual number of bytes that were copied.
 ******************************************************************************/
int buf2chan(chan_t *c, void *dst, void *src, int nbytes, long len)
{
	const char *addr = src;

	if ((c == NULL) || (dst == NULL) || (src == NULL)) {
		return 0;
	}

	if (c->offset >= len) {
		return 0;
	}

	if ((c->offset + nbytes) > len) {
		nbytes = len - c->offset;
	}

	memcpy(dst, addr + c->offset, nbytes);

	c->offset += nbytes;

	return nbytes;
}

/*******************************************************************************
 * This function checks whether a channel (identified by its type and qid) is
 * registered as a mount point.
 * Returns a pointer to the channel it is mounted to when found, NULL otherwise.
 ******************************************************************************/
static chan_t *mntpoint(int type, qid_t qid)
{
	chan_t *cn;
	struct mpoint *mp;

	for (mp = mpoints; mp < &mpoints[NR_MPOINTS]; mp++) {
		cn = mp->new;
		if (cn == NULL) {
			continue;
		}

		if ((cn->type == type) && (cn->qid == qid)) {
			return mp->old;
		}
	}

	return NULL;
}

/*******************************************************************************
 * This function calls the attach function of the driver identified by id.
 ******************************************************************************/
chan_t *attach(int id, int dev)
{
	/* Get the devtab index for the driver identified by id */
	int type = devtype(id);
	if (type < 0) {
		return NULL;
	}

	return devtab[type]->attach(id, dev);
}

/*******************************************************************************
 * This function is the default implementation of the driver attach function.
 * It creates a new channel and returns a pointer to it.
 ******************************************************************************/
chan_t *devattach(int id, int dev)
{
	chan_t *c;
	int type;

	type = devtype(id);
	if (type < 0) {
		return NULL;
	}

	c = newchan(type);
	if (c == NULL) {
		return NULL;
	}

	c->dev = dev;
	c->qid = CHDIR;

	return c;
}

/*******************************************************************************
 * This function returns a channel given a path.
 * It goes through the filesystem, from the root namespace ('/') or from a
 * device namespace ('#'), switching channel on mount points.
 ******************************************************************************/
chan_t *namec(const char *name, int mode)
{
	int i, n;
	const char *s;
	chan_t *mnt, *c;
	char elem[NAMELEN];

	if (name == NULL) {
		return NULL;
	}

	switch (name[0]) {
	case '/':
		c = clone(&slash, NULL);
		s = name;
		break;
	case '#':
		s = next(name+1, elem);
		if (s == NULL) {
			goto noent;
		}

		n = 0;
		for (i = 1; (elem[i] >= '0') && (elem[i] <= '9'); i++) {
			n += elem[i] - '0';
		}

		if (elem[i] != '\0') {
			goto noent;
		}

		c = attach(elem[0], n);
		break;
	default:
		return NULL;
	}

	if (c == NULL) {
		return NULL;
	}

	for (s = next(s, elem); *elem; s = next(s, elem)) {
		if ((c->qid & CHDIR) == 0) {
			goto notfound;
		}

		if (devtab[c->type]->walk(c, elem) < 0) {
			chanclose(c);
			goto notfound;
		}

		mnt = mntpoint(c->type, c->qid);
		if (mnt != NULL) {
			clone(mnt, c);
		}
	}

	if (s == NULL) {
		goto notfound;
	}

	/* TODO: check mode */
	return c;

notfound:
	chanclose(c);
noent:
	return NULL;
}

/*******************************************************************************
 * This function calls the clone function of the driver associated to the
 * channel c.
 ******************************************************************************/
chan_t *clone(chan_t *c, chan_t *nc)
{
	return devtab[c->type]->clone(c, nc);
}

/*******************************************************************************
 * This function is the default implementation of the driver clone function.
 * It creates a new channel and returns a pointer to it.
 * It clones c into nc.
 ******************************************************************************/
chan_t *devclone(chan_t *c, chan_t *nc)
{
	if (c == NULL) {
		return NULL;
	}

	if (nc == NULL) {
		nc = newchan(c->type);
		if (nc == NULL) {
			return NULL;
		}
	}

	nc->qid = c->qid;
	nc->dev = c->dev;
	nc->mode = c->mode;
	nc->offset = c->offset;
	nc->type = c->type;

	return nc;
}

/*******************************************************************************
 * This function is the default implementation of the driver walk function.
 * It goes through all the elements of tab using the gen function until a match
 * is found with name.
 * If a match is found, it copies the qid of the new directory.
 ******************************************************************************/
int devwalk(chan_t *c, const char *name, const dirtab_t *tab,
	    int ntab, devgen_t *gen)
{
	int i;
	dir_t dir;

	if ((c == NULL) || (name == NULL) || (gen == NULL)) {
		return -1;
	}

	if ((name[0] == '.') && (name[1] == '\0')) {
		return 1;
	}

	for (i = 0; ; i++) {
		switch ((*gen)(c, tab, ntab, i, &dir)) {
		case 0:
			/* Intentional fall-through */
		case -1:
			return -1;
		case 1:
			if (strncmp(name, dir.name, NAMELEN) != 0) {
				continue;
			}
			c->qid = dir.qid;
			return 1;
		}
	}
}

/*******************************************************************************
 * This is a helper function which exposes the content of a directory, element
 * by element. It is meant to be called until the end of the directory is
 * reached or an error occurs.
 * It returns -1 on error, 0 on end of directory and 1 when a new file is found.
 ******************************************************************************/
int dirread(chan_t *c, dir_t *dir, const dirtab_t *tab, int ntab, devgen_t *gen)
{
	int i, ret;

	if ((c == NULL) || (dir == NULL) || (gen == NULL)) {
		return -1;
	}

	i = c->offset/sizeof(dir_t);
	ret = (*gen)(c, tab, ntab, i, dir);
	if (ret == 1) {
		c->offset += sizeof(dir_t);
	}

	return ret;
}

/*******************************************************************************
 * This function sets the elements of dir.
 ******************************************************************************/
void mkentry(chan_t *c, dir_t *dir,
	     const char *name, long length, qid_t qid, unsigned int mode)
{
	if ((c == NULL) || (dir == NULL) || (name == NULL)) {
		return;
	}

	strlcpy(dir->name, name, sizeof(dir->name));
	dir->length = length;
	dir->qid = qid;
	dir->mode = mode;

	if ((qid & CHDIR) != 0) {
		dir->mode |= O_DIR;
	}

	dir->type = c->type;
	dir->dev = c->dev;
}

/*******************************************************************************
 * This function is the default implementation of the internal driver gen
 * function.
 * It copies and formats the information of the nth element of tab into dir.
 ******************************************************************************/
int devgen(chan_t *c, const dirtab_t *tab, int ntab, int n, dir_t *dir)
{
	const dirtab_t *dp;

	if ((c == NULL) || (dir == NULL) || (tab == NULL) || (n >= ntab)) {
		return 0;
	}

	dp = &tab[n];
	mkentry(c, dir, dp->name, dp->length, dp->qid, dp->perm);
	return 1;
}

/*******************************************************************************
 * This function returns a file descriptor identifying the channel associated to
 * the path fname.
 ******************************************************************************/
int open(const char *fname, int mode)
{
	chan_t *c;

	if (fname == NULL) {
		return -1;
	}

	if (validmode(mode) == false) {
		return -1;
	}

	c = namec(fname, mode);

	return chan2fd(c);
}

/*******************************************************************************
 * This function closes the channel identified by the file descriptor fd.
 ******************************************************************************/
int close(int fd)
{
	chan_t *c;

	c = fd2chan(fd);
	if (c == NULL) {
		return -1;
	}

	chanclose(c);
	return 0;
}

/*******************************************************************************
 * This function is the default implementation of the driver stat function.
 * It goes through all the elements of tab using the gen function until a match
 * is found with file.
 * If a match is found, dir contains the information file.
 ******************************************************************************/
int devstat(chan_t *dirc, const char *file, dir_t *dir,
	    const dirtab_t *tab, int ntab, devgen_t *gen)
{
	int i, r = 0;
	chan_t *c, *mnt;

	if ((dirc == NULL) || (dir == NULL) || (gen == NULL)) {
		return -1;
	}

	c = namec(file, O_STAT);
	if (c == NULL) {
		return -1;
	}

	for (i = 0; ; i++) {
		switch ((*gen)(dirc, tab, ntab, i, dir)) {
		case 0:
			/* Intentional fall-through */
		case -1:
			r = -1;
			goto leave;
		case 1:
			mnt = mntpoint(dir->type, dir->qid);
			if (mnt != NULL) {
				dir->qid = mnt->qid;
				dir->type = mnt->type;
			}

			if ((dir->qid != c->qid) || (dir->type != c->type)) {
				continue;
			}

			goto leave;
		}
	}

leave:
	chanclose(c);
	return r;
}

/*******************************************************************************
 * This function calls the stat function of the driver associated to the parent
 * directory of the file in path.
 * The result is stored in dir.
 ******************************************************************************/
int stat(const char *path, dir_t *dir)
{
	int r;
	size_t len;
	chan_t *c;
	char *p, dirname[PATHLEN];

	if ((path == NULL) || (dir == NULL)) {
		return -1;
	}

	len = strlen(path);
	if ((len + 1) > sizeof(dirname)) {
		return -1;
	}

	memcpy(dirname, path, len);
	for (p = dirname + len; p > dirname; --p) {
		if (*p != '/') {
			break;
		}
	}

	p = memrchr(dirname, '/', p - dirname);
	if (p == NULL) {
		return -1;
	}

	dirname[p - dirname + 1] = '\0';

	c = namec(dirname, O_STAT);
	if (c == NULL) {
		return -1;
	}

	r = devtab[c->type]->stat(c, path, dir);
	chanclose(c);

	return r;
}

/*******************************************************************************
 * This function calls the read function of the driver associated to fd.
 * It fills buf with at most n bytes.
 * It returns the number of bytes that were actually read.
 ******************************************************************************/
int read(int fd, void *buf, int n)
{
	chan_t *c;

	if (buf == NULL) {
		return -1;
	}

	c = fd2chan(fd);
	if (c == NULL) {
		return -1;
	}

	if (((c->qid & CHDIR) != 0) && (n < sizeof(dir_t))) {
		return -1;
	}

	return devtab[c->type]->read(c, buf, n);
}

/*******************************************************************************
 * This function calls the write function of the driver associated to fd.
 * It writes at most n bytes of buf.
 * It returns the number of bytes that were actually written.
 ******************************************************************************/
int write(int fd, void *buf, int n)
{
	chan_t *c;

	if (buf == NULL) {
		return -1;
	}

	c = fd2chan(fd);
	if (c == NULL) {
		return -1;
	}

	if ((c->qid & CHDIR) != 0) {
		return -1;
	}

	return devtab[c->type]->write(c, buf, n);
}

/*******************************************************************************
 * This function calls the seek function of the driver associated to fd.
 * It applies the offset off according to the strategy whence.
 ******************************************************************************/
int seek(int fd, long off, int whence)
{
	chan_t *c;

	c = fd2chan(fd);
	if (c == NULL) {
		return -1;
	}

	if ((c->qid & CHDIR) != 0) {
		return -1;
	}

	return devtab[c->type]->seek(c, off, whence);
}

/*******************************************************************************
 * This function is the default error implementation of the driver mount
 * function.
 ******************************************************************************/
chan_t *deverrmount(chan_t *c, const char *spec)
{
	return NULL;
}

/*******************************************************************************
 * This function is the default error implementation of the driver write
 * function.
 ******************************************************************************/
int deverrwrite(chan_t *c, void *buf, int n)
{
	return -1;
}

/*******************************************************************************
 * This function is the default error implementation of the driver seek
 * function.
 ******************************************************************************/
int deverrseek(chan_t *c, long off, int whence)
{
	return -1;
}

/*******************************************************************************
 * This function is the default implementation of the driver seek function.
 * It applies the offset off according to the strategy whence to the channel c.
 ******************************************************************************/
int devseek(chan_t *c, long off, int whence)
{
	switch (whence) {
	case KSEEK_SET:
		c->offset = off;
		break;
	case KSEEK_CUR:
		c->offset += off;
		break;
	case KSEEK_END:
		panic(); /* TODO */
	}

	return 0;
}

/*******************************************************************************
 * This function registers the channel associated to the path new as a mount
 * point for the channel c.
 ******************************************************************************/
static int addmntpoint(chan_t *c, const char *new)
{
	int i;
	chan_t *cn;
	struct mpoint *mp;

	if (new == NULL) {
		goto err0;
	}

	cn = namec(new, O_READ);
	if (cn == NULL) {
		goto err0;
	}

	if ((cn->qid & CHDIR) == 0) {
		goto err1;
	}

	for (i = NR_MPOINTS-1; i >= 0; i--) {
		mp = &mpoints[i];
		if (mp->new == NULL) {
			break;
		}
	}

	if (i < 0) {
		goto err1;
	}

	mp->new = cn;
	mp->old = c;

	return 0;

err1:
	chanclose(cn);
err0:
	return -1;
}

/*******************************************************************************
 * This function registers the path new as a mount point for the path old.
 ******************************************************************************/
int bind(const char *old, const char *new)
{
	chan_t *c;

	c = namec(old, O_BIND);
	if (c == NULL) {
		return -1;
	}

	if (addmntpoint(c, new) < 0) {
		chanclose(c);
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * This function calls the mount function of the driver associated to the path
 * srv.
 * It mounts the path srv on the path where.
 ******************************************************************************/
int mount(const char *srv, const char *where, const char *spec)
{
	chan_t *cs, *c;
	int ret;

	cs = namec(srv, O_RDWR);
	if (cs == NULL) {
		goto err0;
	}

	c = devtab[cs->type]->mount(cs, spec);
	if (c == NULL) {
		goto err1;
	}

	ret = addmntpoint(c, where);
	if (ret < 0) {
		goto err2;
	}

	chanclose(cs);

	return 0;

err2:
	chanclose(c);
err1:
	chanclose(cs);
err0:
	return -1;
}

/*******************************************************************************
 * This function initializes the device environment.
 * It creates the '/' channel.
 * It links the device drivers to the physical drivers.
 ******************************************************************************/
void debugfs_init(void)
{
	chan_t *c, *clone_ret;

	for (c = fdset; c < &fdset[NR_CHANS]; c++) {
		chan_clear(c);
	}

	c = devattach('/', 0);
	if (c == NULL) {
		panic();
	}

	clone_ret = clone(c, &slash);
	if (clone_ret == NULL) {
		panic();
	}

	chanclose(c);
	devlink();
}

__dead2 void devpanic(const char *cause)
{
	panic();
}
