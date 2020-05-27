/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TBBR_IMG_DEF_H
#define TBBR_IMG_DEF_H

#if defined(SPD_spmd)
#define SP_CONTENT_CERT_ID		U(31)
#define MAX_SP_IDS			U(8)
#define SPM_IMAGES			(MAX_SP_IDS + U(1))
#else
#define SPM_IMAGES			0
#endif

#include <export/common/tbbr/tbbr_img_def_exp.h>

#endif /* TBBR_IMG_DEF_H */
