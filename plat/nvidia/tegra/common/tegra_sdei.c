/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* SDEI configuration for Tegra platforms */

#include <platform_def.h>

#include <bl31/ehf.h>
#include <common/bl_common.h>
#include <lib/utils_def.h>
#include <services/sdei.h>

/* SDEI dynamic private event numbers */
#define TEGRA_SDEI_DP_EVENT_0		U(100)
#define TEGRA_SDEI_DP_EVENT_1		U(101)
#define TEGRA_SDEI_DP_EVENT_2		U(102)

/* SDEI dynamic shared event numbers */
#define TEGRA_SDEI_DS_EVENT_0		U(200)
#define TEGRA_SDEI_DS_EVENT_1		U(201)
#define TEGRA_SDEI_DS_EVENT_2		U(202)

/* SDEI explicit events */
#define TEGRA_SDEI_EP_EVENT_0		U(300)
#define TEGRA_SDEI_EP_EVENT_1		U(301)

/* Private event mappings */
static sdei_ev_map_t tegra_sdei_private[] = {
	/* Event 0 definition */
	SDEI_DEFINE_EVENT_0(TEGRA_SDEI_SGI_PRIVATE),

	/* Dynamic private events */
	SDEI_PRIVATE_EVENT(TEGRA_SDEI_DP_EVENT_0, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
	SDEI_PRIVATE_EVENT(TEGRA_SDEI_DP_EVENT_1, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
	SDEI_PRIVATE_EVENT(TEGRA_SDEI_DP_EVENT_2, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),

	SDEI_EXPLICIT_EVENT(TEGRA_SDEI_EP_EVENT_0, SDEI_MAPF_CRITICAL),
	SDEI_EXPLICIT_EVENT(TEGRA_SDEI_EP_EVENT_1, SDEI_MAPF_NORMAL)
};

/* Shared event mappings */
static sdei_ev_map_t tegra_sdei_shared[] = {
	/* Dynamic shared events */
	SDEI_SHARED_EVENT(TEGRA_SDEI_DS_EVENT_0, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
	SDEI_SHARED_EVENT(TEGRA_SDEI_DS_EVENT_1, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
	SDEI_SHARED_EVENT(TEGRA_SDEI_DS_EVENT_2, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC)
};

/* Export ARM SDEI events */
REGISTER_SDEI_MAP(tegra_sdei_private, tegra_sdei_shared);
