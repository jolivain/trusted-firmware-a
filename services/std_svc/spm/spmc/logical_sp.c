/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include "logical_sp.h"
#include <services/ffa_svc.h>
#include "spmc.h"

/*******************************************************************************
 * Validate any logical partition descriptors before we initialise.
 * Initialization of said partitions will be taken care of during SPMC boot.
 ******************************************************************************/
int el3_sp_desc_validate(void)
{
	int index;
	int inner_idx;
	el3_lp_desc_t *logical_partitions;

	/*
	 * Assert the number of descriptors is less than maximum allowed.
	 * This constant should be define on a per platform basis.
	 */
	assert(EL3_LP_DESCS_COUNT <= MAX_EL3_LP_DESCS_COUNT);

	/* Check the array bounds are valid. */
	assert(EL3_LP_DESCS_END >= EL3_LP_DESCS_START);

	/* If no logical partitions are implemented then simply bail out */
	if (EL3_LP_DESCS_COUNT == 0U) {
		return 0;
	}

	logical_partitions = get_el3_lp_array();

	for (index = 0U; index < EL3_LP_DESCS_COUNT; index++) {
		el3_lp_desc_t *lp_descriptor = &logical_partitions[index];

		/*
		 * Validate our logical partition descriptors.
		 */

		if (lp_descriptor == NULL) {
			ERROR("Invalid Logical Partition Descriptor\n");
			return -EINVAL;
		}

		/*
		 * Ensure the ID follows the convention to indidate it resides
		 * in the secure world.
		 */
		if (!ffa_is_secure_world_id(lp_descriptor->sp_id)) {
			ERROR("Invalid Logical Partition ID (0x%x)\n",
			      lp_descriptor->sp_id);
			return -EINVAL;
		}

		/* Ensure we don't conflict with the SPMC partition ID. */
		if (lp_descriptor->sp_id == FFA_SPMC_ID) {
			ERROR("Invalid Logical Partition ID (0x%x)\n",
			      lp_descriptor->sp_id);
			return -EINVAL;
		}

		/* Check we have a UUID specified. */
		if (lp_descriptor->uuid == NULL) {
			ERROR("Invalid UUID Specified for Logical Partition (0x%x)\n",
			      lp_descriptor->sp_id);
			return -EINVAL;
		}

		/* Ensure the UUID is not the NULL UUID. */
		if (lp_descriptor->uuid[0] == 0 && lp_descriptor->uuid[1] == 0 &&
		    lp_descriptor->uuid[2] == 0 && lp_descriptor->uuid[3] == 0) {
			ERROR("Invalid UUID Specified for Logical Partition (0x%x)\n",
			      lp_descriptor->sp_id);
			return -EINVAL;
		}

		/* Ensure we have init function callback registered. */
		if (lp_descriptor->init == NULL) {
			ERROR("Missing init function for Logical Partition (0x%x)\n",
			      lp_descriptor->sp_id);
			return -EINVAL;
		}

		/* Ensure we have direct request function callback registered. */
		if (lp_descriptor->direct_req == NULL) {
			ERROR("Missing Direct Request handler for Logical Partition (0x%x)\n",
			      lp_descriptor->sp_id);
			return -EINVAL;
		}

		/* Ensure that all partition IDs are unique. */
		for (inner_idx = index + 1; inner_idx < EL3_LP_DESCS_COUNT; inner_idx++) {
			el3_lp_desc_t *lp_descriptor_other = &logical_partitions[inner_idx];

			if (lp_descriptor->sp_id == lp_descriptor_other->sp_id) {
				ERROR("Duplicate Partition ID Detected (0x%x)\n",
				      lp_descriptor->sp_id);
				return -EINVAL;
			}
		}
	}
	return 0;
}
