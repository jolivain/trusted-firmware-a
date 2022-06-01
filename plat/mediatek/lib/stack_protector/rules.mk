#
# Copyright (c) 2021, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

# Define your module name
MODULE := stack_protector

# Add your source code here
LOCAL_SRCS-y := $(LOCAL_DIR)/mtk_stack_protector.c

# Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
