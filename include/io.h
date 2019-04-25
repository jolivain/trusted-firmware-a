/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef IO_H
#define IO_H

#define NAMELEN   14
#define STATLEN   41
#define ROOTLEN   (2 + 4)
#define FILNAMLEN (2 + NAMELEN)
#define DIRLEN    (STATLEN + FILNAMLEN + 3*ROOTLEN)

typedef struct dir Dir;
typedef unsigned short Qid;

struct dir {
	char name[NAMELEN];
	unsigned long long length;
	unsigned char mode;
	unsigned char type;
	unsigned char dev;
	Qid qid;
};

struct dirent {
	char d_name[NAMELEN];
};

typedef struct {
	int fd;
	unsigned char buf[DIRLEN];
} DIR;

extern int dirtop9(Dir *dp, unsigned char *buf, int n);
extern int p9todir(Dir *dp, unsigned char *buf, int n);

extern DIR *opendir(const char *name);
extern int readdir(DIR *dir, struct dirent *ent);
extern int closedir(DIR *dir);

enum devflags {
	O_READ   = 1 << 0,
	O_WRITE  = 1 << 1,
	O_RDWR   = 1 << 2,
	O_BIND   = 1 << 3,
	O_DIR    = 1 << 4,
};

/* driver functions */
extern int mount(char *srv, char *mnt, char *spec);
extern int create(const char *name, int flags);
extern int open(const char *name, int flags);
extern int close(int fd);
extern int read(int fd, void *buf, int n);
extern int write(int fd, void *buf, int n);
extern int seek(int fd, long long off, int whence);
extern int bind(char *path, char *where);

#endif /* IO_H */
