/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_CSS_H
#define SOC_CSS_H

/*
 * Utility functions for ARM CSS SoCs
 */
void soc_css_init_nic400(void);
void soc_css_init_pcie(void);

static inline void soc_css_security_setup(void)
{
#if (defined(PLAT_tc) && (defined(TARGET_FLAVOUR_FPGA) || (TARGET_PLATFORM == 4)))
#else
	soc_css_init_nic400();
	soc_css_init_pcie();
#endif
}

#endif /* SOC_CSS_H */
