/*
 * Copyright 2020-2023 NXP
 *
 * Copyright (c) 2017-2023 Nuvoton Technology Corp.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl31/ehf.h>

#include <platform_def.h>

ehf_pri_desc_t nuvoton_exceptions[] = {
#if SDEI_SUPPORT
	/* Critical priority SDEI */
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_SDEI_CRITICAL_PRI),

	/* Normal priority SDEI */
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_SDEI_NORMAL_PRI),
#endif
};

/* Plug in ARM exceptions to Exception Handling Framework. */
EHF_REGISTER_PRIORITIES(nuvoton_exceptions, ARRAY_SIZE(nuvoton_exceptions), PLAT_PRI_BITS);
