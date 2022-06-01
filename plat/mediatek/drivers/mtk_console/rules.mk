#
# Copyright (c) 2020, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

# Define your module name
MODULE := mtk_console

# Add your source code here
LOCAL_SRCS-y := ${LOCAL_DIR}/mtk_console.c
LOCAL_SRCS-$(CONFIG_MEM_CONSOLE) += ${LOCAL_DIR}/mem_console.S

# Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
