#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := mtk_ptp3

LOCAL_SRCS-y := ${LOCAL_DIR}/mtk_ptp3_common.c
LOCAL_SRCS-y += ${LOCAL_DIR}/$(MTK_SOC)/mtk_ptp3_main.c

PLAT_INCLUDES += -I${LOCAL_DIR}
PLAT_INCLUDES += -I${LOCAL_DIR}/$(MTK_SOC)

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
