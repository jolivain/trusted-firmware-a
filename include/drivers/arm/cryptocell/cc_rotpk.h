/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CC_ROTPK_H
#define _CC_ROTPK_H

#ifdef __cplusplus
extern "C"
{
#endif

int cc_get_rotpk_hash(unsigned char *dst, unsigned int len,
		      unsigned int *flags);

#ifdef __cplusplus
}
#endif

#endif
