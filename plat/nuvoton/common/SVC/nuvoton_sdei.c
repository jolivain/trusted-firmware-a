/*
 * Copyright (c) 2017-2023, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright 2020-2023 NXP
 *
 * Copyright (c) 2017-2023 Nuvoton Technology Corp.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* SDEI configuration for ARM platforms */

#include <bl31/ehf.h>
#include <common/debug.h>
#include <lib/utils_def.h>
#include <platform_def.h>
#include <services/sdei.h>

/* Private event mappings */
static sdei_ev_map_t nuvoton_sdei_private[] = {
	SDEI_DEFINE_EVENT_0(PLAT_SDEI_SGI_PRIVATE),
};

/* Shared event mappings */
static sdei_ev_map_t inuvoton_sdei_shared[] = {
};

void plat_sdei_setup(void)
{
	INFO("SDEI platform setup\n");
}

/* Export ARM SDEI events */
REGISTER_SDEI_MAP(nuvoton_sdei_private, nuvoton_sdei_shared);
