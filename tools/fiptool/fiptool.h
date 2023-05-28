/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIPTOOL_H
#define FIPTOOL_H

#include <stddef.h>
#include <stdint.h>

#include <firmware_image_package.h>
#include <uuid.h>

#include "fiptool_platform.h"

#define NELEM(x) (sizeof (x) / sizeof *(x))

#define STRING_FIP_CORRUPT_ADDR_SPACE \
	"FIP %s corrupted: entry size exceeds 64 bit address space"
#define STRING_FIP_CORRUPT_ENTRY_SIZE \
	"FIP %s corrupted: entry size exceeds FIP file size"

enum {
	DO_UNSPEC = 0,
	DO_PACK   = 1,
	DO_UNPACK = 2,
	DO_REMOVE = 3
};

enum {
	LOG_DBG,
	LOG_WARN,
	LOG_ERR
};

typedef struct image_desc {
	uuid_t             uuid;
	char              *name;
	char              *cmdline_name;
	int                action;
	char              *action_arg;
	struct image      *image;
	struct image_desc *next;
} image_desc_t;

typedef struct image {
	struct fip_toc_entry toc_e;
	void                *buffer;
} image_t;

typedef struct cmd {
	char              *name;
	int              (*handler)(int, char **);
	void             (*usage)(int);
} cmd_t;

int info_cmd(int argc, char *argv[]);
int create_cmd(int argc, char *argv[]);
void create_usage(int exit_status);
int update_cmd(int argc, char *argv[]);
void parse_plat_toc_flags(const char *arg,
    unsigned long long *toc_flags);
void update_fip(void);
int unpack_cmd(int argc, char *argv[]);
void unpack_usage(int exit_status);
int parse_fip(const char *filename, fip_toc_header_t *toc_header_out);
image_desc_t *lookup_image_desc_from_uuid(const uuid_t *uuid);
image_desc_t *new_image_desc(const uuid_t *uuid,
    const char *name, const char *cmdline_name);
void add_image_desc(image_desc_t *desc);
void set_image_desc_action(image_desc_t *desc, int action,
    const char *arg);
int pack_images(const char *filename, uint64_t toc_flags,
    unsigned long align);
int write_image_to_file(const image_t *image, const char *filename);
int remove_cmd(int argc, char *argv[]);
void remove_usage(int exit_status);
struct option *fill_common_opts(struct option *opts, size_t *nr_opts,
    int has_arg);
struct option *add_opt(struct option *opts, size_t *nr_opts,
    const char *name, int has_arg, int val);
image_desc_t *lookup_image_desc_from_opt(const char *opt);
unsigned long get_image_align(char *arg);
void parse_blob_opt(char *arg, uuid_t *uuid, char *filename,
    size_t len);
void update_usage(int exit_status);
image_t *read_image_from_file(const uuid_t *uuid, const char *filename);
void md_print(const unsigned char *md, size_t len);
void fill_image_descs(void);
image_desc_t *lookup_image_desc_from_opt(const char *opt);
void info_usage(int exit_status);
int version_cmd(int argc, char *argv[]);
void version_usage(int exit_status);
int help_cmd(int argc, char *argv[]);
void usage(void);

/* TODO: move these to separate library files */
void uuid_from_str(uuid_t *u, const char *s);
void vlog(int prio, const char *msg, va_list ap);
void log_dbgx(const char *msg, ...);
void log_warnx(const char *msg, ...);
void log_err(const char *msg, ...);
void log_errx(const char *msg, ...);
char *xstrdup(const char *s, const char *msg);
void *xmalloc(size_t size, const char *msg);
void *xzalloc(size_t size, const char *msg);
FILE *xfopen(const char *filename, const char *mode);
struct BLD_PLAT_STAT xfstat(FILE *fp, const char *filename);
void xfread(void *buf, size_t size, FILE *fp, const char *filename);
void xfwrite(void *buf, size_t size, FILE *fp, const char *filename);
void xfclose(FILE *fp, const char *filename);
void uuid_to_str(char *s, size_t len, const uuid_t *u);
int is_power_of_2(unsigned long x);

#endif /* FIPTOOL_H */
