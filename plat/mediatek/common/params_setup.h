/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PARAMS_SETUP_H
#define PARAMS_SETUP_H

#include <plat_params.h>

struct bl_aux_gpio_info *plat_get_mtk_gpio_reset(void);
void params_early_setup(u_register_t plat_param_from_bl2);

#endif
