/*
 * Copyright (c) 2019, Linaro Limited
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TBB_FW_H
#define TBB_FW_H

typedef struct fw_s {
	const char *opt;	/* Command line option to specify a firmware */
	const char *help_msg;	/* Help message */
	char *fn;		/* Filename to load/store the firmware */
} fw_t;

/* Macro to register the firmwares to be encrypted */
#define REGISTER_FWS(_fws) \
	fw_t *fws = &_fws[0]; \
	const unsigned int num_fws = sizeof(_fws)/sizeof(_fws[0])

/* Exported variables */
extern fw_t *fws;
extern const unsigned int num_fws;

#endif /* TBB_FW_H */
