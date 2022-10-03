/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef NSSMC_OPTEED_MACROS_H
#define NSSMC_OPTEED_MACROS_H

#include <common/runtime_svc.h>

#define NSSMC_OPTEED_CALL(func_num) \
		((SMC_TYPE_FAST << FUNCID_TYPE_SHIFT) | \
		((SMC_32) << FUNCID_CC_SHIFT) | \
		(50 << FUNCID_OEN_SHIFT) | \
		((func_num) & FUNCID_NUM_MASK))

#endif /* NSSMC_OPTEED_MACROS_H */
