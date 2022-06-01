#
# Copyright (c) 2020, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

# Define your module name
MODULE := boot_tag

# Add your source code here
LOCAL_SRCS-y := $(LOCAL_DIR)/boot_tag.c
LOCAL_SRCS-y += $(LOCAL_DIR)/boot_reason.c

# Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
