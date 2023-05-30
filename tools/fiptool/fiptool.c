/*
 * Copyright (c) 2016-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MSC_VER
#include <sys/mount.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fiptool.h"
#include "tbbr_config.h"

image_desc_t *image_desc_head;
size_t nr_image_descs;
const uuid_t uuid_null;
int verbose;

/* Available subcommands. */
cmd_t cmds[] = {
{ .name = "info",    .handler = cmd_info,    .usage = cmd_info_usage	},
{ .name = "create",  .handler = cmd_create,  .usage = cmd_create_usage	},
{ .name = "update",  .handler = cmd_update,  .usage = cmd_update_usage	},
{ .name = "unpack",  .handler = cmd_unpack,  .usage = cmd_unpack_usage	},
{ .name = "remove",  .handler = cmd_remove,  .usage = cmd_remove_usage	},
{ .name = "version", .handler = cmd_version, .usage = cmd_version_usage	},
{ .name = "help",    .handler = cmd_help,    .usage = NULL		},
};

int
main(int argc, char *argv[])
{
	int i, c, opt_index = 0, ret = 0;
	struct option opts[] = {
		{ "verbose", no_argument, NULL, 'v' },
		{ NULL, no_argument, NULL, 0 }
	};
	while ((c = getopt_long(argc, argv, "+v", opts, &opt_index)) != -1) {
		switch (c) {
		case 'v':
			verbose = 1;
			break;
		default:
			usage_main();
		}
	}
	argc -= optind, argv += optind;
	/* Reset optind for subsequent getopt processing. */
	optind = 0;

	if (argc == 0)
		usage_main();

	fill_image_descs();
	for (i = 0; i < NELEM(cmds); i++) {
		if (strcmp(cmds[i].name, argv[0]) == 0) {
			ret = cmds[i].handler(argc, argv);
			break;
		}
	}
	if (i == NELEM(cmds))
		usage_main();
	if (ret)
		(void)set_errno();
	if (errno)
		err(ERR, NULL);
	return 0;
}

int
cmd_info(int argc, char *argv[])
{
	image_desc_t *desc;
	fip_toc_header_t toc_header;

	if (argc != 2)
		cmd_info_usage(set_errno());
	argc--, argv++;

	parse_fip(argv[0], &toc_header);
	if (verbose) {
		err(DBG, "toc_header[name]: 0x%llX",
		    (unsigned long long)toc_header.name);
		err(DBG, "toc_header[serial_number]: 0x%llX",
		    (unsigned long long)toc_header.serial_number);
		err(DBG, "toc_header[flags]: 0x%llX",
		    (unsigned long long)toc_header.flags);
	}

	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		image_t *image = desc->image;

		if (image == NULL)
			continue;
		printf("%s: offset=0x%llX, size=0x%llX, cmdline=\"--%s\"",
		       desc->name,
		       (unsigned long long)image->toc_e.offset_address,
		       (unsigned long long)image->toc_e.size,
		       desc->cmdline_name);
#ifndef _MSC_VER	/* We don't have SHA256 for Visual Studio. */
		if (verbose) {
			unsigned char md[SHA256_DIGEST_LENGTH];

			SHA256(image->buffer, image->toc_e.size, md);
			printf(", sha256=");
			md_print(md, sizeof(md));
		}
#endif
		putchar('\n');
	}
	return 0;
}

int
cmd_create(int argc, char *argv[])
{
	int c, opt_index = 0;
	struct option *opts = NULL;
	size_t nr_opts = 0;
	unsigned long long toc_flags = 0;
	unsigned long align = 1;

	if (argc < 2)
		cmd_create_usage(set_errno());

	opts = fill_common_opts(opts, &nr_opts, required_argument);
	opts = add_opt(opts, &nr_opts, "plat-toc-flags", required_argument,
	    OPT_PLAT_TOC_FLAGS);
	opts = add_opt(opts, &nr_opts, "align", required_argument, OPT_ALIGN);
	opts = add_opt(opts, &nr_opts, "blob", required_argument, 'b');
	opts = add_opt(opts, &nr_opts, NULL, 0, 0);

	while ((c = getopt_long(argc, argv, "b:", opts, &opt_index)) != -1) {
		switch (c) {
		case OPT_TOC_ENTRY: {
			image_desc_t *desc =
			    lookup_image_desc_from_opt(opts[opt_index].name);
			set_image_desc_action(desc, DO_PACK, optarg);
			break;
		}
		case OPT_PLAT_TOC_FLAGS:
			parse_plat_toc_flags(optarg, &toc_flags);
			break;
		case OPT_ALIGN:
			align = get_image_align(optarg);
			break;
		case 'b': {
			char name[_UUID_STR_LEN + 1];
			char filename[PATH_MAX] = { 0 };
			uuid_t uuid = uuid_null;
			image_desc_t *desc;

			parse_blob_opt(optarg, &uuid,
			    filename, sizeof(filename));

			if (memcmp(&uuid, &uuid_null, sizeof(uuid_t)) == 0 ||
			    filename[0] == '\0')
				cmd_create_usage(set_errno());

			desc = lookup_image_desc_from_uuid(&uuid);
			if (desc == NULL) {
				uuid_to_str(name, sizeof(name), &uuid);
				desc = new_image_desc(&uuid, name, "blob");
				add_image_desc(desc);
			}
			set_image_desc_action(desc, DO_PACK, filename);
			break;
		}
		default:
			cmd_create_usage(set_errno());
		}
	}
	if ((argc -= optind) == 0)
		cmd_create_usage(0);
	argv += optind;

	update_fip();
	pack_images(argv[0], toc_flags, align);
	return 0;
}

int
cmd_update(int argc, char *argv[])
{
	int c, opt_index = 0, pflag = 0;
	struct option *opts = NULL;
	size_t nr_opts = 0;
	char outfile[PATH_MAX] = { 0 };
	fip_toc_header_t toc_header = { 0 };
	unsigned long long toc_flags = 0;
	unsigned long align = 1;

	if (argc < 2)
		cmd_update_usage(set_errno());

	opts = fill_common_opts(opts, &nr_opts, required_argument);
	opts = add_opt(opts, &nr_opts, "align", required_argument, OPT_ALIGN);
	opts = add_opt(opts, &nr_opts, "blob", required_argument, 'b');
	opts = add_opt(opts, &nr_opts, "out", required_argument, 'o');
	opts = add_opt(opts, &nr_opts, "plat-toc-flags", required_argument,
	    OPT_PLAT_TOC_FLAGS);
	opts = add_opt(opts, &nr_opts, NULL, 0, 0);

	while ((c = getopt_long(argc, argv, "b:o:", opts, &opt_index)) != -1) {
		switch (c) {
		case OPT_TOC_ENTRY: {
			image_desc_t *desc =
			    lookup_image_desc_from_opt(opts[opt_index].name);
			set_image_desc_action(desc, DO_PACK, optarg);
			break;
		}
		case OPT_PLAT_TOC_FLAGS:
			parse_plat_toc_flags(optarg, &toc_flags);
			pflag = 1;
			break;
		case 'b': {
			char name[_UUID_STR_LEN + 1];
			char filename[PATH_MAX] = { 0 };
			uuid_t uuid = uuid_null;
			image_desc_t *desc;

			parse_blob_opt(optarg, &uuid,
			    filename, sizeof(filename));

			if (memcmp(&uuid, &uuid_null, sizeof(uuid_t)) == 0 ||
			    filename[0] == '\0')
				cmd_update_usage(set_errno());

			desc = lookup_image_desc_from_uuid(&uuid);
			if (desc == NULL) {
				uuid_to_str(name, sizeof(name), &uuid);
				desc = new_image_desc(&uuid, name, "blob");
				add_image_desc(desc);
			}
			set_image_desc_action(desc, DO_PACK, filename);
			break;
		}
		case OPT_ALIGN:
			align = get_image_align(optarg);
			break;
		case 'o':
			snprintf(outfile, sizeof(outfile), "%s", optarg);
			break;
		default:
			cmd_update_usage(set_errno());
		}
	}
	argc -= optind, argv += optind;

	if (argc == 0)
		cmd_update_usage(0);
	if (outfile[0] == '\0')
		snprintf(outfile, sizeof(outfile), "%s", argv[0]);
	if (access(argv[0], F_OK) == 0)
		parse_fip(argv[0], &toc_header);
	if (pflag)
		toc_header.flags &= ~(0xffffULL << 32);
	toc_flags = (toc_header.flags |= toc_flags);

	update_fip();
	pack_images(outfile, toc_flags, align);
	return 0;
}

void
parse_plat_toc_flags(const char *arg, unsigned long long *toc_flags)
{
	char *endptr;
	errno = 0;
	unsigned long long flags = strtoull(arg, &endptr, 16);
	if (*endptr != '\0' || flags > UINT16_MAX || errno != 0)
		err(ERR, "Invalid platform ToC flags: %s", arg);
	/* Platform ToC flags is a 16-bit field occupying bits [32-47]. */
	*toc_flags |= flags << 32;
}

/*
 * This function is shared between the create and update subcommands.
 * The difference between the two subcommands is that when the FIP file
 * is created, the parsing of an existing FIP is skipped.  This results
 * in update_fip() creating the new FIP file from scratch because the
 * internal image table is not populated.
 */
void
update_fip(void)
{
	/* Add or replace images in the FIP file. */
	for (image_desc_t *desc = image_desc_head; desc != NULL;
	    desc = desc->next) {
		if (desc->action != DO_PACK)
			continue;
		image_t *image = read_image_from_file(&desc->uuid,
		    desc->action_arg);
		if (desc->image != NULL) {
			if (verbose)
				err(DBG, "Replacing %s with %s",
				    desc->cmdline_name,
				    desc->action_arg);
			free(desc->image);
		} else if (verbose)
			err(DBG, "Adding image %s", desc->action_arg);
		desc->image = image;
	}
}

int
cmd_unpack(int argc, char *argv[])
{
	int c, opt_index = 0, fflag = 0, unpack_all = 1;
	struct option *opts = NULL;
	size_t nr_opts = 0;
	char outdir[PATH_MAX] = { 0 };
	image_desc_t *desc;

	if (argc < 2)
		cmd_unpack_usage(set_errno());

	opts = fill_common_opts(opts, &nr_opts, required_argument);
	opts = add_opt(opts, &nr_opts, "blob", required_argument, 'b');
	opts = add_opt(opts, &nr_opts, "force", no_argument, 'f');
	opts = add_opt(opts, &nr_opts, "out", required_argument, 'o');
	opts = add_opt(opts, &nr_opts, NULL, 0, 0);

	while ((c = getopt_long(argc, argv, "b:fo:", opts, &opt_index)) != -1) {
		switch (c) {
		case OPT_TOC_ENTRY: {
			image_desc_t *desc;

			desc = lookup_image_desc_from_opt(opts[opt_index].name);
			set_image_desc_action(desc, DO_UNPACK, optarg);
			unpack_all = 0;
			break;
		}
		case 'b': {
			char name[_UUID_STR_LEN + 1];
			char filename[PATH_MAX] = { 0 };
			uuid_t uuid = uuid_null;
			image_desc_t *desc;

			parse_blob_opt(optarg, &uuid,
			    filename, sizeof(filename));

			if (memcmp(&uuid, &uuid_null, sizeof(uuid_t)) == 0 ||
			    filename[0] == '\0')
				cmd_unpack_usage(set_errno());

			desc = lookup_image_desc_from_uuid(&uuid);
			if (desc == NULL) {
				uuid_to_str(name, sizeof(name), &uuid);
				desc = new_image_desc(&uuid, name, "blob");
				add_image_desc(desc);
			}
			set_image_desc_action(desc, DO_UNPACK, filename);
			unpack_all = 0;
			break;
		}
		case 'f':
			fflag = 1;
			break;
		case 'o':
			snprintf(outdir, sizeof(outdir), "%s", optarg);
			break;
		default:
			cmd_unpack_usage(set_errno());
		}
	}
	argc -= optind, argv += optind;
	if (argc == 0)
		cmd_unpack_usage(0);

	parse_fip(argv[0], NULL);

	if (outdir[0] != '\0')
		if (chdir(outdir) == -1)
			err(ERR, "chdir %s", outdir);

	/* Unpack all specified images. */
	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		char file[PATH_MAX];
		image_t *image = desc->image;

		if (!unpack_all && desc->action != DO_UNPACK)
			continue;
		/* Build filename. */
		if (desc->action_arg == NULL)
			snprintf(file, sizeof(file), "%s.bin",
			    desc->cmdline_name);
		else
			snprintf(file, sizeof(file), "%s",
			    desc->action_arg);
		if (image == NULL) {
			if (!unpack_all)
				err(WARN, "%s does not exist in %s",
				    file, argv[0]);
			continue;
		}
		if (fflag || access(file, F_OK) != 0) {
			if (verbose)
				err(DBG, "Unpacking %s", file);
			write_image_to_file(image, file);
		} else
			err(WARN, "File %s exists, use --force to overwrite",
			    file);
	}
	return 0;
}

int
parse_fip(const char *filename, fip_toc_header_t *toc_header_out)
{
	char *buf, *bufend;
	fip_toc_header_t *toc_header;
	fip_toc_entry_t *toc_entry;
	int terminated = 0;
	size_t st_size;
	FILE *fp = xfopen(filename, "rb");
	struct BLD_PLAT_STAT st = xfstat(fp, filename);

	st_size = st.st_size;
#ifdef BLKGETSIZE64
	if ((st.st_mode & S_IFBLK) != 0)
		if (ioctl(fileno(fp), BLKGETSIZE64, &st_size) == -1)
			err(ERR, "ioctl %s", filename);
#endif
	buf = xmalloc(st_size, "failed to load file into memory");
	xfread(buf, st_size, fp, filename);
	bufend = buf + st_size;
	xfclose(fp, filename);

	if (st_size < sizeof(fip_toc_header_t))
		err(ERR, "FIP %s is truncated", filename);

	toc_header = (fip_toc_header_t *)buf;
	toc_entry = (fip_toc_entry_t *)(toc_header + 1);
	if (toc_header->name != TOC_HEADER_NAME)
		err(ERR, "%s is not a FIP file", filename);
	/* Return the ToC header if the caller wants it. */
	if (toc_header_out != NULL)
		*toc_header_out = *toc_header;

	/* Walk through each ToC entry in the file. */
	while ((char *)toc_entry + sizeof(*toc_entry) - 1 < bufend) {
		image_t *image;
		image_desc_t *desc;

		/* Found the ToC terminator, we are done. */
		if (memcmp(&toc_entry->uuid, &uuid_null, sizeof(uuid_t)) == 0) {
			terminated = 1;
			break;
		}
		/*
		 * Build a new image out of the ToC entry and add it to the
		 * table of images.
		 */
		image = xzalloc(sizeof(*image),
		    "failed to allocate memory for image");
		image->toc_e = *toc_entry;
		image->buffer = xmalloc(toc_entry->size,
		    "failed to allocate image buffer, is FIP file corrupted?");
		/* Overflow checks before memory copy. */
		if (toc_entry->size > (uint64_t)-1 - toc_entry->offset_address)
			err(ERR, STRING_FIP_CORRUPT_ADDR_SPACE, filename);
		if (toc_entry->size + toc_entry->offset_address > st_size)
			err(ERR, STRING_FIP_CORRUPT_ENTRY_SIZE, filename);

		memcpy(image->buffer, buf + toc_entry->offset_address,
		    toc_entry->size);

		/* If this is an unknown image, create a descriptor for it. */
		desc = lookup_image_desc_from_uuid(&toc_entry->uuid);
		if (desc == NULL) {
			char name[_UUID_STR_LEN + 1], filename[PATH_MAX];

			uuid_to_str(name, sizeof(name), &toc_entry->uuid);
			snprintf(filename, sizeof(filename), "%s%s",
			    name, ".bin");
			desc = new_image_desc(&toc_entry->uuid, name, "blob");
			desc->action = DO_UNPACK;
			desc->action_arg = xstrdup(filename,
			    "failed to allocate memory for blob filename");
			add_image_desc(desc);
		}
		assert(desc->image == NULL);
		desc->image = image;
		toc_entry++;
	}
	if (terminated == 0)
		err(ERR, "FIP %s does not have a ToC terminator entry",
		    filename);
	free(buf);
	return 0;
}

image_desc_t
*lookup_image_desc_from_uuid(const uuid_t *uuid)
{
	image_desc_t *desc;
	for (desc = image_desc_head; desc != NULL; desc = desc->next)
		if (memcmp(&desc->uuid, uuid, sizeof(uuid_t)) == 0)
			return desc;
	return NULL;
}

image_desc_t
*new_image_desc(const uuid_t *uuid,
    const char *name, const char *cmdline_name)
{
	image_desc_t *desc = xzalloc(sizeof(*desc),
	    "failed to allocate memory for image descriptor");
	memcpy(&desc->uuid, uuid, sizeof(uuid_t));
	desc->name = xstrdup(name,
	    "failed to allocate memory for image name");
	desc->cmdline_name = xstrdup(cmdline_name,
	    "failed to allocate memory for image command line name");
	desc->action = DO_UNSPEC;
	return desc;
}

void
add_image_desc(image_desc_t *desc)
{
	image_desc_t **p = &image_desc_head;
	while (*p)
		p = &(*p)->next;
	assert(*p == NULL);
	*p = desc;
	nr_image_descs++;
}

void
set_image_desc_action(image_desc_t *desc, int action,
    const char *arg)
{
	assert(desc != NULL);
	if (desc->action_arg != (char *)DO_UNSPEC)
		free(desc->action_arg);
	desc->action = action;
	desc->action_arg = NULL;
	if (arg != NULL)
		desc->action_arg = xstrdup(arg,
		    "failed to allocate memory for argument");
}

int
pack_images(const char *filename, uint64_t toc_flags,
    unsigned long align)
{
	FILE *fp;
	image_desc_t *desc;
	fip_toc_header_t *toc_header;
	fip_toc_entry_t *toc_entry;
	char *buf;
	uint64_t entry_offset, buf_size, payload_size = 0, pad_size;
	size_t nr_images = 0;

	for (desc = image_desc_head; desc != NULL; desc = desc->next)
		if (desc->image != NULL)
			nr_images++;

	buf_size = sizeof(fip_toc_header_t) +
	    sizeof(fip_toc_entry_t) * (nr_images + 1);
	if ((buf = calloc(1, buf_size)) == NULL)
		err(ERR, "calloc");

	/* Build up header and ToC entries from the image table. */
	toc_header = (fip_toc_header_t *)buf;
	toc_header->name = TOC_HEADER_NAME;
	toc_header->serial_number = TOC_HEADER_SERIAL_NUMBER;
	toc_header->flags = toc_flags;
	toc_entry = (fip_toc_entry_t *)(toc_header + 1);

	entry_offset = buf_size;
	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		image_t *image = desc->image;
		if (image == NULL || (image->toc_e.size == 0ULL))
			continue;
		payload_size += image->toc_e.size;
		entry_offset = (entry_offset + align - 1) & ~(align - 1);
		image->toc_e.offset_address = entry_offset;
		*toc_entry++ = image->toc_e;
		entry_offset += image->toc_e.size;
	}

	/*
	 * Append a null uuid entry to mark the end of ToC entries.
	 * NOTE the offset address for the last toc_entry must match the fip
	 * size.
	 */
	memset(toc_entry, 0, sizeof(*toc_entry));
	toc_entry->offset_address = (entry_offset + align - 1) & ~(align - 1);

	/* Generate the FIP file. */
	fp = xfopen(filename, "wb");
	if (verbose)
		err(DBG, "Metadata size: %zu bytes", buf_size);

	xfwrite(buf, buf_size, fp, filename);
	if (verbose)
		err(DBG, "Payload size: %zu bytes", payload_size);

	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		image_t *image = desc->image;
		if (image == NULL)
			continue;
		if (fseek(fp, image->toc_e.offset_address, SEEK_SET) == -1)
			err(ERR, "Failed to set file position");
		xfwrite(image->buffer, image->toc_e.size, fp, filename);
	}
	if (fseek(fp, entry_offset, SEEK_SET) == -1)
		err(ERR, "Failed to set file position");

	pad_size = toc_entry->offset_address - entry_offset;
	while (pad_size--)
		fputc(0x0, fp);

	free(buf);
	xfclose(fp, filename);
	return 0;
}

int
write_image_to_file(const image_t *image, const char *filename)
{
	FILE *fp = xfopen(filename, "wb");
	xfwrite(image->buffer, image->toc_e.size, fp, filename);
	xfclose(fp, filename);
	return 0;
}

int
cmd_remove(int argc, char *argv[])
{
	int c, opt_index = 0, fflag = 0;
	struct option *opts = NULL;
	size_t nr_opts = 0;
	char outfile[PATH_MAX] = { 0 };
	fip_toc_header_t toc_header;
	image_desc_t *desc;
	unsigned long align = 1;

	if (argc < 2)
		cmd_remove_usage(set_errno());

	opts = fill_common_opts(opts, &nr_opts, no_argument);
	opts = add_opt(opts, &nr_opts, "align", required_argument, OPT_ALIGN);
	opts = add_opt(opts, &nr_opts, "blob", required_argument, 'b');
	opts = add_opt(opts, &nr_opts, "force", no_argument, 'f');
	opts = add_opt(opts, &nr_opts, "out", required_argument, 'o');
	opts = add_opt(opts, &nr_opts, NULL, 0, 0);

	while ((c = getopt_long(argc, argv, "b:fo:", opts, &opt_index)) != -1) {
		switch (c) {
		case OPT_TOC_ENTRY: {
			image_desc_t *desc;

			desc = lookup_image_desc_from_opt(opts[opt_index].name);
			set_image_desc_action(desc, DO_REMOVE, NULL);
			break;
		}
		case OPT_ALIGN:
			align = get_image_align(optarg);
			break;
		case 'b': {
			char name[_UUID_STR_LEN + 1], filename[PATH_MAX];
			uuid_t uuid = uuid_null;
			image_desc_t *desc;

			parse_blob_opt(optarg, &uuid,
			    filename, sizeof(filename));

			if (memcmp(&uuid, &uuid_null, sizeof(uuid_t)) == 0)
				cmd_remove_usage(set_errno());

			desc = lookup_image_desc_from_uuid(&uuid);
			if (desc == NULL) {
				uuid_to_str(name, sizeof(name), &uuid);
				desc = new_image_desc(&uuid, name, "blob");
				add_image_desc(desc);
			}
			set_image_desc_action(desc, DO_REMOVE, NULL);
			break;
		}
		case 'f':
			fflag = 1;
			break;
		case 'o':
			snprintf(outfile, sizeof(outfile), "%s", optarg);
			break;
		default:
			cmd_remove_usage(set_errno());
		}
	}
	argc -= optind, argv += optind;

	if (argc == 0)
		cmd_remove_usage(0);
	if (outfile[0] != '\0' && access(outfile, F_OK) == 0 && !fflag)
		err(ERR, "File %s exists, use --force to overwrite it",
		    outfile);
	if (outfile[0] == '\0')
		snprintf(outfile, sizeof(outfile), "%s", argv[0]);

	parse_fip(argv[0], &toc_header);

	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		if (desc->action != DO_REMOVE)
			continue;
		if (desc->image != NULL) {
			if (verbose)
				err(DBG, "Removing %s",
				    desc->cmdline_name);
			free(desc->image);
			desc->image = NULL;
		} else
			err(WARN, "%s does not exist in %s",
			    desc->cmdline_name, argv[0]);
	}
	pack_images(outfile, toc_header.flags, align);
	return 0;
}

struct option
*fill_common_opts(struct option *opts, size_t *nr_opts,
    int has_arg)
{
	image_desc_t *desc;
	for (desc = image_desc_head; desc != NULL; desc = desc->next)
		opts = add_opt(opts, nr_opts, desc->cmdline_name, has_arg,
		    OPT_TOC_ENTRY);
	return opts;
}

struct option
*add_opt(struct option *opts, size_t *nr_opts,
    const char *name, int has_arg, int val)
{
	if ((opts = realloc(opts, (*nr_opts + 1) * sizeof(*opts))) == NULL)
		err(ERR, "realloc");
	opts[*nr_opts].name = name;
	opts[*nr_opts].has_arg = has_arg;
	opts[*nr_opts].flag = NULL;
	opts[*nr_opts].val = val;
	++*nr_opts;
	return opts;
}

image_desc_t
*lookup_image_desc_from_opt(const char *opt)
{
	image_desc_t *desc;
	for (desc = image_desc_head; desc != NULL; desc = desc->next)
		if (strcmp(desc->cmdline_name, opt) == 0)
			return desc;
	return NULL;
}

unsigned long
get_image_align(char *arg)
{
	char *endptr;
	unsigned long align;

	errno = 0;
	align = strtoul(arg, &endptr, 0);
	if (*endptr != '\0' || !IS_POWER_OF_2(align) || errno != 0)
		err(ERR, "Invalid alignment: %s", arg);
	return align;
}

void
parse_blob_opt(char *arg, uuid_t *uuid, char *filename, size_t len)
{
	for (char *p = strtok(arg, ","); p != NULL; p = strtok(NULL, ",")) {
		if (strncmp(p, "uuid=", strlen("uuid=")) == 0) {
			p += strlen("uuid=");
			uuid_from_str(uuid, p);
		} else if (strncmp(p, "file=", strlen("file=")) == 0) {
			p += strlen("file=");
			snprintf(filename, len, "%s", p);
		}
	}
}

image_t
*read_image_from_file(const uuid_t *uuid, const char *filename)
{
	assert(uuid != NULL);
	assert(filename != NULL);

	FILE *fp = xfopen(filename, "rb");
	struct BLD_PLAT_STAT st = xfstat(fp, filename);
	image_t *image = xzalloc(sizeof(*image),
	    "failed to allocate memory for image");

	image->toc_e.uuid = *uuid;
	image->buffer = xmalloc(st.st_size, "failed to allocate image buffer");
	xfread(image->buffer, st.st_size, fp, filename);
	image->toc_e.size = st.st_size;

	xfclose(fp, filename);
	return image;
}

void
md_print(const unsigned char *md, size_t len)
{
	for (size_t i = 0; i < len; i++)
		printf("%02x", md[i]);
}

void
fill_image_descs(void)
{
	toc_entry_t *toc_entry;
	for (toc_entry = toc_entries;
	     toc_entry->cmdline_name != NULL;
	     toc_entry++) {
		image_desc_t *desc;
		desc = new_image_desc(&toc_entry->uuid,
		    toc_entry->name,
		    toc_entry->cmdline_name);
		add_image_desc(desc);
	}
#ifdef PLAT_DEF_FIP_UUID
	for (toc_entry = plat_def_toc_entries;
	     toc_entry->cmdline_name != NULL;
	     toc_entry++) {
		image_desc_t *desc;
		desc = new_image_desc(&toc_entry->uuid,
		    toc_entry->name,
		    toc_entry->cmdline_name);
		add_image_desc(desc);
	}
#endif
}

int
cmd_version(int argc, char *argv[])
{
#ifdef VERSION
	puts(VERSION);
#else
	/* If built from fiptool directory, VERSION is not set. */
	puts("Unknown version");
#endif
	return 0;
}

int
cmd_help(int argc, char *argv[])
{
	int i;
	if (argc < 2)
		usage_main();
	argc--, argv++;
	for (i = 0; i < NELEM(cmds); i++) {
		if (strcmp(cmds[i].name, argv[0]) == 0 &&
		    cmds[i].usage != NULL)
			cmds[i].usage(0);
	}
	if (i == NELEM(cmds))
		printf("No help for subcommand '%s'\n", argv[0]);
	return 0;
}

void
uuid_from_str(uuid_t *u, const char *s)
{
	if (s == NULL)
		err(ERR, "UUID cannot be NULL");
	if (strlen(s) != _UUID_STR_LEN)
		err(ERR, "Invalid UUID: %s", s);
	int n = sscanf(s,
	    "%2hhx%2hhx%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
	    &u->time_low[0], &u->time_low[1], &u->time_low[2], &u->time_low[3],
	    &u->time_mid[0], &u->time_mid[1],
	    &u->time_hi_and_version[0], &u->time_hi_and_version[1],
	    &u->clock_seq_hi_and_reserved, &u->clock_seq_low,
	    &u->node[0], &u->node[1],
	    &u->node[2], &u->node[3],
	    &u->node[4], &u->node[5]);
	/*
	 * Given the format specifier above, we expect 16 items to be scanned
	 * for a properly formatted UUID.
	 */
	if (n != 16)
		err(ERR, "Invalid UUID: %s", s);
}

char
*xstrdup(const char *s, const char *msg)
{
	char *d;
	if ((d = strdup(s)) == NULL)
		err(ERR, "strdup: %s", msg);
	return d;
}

void
*xmalloc(size_t size, const char *msg)
{
	void *d;
	if ((d = malloc(size)) == NULL)
		err(ERR, "malloc: %s", msg);
	return d;
}

void
*xzalloc(size_t size, const char *msg)
{
	return memset(xmalloc(size, msg), 0, size);
}

FILE
*xfopen(const char *filename, const char *mode)
{
	FILE *fp;
	if ((fp = fopen(filename, mode)) == NULL)
		err(ERR, "fopen %s, mode %s", filename, mode);
	return fp;
}

struct BLD_PLAT_STAT
xfstat(FILE *fp, const char *filename)
{
	struct BLD_PLAT_STAT st;
	if (fstat(fileno(fp), &st) == -1)
		err(ERR, "fstat %s", filename);
	return st;
}

void
xfread(void *buf, size_t size, FILE *fp, const char *filename)
{
	if (fread(buf, 1, size, fp) != size)
		err(ERR, "Failed to read %s", filename);
}

void
xfwrite(void *buf, size_t size, FILE *fp, const char *filename)
{
	if (fwrite(buf, 1, size, fp) != size)
		err(ERR, "Failed to write %s", filename);
}

void
xfclose(FILE *fp, const char *filename)
{
	if (fclose(fp) == EOF)
		err(ERR, "%s", filename);
}

void
uuid_to_str(char *s, size_t len, const uuid_t *u)
{
	assert(len >= (_UUID_STR_LEN + 1));
	snprintf(s, len,
	    "%02X%02X%02X%02X-%02X%02X-%02X%02X-%04X-%04X%04X%04X",
	    u->time_low[0], u->time_low[1], u->time_low[2], u->time_low[3],
	    u->time_mid[0], u->time_mid[1],
	    u->time_hi_and_version[0], u->time_hi_and_version[1],
	    (u->clock_seq_hi_and_reserved << 8) | u->clock_seq_low,
	    (u->node[0] << 8) | u->node[1],
	    (u->node[2] << 8) | u->node[3],
	    (u->node[4] << 8) | u->node[5]);
}

void
err(int prio, const char *msg, ...)
{
	char *prefix[] = { "DEBUG", "WARN", "ERROR" };
	va_list ap;
	assert_err(&prio, (prio < 0) || (prio > 2), "Bad error type given");
	assert_err(&prio, (prio != ERR) && (!msg),
	    "Null log message in non-error condition.");
	fprintf(stderr, "%s: ", prefix[prio]);
	if (msg) {
		va_start(ap, msg);
		vfprintf(stderr, msg, ap);
		va_end(ap);
		if (prio == ERR)
			fprintf(stderr, ": ");
		else
			fprintf(stderr, "\n");
	}
	if (prio == ERR) {
		(void)set_errno();
		fprintf(stderr, "%s\n", strerror(errno));
		exit(errno);
	}
}

void
assert_err(int *prio, int condition, const char *msg)
{
	if (!condition)
		return;
	err(WARN, "%s", msg);
	fprintf(stderr, "Assuming error condition.\n");
	*prio = ERR;
}

int
set_errno(void)
{
	if (!errno)
		errno = ECANCELED;
	return errno;
}

