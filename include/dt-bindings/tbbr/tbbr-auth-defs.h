/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TBBR_AUTH_DEFS_H
#define TBBR_AUTH_DEFS_H

#define INVALID_IMAGE_ID                U(0xFFFFFFFF)

/*
 * The method type defines how an image is authenticated
 */
#define AUTH_METHOD_NONE        0
#define AUTH_METHOD_HASH        1
#define AUTH_METHOD_SIG         2
#define AUTH_METHOD_NV_CTR      3
#define AUTH_METHOD_NUM         4

/*
 * Image types. A parser should be instantiated and registered for each type
 */
#define IMG_RAW                 0
#define IMG_PLAT                1
#define IMG_CERT                2
#define IMG_MAX_TYPES           3

#endif /* TBBR_AUTH_DEFS_H */
