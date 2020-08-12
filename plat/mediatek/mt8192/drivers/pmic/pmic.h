/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_H
#define PMIC_H

enum {
	PMIC_PWRHOLD = 0x0a08,
};

/* external API */
void pmic_power_off(void);

#endif /* PMIC_H */
