#
# Copyright (c) 2020, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

# Define your module name
MODULE := helpers

# Add your source code here
LOCAL_SRCS-y := ${LOCAL_DIR}/mtk_helpers.S
LOCAL_SRCS-y += $(LOCAL_DIR)/$(ARCH_VERSION)/arch_helpers.S
LOCAL_SRCS-$(CONFIG_MTK_SOC_HELPERS) += $(LOCAL_DIR)/$(MTK_SOC)_helpers.S

# Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
