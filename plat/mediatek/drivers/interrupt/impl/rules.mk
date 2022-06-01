#
# Copyright (c) 2021, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)
LOCAL_SRCS-y :=

#Define your module name
MODULE := interrupt_impl

#Add your source code here
LOCAL_SRCS-$(CONFIG_GICV3_IMPL) := $(LOCAL_DIR)/gicv3_impl.c

#Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
