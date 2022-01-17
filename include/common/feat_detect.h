/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FEAT_DETECT
#define FEAT_DETECT

#include <arch_features.h>
#include <common/debug.h>

/* Function Prototypes */
void detect_arch_features(void);

/* Macro Definitions */
#define FEAT_STATE_1	1
#define FEAT_STATE_2	2
#define feat_detect_panic(a, b)		((a) ? (void)0 : features_panic(b))

/*******************************************************************************
 * Function : features_panic
 * Customised panic module with error logging mechanism to list all the features
 * not supported by the PE.
 ******************************************************************************/
static inline void features_panic(char *feat_name)
{
	ERROR("FEAT_%s not supported by the PE\n", feat_name);
        panic();
}

#endif /* FEAT_DETECT */
