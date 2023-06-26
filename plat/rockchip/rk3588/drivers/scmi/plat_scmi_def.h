/*
 * Copyright (C) 2023, Rockchip Electronics Co., Ltd.
 * All rights reserved.
 */

#ifndef PLAT_SCMI_DEF_H
#define PLAT_SCMI_DEF_H

#include <platform_def.h>

#define SMT_BUFFER_BASE		SCMI_SHARE_MEM_BASE
#define SMT_BUFFER0_BASE	SMT_BUFFER_BASE

void rockchip_init_scmi_server(void);

#endif /* PLAT_SCMI_DEF_H */
