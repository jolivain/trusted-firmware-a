/*
 * Copyright (c) 2018-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sptool.h"

#define SP_ALIGN		8

/*
 * Entry describing Secure Partition package.
 */
struct sp_pkg_info {
	/* Location of the files in the host's RAM. */
	void *img_data, *pm_data;

	/* Size of the files. */
	uint64_t img_size, pm_size;

	/* Location of the binary files inside the package output file */
	uint64_t img_offset, pm_offset;

};

/* Align an address to a power-of-two boundary. */
static unsigned int align_to(unsigned int address, unsigned int boundary)
{
	unsigned int mask = boundary - 1U;

	if ((address & mask) != 0U)
		return (address + boundary) & ~mask;
	else
		return address;
}

/* Allocate a memory area of 'size' bytes and zero it. */
static void *xzalloc(size_t size, const char *msg)
{
	void *d;

	d = malloc(size);
	if (d == NULL) {
		fprintf(stderr, "error: malloc: %s\n", msg);
		exit(1);
	}

	memset(d, 0, size);

	return d;
}

/*
 * Write 'size' bytes from 'buf' into the specified file stream.
 * Exit the program on error.
 */
static void xfwrite(void *buf, size_t size, FILE *fp)
{
	if (fwrite(buf, 1, size, fp) != size) {
		fprintf(stderr, "error: Failed to write to output file.\n");
		exit(1);
	}
}

/*
 * Set the file position indicator for the specified file stream.
 * Exit the program on error.
 */
static void xfseek(FILE *fp, long offset, int whence)
{
	if (fseek(fp, offset, whence) != 0) {
		fprintf(stderr, "error: Failed to set file to offset 0x%lx (%d).\n",
		       offset, whence);
		perror(NULL);
		exit(1);
	}
}

static void cleanup(struct sp_pkg_info *sp)
{

	if (sp != NULL) {
		if (sp->img_data != NULL) {
			free(sp->img_data);
		}

		if (sp->pm_data != NULL) {
			free(sp->pm_data);
		}

		free(sp);

	}
}

/*
 * Allocate a buffer big enough to store the content of the specified file and
 * load the file into it. Fill 'size' with the file size. Exit the program on
 * error.
 */
static void load_file(const char *path, void **ptr, uint64_t *size)
{
	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		fprintf(stderr, "error: %s couldn't be opened.\n", path);
		exit(1);
	}

	xfseek(f, 0, SEEK_END);
	*size = ftell(f);
	if (*size == 0) {
		fprintf(stderr, "error: Size of %s is 0\n", path);
		exit(1);
	}

	rewind(f);

	*ptr = malloc(*size);
	if (*ptr == NULL) {
		fprintf(stderr, "error: Not enough memory to load %s\n", path);
		exit(1);
	}

	if (fread(*ptr, *size, 1, f) != 1) {
		fprintf(stderr, "error: Couldn't read %s\n", path);
		exit(1);
	}

	fclose(f);
}

static void load_sp_rd(char *path, struct sp_pkg_info **sp_out)
{
	struct sp_pkg_info *sp;

	char *split_mark = strstr(path, ":");

	*split_mark = '\0';

	char *sp_path = path;
	char *rd_path = split_mark + 1;

	sp = xzalloc(sizeof(struct sp_pkg_info),
		"Failed to allocate sp_pkg_info struct");

	load_file(sp_path, &sp->img_data, &sp->img_size);
	printf("Loaded image file %s (%lu bytes)\n", sp_path, sp->img_size);

	load_file(rd_path, &sp->pm_data, &sp->pm_size);
	printf("Loaded RD file %s (%lu bytes)\n", rd_path, sp->pm_size);

	*sp_out = sp;
}

static void output_write(const char *path, struct sp_pkg_info *sp)
{
	struct sp_pkg_header sp_header_info;

	FILE *f = fopen(path, "wb");
	if (f == NULL) {
		fprintf(stderr, "error: Failed to open %s\n", path);
		exit(1);
	}

	/* First, save partition image aligned to 8 bytes */

	unsigned int file_ptr = align_to(sizeof(struct sp_pkg_header), SP_ALIGN);

	xfseek(f, file_ptr, SEEK_SET);
	printf("Writing Image blob to offset 0x%x (0x%lx bytes)\n",
	       file_ptr, sp->img_size);

	sp->img_offset = file_ptr;
	xfwrite(sp->img_data, sp->img_size, f);

	/* Now, save partition manifest blob aligned to 8 bytes */

	file_ptr = align_to(file_ptr + sp->img_size, SP_ALIGN);
	xfseek(f, file_ptr, SEEK_SET);
	printf("Writing RD blob to offset 0x%x (0x%lx bytes)\n",
	       file_ptr, sp->pm_size);

	sp->pm_offset = file_ptr;
	xfwrite(sp->pm_data, sp->pm_size, f);

	/* Finally, write header */

	sp_header_info.magic = SECURE_PARTITION_MAGIC;
	sp_header_info.version = 0x1;
	sp_header_info.img_offset = sp->img_offset;
	sp_header_info.img_size = sp->img_size;
	sp_header_info.pm_offset = sp->pm_offset;
	sp_header_info.pm_size = sp->pm_size;

	xfseek(f, 0, SEEK_SET);

	printf("Writing package header\n");

	xfwrite(&sp_header_info, sizeof(struct sp_pkg_header), f);

	/* All information has been written now */
	printf("\nsptool: Built Secure Partition blob %s\n\n", path);

	fclose(f);
}

static void usage(void)
{
	printf("usage: sptool ");
#ifdef VERSION
	printf(VERSION);
#else
	/* If built from sptool directory, VERSION is not set. */
	printf("version unknown");
#endif
	printf(" [<args>]\n\n");

	printf("This tool takes as input set of image binary files and the\n"
	       "partition manifest blobs as input and generates set of\n"
	       "output package files\n"
	       "Usage example: sptool -i sp1.bin:sp1.dtb -o sp1.pkg\n"
	       "                      -i sp2.bin:sp2.dtb -o sp2.pkg ...\n\n");
	printf("Commands supported:\n");
	printf("  -o <path>            Set output file path.\n");
	printf("  -i <sp_path:rd_path> Add Secure Partition image and Resource\n"
	       "                       Description blob (specified in two paths\n"
	       "                       separated by a colon).\n");
	printf("  -h                   Show this message.\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	struct sp_pkg_info *sp_info = NULL;
	int ch;

	if (argc <= 1) {
		fprintf(stderr, "error: File paths must be provided.\n\n");
		usage();
		return 1;
	}

	while ((ch = getopt(argc, argv, "hi:o:")) != -1) {
		switch (ch) {
		case 'i':
			load_sp_rd(optarg, &sp_info);
			break;
		case 'o':
			output_write(optarg, sp_info);
			break;
		case 'h':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	cleanup(sp_info);

	return 0;
}
