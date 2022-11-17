/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FEAT_DETECT_H
#define FEAT_DETECT_H

#include <arch_features.h>
#include <common/debug.h>

/* Function Prototypes */
void detect_arch_features(void);

/* Macro Definitions */
#define FEAT_STATE_1	1
#define FEAT_STATE_2	2
#define feat_detect_panic(a, b)		((a) ? feature_detected(b) : feature_panic(b))

/*******************************************************************************
 * Function : feature_detected
 * Customised panic module with debugging logging mechanism to list the feature
 * is supported by the PE.
 ******************************************************************************/
static inline void feature_detected(char *feat_name)
{
	VERBOSE("FEAT_%s is supported by the PE\n", feat_name);
}

/*******************************************************************************
 * Function : feature_panic
 * Customised panic module with error logging mechanism to list the feature
 * not supported by the PE.
 ******************************************************************************/
static inline void feature_panic(char *feat_name)
{
	ERROR("FEAT_%s not supported by the PE\n", feat_name);
	panic();
}

#endif /* FEAT_DETECT_H */
