/*
 * Copyright (c) 2016-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIPTOOL_H
#define FIPTOOL_H

#include <stddef.h>
#include <stdint.h>

#include <firmware_image_package.h>
#include <uuid.h>
#include <errno.h>

#include "fiptool_platform.h"
#include "tbbr_config.h"

#define NELEM(x) (sizeof (x) / sizeof *(x))

#define STRING_FIP_CORRUPT_ADDR_SPACE \
	"FIP %s corrupted: entry size exceeds 64 bit address space"
#define STRING_FIP_CORRUPT_ENTRY_SIZE \
	"FIP %s corrupted: entry size exceeds FIP file size"

#define OPT_TOC_ENTRY 0
#define OPT_PLAT_TOC_FLAGS 1
#define OPT_ALIGN 2

#define IS_POWER_OF_2(x) (x && !(x & (x - 1)))
#define SET_ERRNO() errno = (!errno) ? ECANCELED : errno

enum {
	DO_UNSPEC = 0,
	DO_PACK   = 1,
	DO_UNPACK = 2,
	DO_REMOVE = 3
};

enum {
	DBG,
	WARN,
	ERR
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
	void              (*handler)(int, char **);
	void             (*usage)(int);
} cmd_t;

void cmd_info(int argc, char *argv[]);
void cmd_create(int argc, char *argv[]);
void cmd_update(int argc, char *argv[]);
void parse_plat_toc_flags(const char *arg,
    unsigned long long *toc_flags);
void update_fip(void);
void cmd_unpack(int argc, char *argv[]);
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
void cmd_remove(int argc, char *argv[]);
struct option *fill_common_opts(struct option *opts, size_t *nr_opts,
    int has_arg);
struct option *add_opt(struct option *opts, size_t *nr_opts,
    const char *name, int has_arg, int val);
image_desc_t *lookup_image_desc_from_opt(const char *opt);
unsigned long get_image_align(char *arg);
void parse_blob_opt(char *arg, uuid_t *uuid, char *filename,
    size_t len);
image_t *read_image_from_file(const uuid_t *uuid, const char *filename);
void md_print(const unsigned char *md, size_t len);
void fill_image_descs(void);
image_desc_t *lookup_image_desc_from_opt(const char *opt);
void cmd_version(int argc, char *argv[]);
void cmd_help(int argc, char *argv[]);

void uuid_from_str(uuid_t *u, const char *s);
char *xstrdup(const char *s, const char *msg);
void *xmalloc(size_t size, const char *msg);
void *xzalloc(size_t size, const char *msg);
FILE *xfopen(const char *filename, const char *mode);
struct BLD_PLAT_STAT xfstat(FILE *fp, const char *filename);
void xfread(void *buf, size_t size, FILE *fp, const char *filename);
void xfwrite(void *buf, size_t size, FILE *fp, const char *filename);
void xfclose(FILE *fp, const char *filename);
void uuid_to_str(char *s, size_t len, const uuid_t *u);
void abort_on_err(const char *msg);
void err(int prio, const char *msg, ...);
void assert_err(int *prio, int condition, const char *msg);

void
usage_exit(int exit_status)
{
	if (!exit_status)
		exit(0);
	if (errno)
		err(ERR, NULL);
	exit(ECANCELED);
}

void
print_toc_entries(const char *argf)
{
	toc_entry_t *toc_entry = toc_entries;
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s%s\t%s\n", toc_entry->cmdline_name,
		    argf, toc_entry->name);
#ifdef PLAT_DEF_FIP_UUID
	toc_entry = plat_def_toc_entries;
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s%s\t%s\n", toc_entry->cmdline_name,
		    argf, toc_entry->name);
#endif
}

void
usage_main(void)
{
#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1)
		err(ERR, "pledge");
#endif
	printf(
	    "usage: fiptool [--verbose] <command> [<args>]\n\n"

	    "Global options supported:\n"
	    "  --verbose\tEnable verbose output for all commands.\n\n"

	    "Commands supported:\n"
	    "  info\t\tList images contained in FIP.\n"
	    "  create\tCreate a new FIP with the given images.\n"
	    "  update\tUpdate an existing FIP with the given images.\n"
	    "  unpack\tUnpack images from FIP.\n"
	    "  remove\tRemove images from FIP.\n"
	    "  version\tShow fiptool version.\n"
	    "  help\t\tShow help for given command.\n");
	usage_exit(errno = EINVAL);
}

void
cmd_create_usage(int exit_status)
{
#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1)
		err(ERR, "pledge");
#endif
	printf(
	    "fiptool create [opts] FIP_FILENAME\n\n"

	    "Options:\n"
	    "  --align <val>\t\tEach image is aligned to <val> (default: 1).\n"
	    "  --blob uuid=...,file=...\tAdd an image with the given UUID"
	        " pointed to by file.\n"
	    "  --plat-toc-flags <value>\t16-bit platform specific flag"
	        " field occupying bits 32-47 in 64-bit ToC header.\n\n"

	    "Specific images are packed with the following options:\n");
	print_toc_entries(" FILENAME");
	usage_exit(exit_status);
}

void
cmd_unpack_usage(int exit_status)
{
#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1)
		err(ERR, "pledge");
#endif
	printf(
	    "fiptool unpack [opts] FIP_FILENAME\n\n"

	    "Options:\n"
	    "  --blob uuid=...,file=...\tUnpack an image with the given"
	        " UUID to file.\n"
	    "  --force\t\t\tOverwrite the file, even if it already exists.\n"
	    "  --out path\t\t\tSet the output directory path.\n\n"

	    "Specific images are unpacked with the following options:\n");
	print_toc_entries(" FILENAME");
	printf(
	    "\nIf no options are provided, all images will be unpacked.\n");
	usage_exit(exit_status);
}

void
cmd_remove_usage(int exit_status)
{
#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1)
		err(ERR, "pledge");
#endif
	printf(
	    "fiptool remove [opts] FIP_FILENAME\n\n"

	    "Options:\n"
	    "  --align <val>\tEach image is aligned to <val> (default: 1).\n"
	    "  --blob uuid=...\tRemove an image with the given UUID.\n"
	    "  --force\t\tIf the output FIP file already exists, use"
	        " --force to overwrite it.\n"
	    "  --out FIP_FILENAME\tSet an alternative output FIP file.\n\n"

	    "Specific images are removed with the following options:\n");
	print_toc_entries(" ");
	usage_exit(exit_status);
}

void
cmd_update_usage(int exit_status)
{
#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1)
		err(ERR, "pledge");
#endif
	printf(
	    "fiptool update [opts] FIP_FILENAME\n\n"

	    "Options:\n"
	    "  --align <val>\t\tEach image is aligned to <val> (default: 1).\n"
	    "  --blob uuid=...,file=...\tAdd or update an image with the "
	        "given UUID pointed to by file.\n"
	    "  --out FIP_FILENAME\t\tSet an alternative output FIP file.\n"
	    "  --plat-toc-flags <val>\t16-bit platform specific flag"
	        " field occupying bits 32-47 in 64-bit ToC header.\n\n"

	    "Specific images are packed with the following options:\n");
	print_toc_entries(" FILENAME");
	usage_exit(exit_status);
}

void
cmd_info_usage(int exit_status)
{
#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1)
		err(ERR, "pledge");
#endif
	printf("fiptool info FIP_FILENAME\n");
	usage_exit(exit_status);
}

void
cmd_version_usage(int exit_status)
{
#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1)
		err(ERR, "pledge");
#endif
	printf("fiptool version\n");
	usage_exit(exit_status);
}

#endif /* FIPTOOL_H */
