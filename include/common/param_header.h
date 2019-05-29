/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PARAM_HEADER_H
#define PARAM_HEADER_H

#include <stdbool.h>
#include <lib/utils_def.h>

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif /*__ASSEMBLY__*/

#include "param_header_raw.h"

/* Deprecated name for param header version -- switch to PARAM_VERSION_X! */
#define VERSION_1	PARAM_VERSION_1
#define VERSION_2	PARAM_VERSION_2

#define SET_PARAM_HEAD(_p, _type, _ver, _attr) do { \
	(_p)->h.type = (uint8_t)(_type); \
	(_p)->h.version = (uint8_t)(_ver); \
	(_p)->h.size = (uint16_t)sizeof(*(_p)); \
	(_p)->h.attr = (uint32_t)(_attr) ; \
	} while (false)

/* Following is used for populating structure members statically. */
#define SET_STATIC_PARAM_HEAD(_p, _type, _ver, _p_type, _attr)	\
	._p.h.type = (uint8_t)(_type), \
	._p.h.version = (uint8_t)(_ver), \
	._p.h.size = (uint16_t)sizeof(_p_type), \
	._p.h.attr = (uint32_t)(_attr)

#endif /* PARAM_HEADER_H */
