#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

# Define your module name
MODULE := uart

# Add your source code here
LOCAL_SRCS-$(CONFIG_MTK_UART) := $(LOCAL_DIR)/uart_impl.c
LOCAL_SRCS-$(CONFIG_UART_8250) += $(LOCAL_DIR)/8250_console.S

# Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
