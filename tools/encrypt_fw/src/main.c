/*
 * Copyright (c) 2019, Linaro Limited
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <openssl/conf.h>

#include "cmd_opt.h"
#include "debug.h"
#include "encrypt.h"
#include "tbbr_config.h"

#define NUM_ELEM(x)			((sizeof(x)) / (sizeof(x[0])))
#define HELP_OPT_MAX_LEN		128

/* Global options */

/* Info messages created in the Makefile */
extern const char build_msg[];
extern const char platform_msg[];

static char *key_algs_str[] = {
	[KEY_ALG_GCM] = "gcm",
};

static char *strdup(const char *str)
{
	int n = strlen(str) + 1;
	char *dup = malloc(n);

	if (dup) {
		strcpy(dup, str);
	}
	return dup;
}

static void print_help(const char *cmd, const struct option *long_opt)
{
	int rem, i = 0;
	const struct option *opt;
	char line[HELP_OPT_MAX_LEN];
	char *p;

	assert(cmd != NULL);
	assert(long_opt != NULL);

	printf("\n\n");
	printf("The firmware encryption tool loads the binary images and\n"
	       "outputs encrypted binary images using an encryption key\n"
	       "provided as an input hex string.\n");
	printf("\n");
	printf("Usage:\n");
	printf("\t%s [OPTIONS]\n\n", cmd);

	printf("Available options:\n");
	opt = long_opt;
	while (opt->name) {
		p = line;
		rem = HELP_OPT_MAX_LEN;
		if (isalpha(opt->val)) {
			/* Short format */
			sprintf(p, "-%c,", (char)opt->val);
			p += 3;
			rem -= 3;
		}
		snprintf(p, rem, "--%s %s", opt->name,
			 (opt->has_arg == required_argument) ? "<arg>" : "");
		printf("\t%-32s %s\n", line, cmd_opt_get_help_msg(i));
		opt++;
		i++;
	}
	printf("\n");
}

static int get_key_alg(const char *key_alg_str)
{
	int i;

	for (i = 0 ; i < NUM_ELEM(key_algs_str) ; i++) {
		if (strcmp(key_alg_str, key_algs_str[i]) == 0) {
			return i;
		}
	}

	return -1;
}

/* Common command line options */
static const cmd_opt_t common_cmd_opt[] = {
	{
		{ "help", no_argument, NULL, 'h' },
		"Print this message and exit"
	},
	{
		{ "key-alg", required_argument, NULL, 'a' },
		"Encryption key algorithm: 'gcm' (default)"
	},
	{
		{ "key", required_argument, NULL, 'k' },
		"Encryption key (for supported algorithm)."
	},
};

static int cmd_fw_init(void)
{
	cmd_opt_t cmd_opt;
	fw_t *fw;
	unsigned int i;

	for (i = 0; i < num_fws; i++) {
		fw = &fws[i];
		if (fw->opt != NULL) {
			cmd_opt.long_opt.name = fw->opt;
			cmd_opt.long_opt.has_arg = required_argument;
			cmd_opt.long_opt.flag = NULL;
			cmd_opt.long_opt.val = CMD_OPT_FW;
			cmd_opt.help_msg = fw->help_msg;
			cmd_opt_add(&cmd_opt);
		}
	}

	return 0;
}

static fw_t *fw_get_by_opt(const char *opt)
{
	fw_t *fw;
	unsigned int i;

	for (i = 0; i < num_fws; i++) {
		fw = &fws[i];
		if (strcmp(fw->opt, opt) == 0) {
			return fw;
		}
	}

	return NULL;
}

static void check_cmd_params(void)
{
	unsigned int i;

	for (i = 0; i < num_fws; i += 2) {
		if (fws[i].fn != NULL) {
			if (fws[i + 1].fn == NULL) {
				ERROR("Missing required cmdline option: %s\n",
				      fws[i + 1].opt);
				exit(1);
			}
		} else {
			if (fws[i + 1].fn != NULL) {
				ERROR("Missing required cmdline option: %s\n",
				      fws[i].opt);
				exit(1);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	int i, key_alg;
	int c, opt_idx = 0;
	const struct option *cmd_opt;
	const char *cur_opt;
	char *key = NULL;
	fw_t *fw;

	NOTICE("Firmware Encryption Tool: %s\n", build_msg);
	NOTICE("Target platform: %s\n", platform_msg);

	/* Set default options */
	key_alg = KEY_ALG_GCM;

	/* Add common command line options */
	for (i = 0; i < NUM_ELEM(common_cmd_opt); i++) {
		cmd_opt_add(&common_cmd_opt[i]);
	}

	if (cmd_fw_init() != 0) {
		ERROR("Cannot initialize firmware cmdline options\n");
		exit(1);
	}

	/* Get the command line options populated during the initialization */
	cmd_opt = cmd_opt_get_array();

	while (1) {
		/* getopt_long stores the option index here. */
		c = getopt_long(argc, argv, "a:hk:", cmd_opt, &opt_idx);

		/* Detect the end of the options. */
		if (c == -1) {
			break;
		}

		switch (c) {
		case 'a':
			key_alg = get_key_alg(optarg);
			if (key_alg < 0) {
				ERROR("Invalid key algorithm '%s'\n", optarg);
				exit(1);
			}
			break;
		case 'k':
			key = optarg;
			break;
		case 'h':
			print_help(argv[0], cmd_opt);
			exit(0);
		case CMD_OPT_FW:
			cur_opt = cmd_opt_get_name(opt_idx);
			fw = fw_get_by_opt(cur_opt);
			fw->fn = strdup(optarg);
			break;
		case '?':
		default:
			print_help(argv[0], cmd_opt);
			exit(1);
		}
	}

	if (!key) {
		ERROR("Key must not be NULL\n");
		exit(1);
	}

	/* Check command line arguments */
	check_cmd_params();

	for (i = 0; i < num_fws; i += 2) {
		if (fws[i].fn != NULL)
			encrypt_file(key_alg, key, fws[i].fn, fws[i + 1].fn);
	}

	CRYPTO_cleanup_all_ex_data();

	return 0;
}
