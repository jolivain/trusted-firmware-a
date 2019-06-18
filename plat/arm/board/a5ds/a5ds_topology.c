/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <drivers/arm/fvp/fvp_pwrc.h>
#include <lib/cassert.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* The FVP VE power domain tree descriptor */
static const unsigned char a5ds_power_domain_tree_desc[] = {
	1,
	/* No of children for the root node */
	A5DS_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	A5DS_CORE_COUNT,
};

/*******************************************************************************
 * This function returns the topology according to A5DS_CLUSTER_COUNT.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return a5ds_power_domain_tree_desc;
}

/*******************************************************************************
 * Currently only core 0 is used
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	return 0;
}
