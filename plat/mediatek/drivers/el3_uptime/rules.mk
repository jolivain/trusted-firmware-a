#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

# Define your module name
MODULE := el3_uptime

# Add your source code here
LOCAL_SRCS-y := $(LOCAL_DIR)/el3_uptime.c
LOCAL_SRCS-y += $(LOCAL_DIR)/asm_el3_uptime.S

# Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
