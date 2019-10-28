/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>
#include <plat/arm/css/common/css_pm.h>

#define RD_N1_EDGE_MAX_PE	PLAT_ARM_CSS_NUM_CHIPS * CSS_SGI_MAX_PE_PER_CHIP
#define SCMI_CHANNEL_ID(n)	(n << SCMI_CHANNEL_ID_SHIFT)

/******************************************************************************
 * The power domain tree descriptor.
 ******************************************************************************/
const unsigned char rdn1edge_pd_tree_desc[] = {
	PLAT_ARM_CLUSTER_COUNT * PLAT_ARM_CSS_NUM_CHIPS,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
#if (PLAT_ARM_CSS_NUM_CHIPS > 1)
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER
#endif
};

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return rdn1edge_pd_tree_desc;
}

/*******************************************************************************
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 ******************************************************************************/
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[RD_N1_EDGE_MAX_PE] = {
	(SCMI_CHANNEL_ID(0) | 0x0), (SCMI_CHANNEL_ID(0) | 0x1),
	(SCMI_CHANNEL_ID(0) | 0x2), (SCMI_CHANNEL_ID(0) | 0x3),
	(SCMI_CHANNEL_ID(0) | 0x4), (SCMI_CHANNEL_ID(0) | 0x5),
	(SCMI_CHANNEL_ID(0) | 0x6), (SCMI_CHANNEL_ID(0) | 0x7),
#if (PLAT_ARM_CSS_NUM_CHIPS > 1)
	(SCMI_CHANNEL_ID(1) | 0x0), (SCMI_CHANNEL_ID(1) | 0x1),
	(SCMI_CHANNEL_ID(1) | 0x2), (SCMI_CHANNEL_ID(1) | 0x3),
	(SCMI_CHANNEL_ID(1) | 0x4), (SCMI_CHANNEL_ID(1) | 0x5),
	(SCMI_CHANNEL_ID(1) | 0x6), (SCMI_CHANNEL_ID(1) | 0x7),
#endif
};
