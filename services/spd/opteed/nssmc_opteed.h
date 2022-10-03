/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NSSMC_OPTEED_H
#define NSSMC_OPTEED_H

#include "nssmc_opteed_macros.h"

/*
 * This file specifies SMC function IDs used when the secure monitor is invoked
 * from the non-secure world.
 */

/*
 * Load OPTEE image from the payload specified in the registers.
 *
 * WARNING: Use this cautiously as it could lead to insecure loading of the
 * Trusted OS. Further details are in opteed.mk.
 *
 * Call register usage:
 * x0 SMC Function ID, OPTEE_SMC_CALL_LOAD_ELF
 * x1 Upper 32bit of a 64bit size for the payload
 * x2 Lower 32bit of a 64bit size for the payload
 * x3 Upper 32bit of the physical address for the payload
 * x4 Lower 32bit of the physical address for the payload
 *
 * The payload consists of a optee_image_info struct that contains
 * optee_segment structs in a flex array, immediately following that in memory
 * is the data referenced by the optee_segment structs.
 * Example:
 *
 * struct optee_image_info (with n segments specified)
 * segment 0 data
 * segment 1 data
 * ...
 * segment n-1 data
 *
 * Returns 0 on success and an error code otherwise.
 */
#define NSSMC_OPTEED_FUNCID_LOAD_ELF 2
#define NSSMC_OPTEED_CALL_LOAD_ELF \
	NSSMC_OPTEED_CALL(NSSMC_OPTEED_FUNCID_LOAD_ELF)

#endif /*NSSMC_OPTEED_H*/
