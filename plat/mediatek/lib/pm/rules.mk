#
# Copyright (c) 2021, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

# Define your module name
MODULE := pm

# Add your source code here
LOCAL_SRCS-y := ${LOCAL_DIR}/mtk_pm.c


# Epilogue, build sources, since needs IMAGE_AT_EL3
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))

ifeq (${CONFIG_MTK_PM_SUPPORT},y)
# Include sub rules.mk
SUB_RULES-$(CONFIG_MTK_PM_SUPPORT) := $(LOCAL_DIR)/armv${CONFIG_MTK_PM_ARCH}
# Expand sub rules.mk
$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
endif
