/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PARAMS_SETUP_H
#define PARAMS_SETUP_H

#include <plat_params.h>

struct gpio_info *plat_get_gpio_reset(void);
void params_early_setup(void *plat_param_from_bl2);

#endif
